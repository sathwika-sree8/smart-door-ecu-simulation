/**
 * @file ecu_app.cpp
 * @brief Implements the EcuApp runtime orchestrator for the Smart Door ECU simulation.
 *
 * This file contains the top-level application flow for the Smart Door ECU:
 * - application startup
 * - CAN receive loop
 * - command dispatch
 * - timeout-driven behavior
 * - persistence triggering
 * - event logging
 *
 * EcuApp acts as the integration layer that connects communication, business logic,
 * persistence, and observability into one continuously running ECU process.
 */

#include "ecu_app.hpp"

#include <iomanip>
#include <iostream>
#include <linux/can.h>
#include <sstream>
#include <string>

#include "project_config.hpp"
#include "smartdoor_utils.hpp"
#include "status_frame_builder.hpp"

namespace
{
/**
 * @brief Format CAN ID for readable log output.
 * @param canId Raw CAN identifier.
 * @return Formatted string such as "0x101".
 */
std::string formatCanId(uint32_t canId)
{
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex << canId << std::dec;
    return oss.str();
}
} // namespace

EcuApp::EcuApp(const std::string& canInterfaceName,
               const std::string& storageFilePath,
               const std::string& eventLogFilePath,
               uint32_t loopTimeoutMs,
               uint32_t lampTimeoutMs,
               std::size_t eventHistoryCapacity)
    : canInterfaceName(canInterfaceName),
      loopTimeoutMs(loopTimeoutMs),
      lampTimeoutMs(lampTimeoutMs),
      running(false),
      initialized(false),
      storage(storageFilePath),
      eventLog(eventHistoryCapacity),
      eventFileLogger(eventLogFilePath),
      controller(nullptr)
{
}

bool EcuApp::init()
{
    std::cout << "[INFO] [ECU] Smart Door ECU Simulation Started" << std::endl;

    if (!eventFileLogger.appendLine("============================================================"))
    {
        std::cerr << "[WARN] [LOG] Failed to write session banner to event log file" << std::endl;
    }
    if (!eventFileLogger.appendLine("ECU SESSION START"))
    {
        std::cerr << "[WARN] [LOG] Failed to write session start marker to event log file" << std::endl;
    }
    if (!eventFileLogger.appendLine("============================================================"))
    {
        std::cerr << "[WARN] [LOG] Failed to write session separator to event log file" << std::endl;
    }

    if (!can.open(canInterfaceName))
    {
        std::cerr << "[ERROR] [CAN] EcuApp init failed: could not open interface "
                  << canInterfaceName << std::endl;
        return false;
    }

    LockState initialLockState = LockState::UNLOCKED;
    if (storage.loadLockState(initialLockState))
    {
        std::cout << "[INFO] [PERSIST] Restored persisted lock state: "
                  << toString(initialLockState) << std::endl;
        logEvent(std::string("Restored persisted lock state: ") + toString(initialLockState));
    }
    else
    {
        std::cout << "[INFO] [PERSIST] No persisted state found. Starting with default UNLOCKED state."
                  << std::endl;
        logEvent("No persisted state found. Starting with default UNLOCKED state.");
    }

    controller = std::make_unique<DoorController>(initialLockState, lampTimeoutMs);

    printStartupBanner();
    logEvent("ECU initialized successfully");
    initialized = true;
    running = true;
    return true;
}

void EcuApp::printStartupBanner() const
{
    std::cout << "[INFO] [ECU] ECU running continuously..." << std::endl;
    std::cout << "[INFO] [CFG] Command CAN ID : "
              << formatCanId(ProjectConfig::COMMAND_CAN_ID) << std::endl;
    std::cout << "[INFO] [CFG] Status CAN ID  : "
              << formatCanId(ProjectConfig::STATUS_CAN_ID) << std::endl;
    std::cout << "[INFO] [CFG] Loop timeout   : "
              << loopTimeoutMs << " ms" << std::endl;
    std::cout << "[INFO] [CFG] Lamp timeout   : "
              << lampTimeoutMs << " ms" << std::endl;
    std::cout << "[INFO] [CFG] State file     : "
              << ProjectConfig::STATE_FILE << std::endl;
    std::cout << "[INFO] [CFG] Event file     : "
              << ProjectConfig::EVENT_FILE << std::endl;

    std::cout << "[INFO] [CMD] Supported commands:" << std::endl;
    std::cout << "  0x01 = LOCK" << std::endl;
    std::cout << "  0x02 = UNLOCK" << std::endl;
    std::cout << "  0x03 = DOOR_OPEN" << std::endl;
    std::cout << "  0x04 = DOOR_CLOSE" << std::endl;
    std::cout << "  0x05 = RESET_FAULT" << std::endl;
    std::cout << "  0x06 = READ_STATUS" << std::endl;

    std::cout << "[INFO] [RULE] LOCK is rejected if door is open" << std::endl;
    std::cout << "[INFO] [ECU] Press Ctrl+C to stop" << std::endl;
}

void EcuApp::run()
{
    if (!initialized || !controller)
    {
        std::cerr << "[ERROR] [ECU] EcuApp::run() called before successful init()" << std::endl;
        return;
    }

    while (running)
    {
        struct can_frame rxFrame{};
        int receiveStatus = can.receiveWithTimeout(rxFrame, static_cast<int>(loopTimeoutMs));

        if (receiveStatus < 0)
        {
            std::cerr << "[ERROR] [CAN] receiveWithTimeout failed, continuing..." << std::endl;
            logEvent("receiveWithTimeout failed");
            continue;
        }

        if (receiveStatus == 0)
        {
            handleTimeout();
            continue;
        }

        handleFrame(rxFrame);
    }

    logEvent("ECU main loop exited");

    if (!eventFileLogger.appendLine("ECU SESSION END"))
    {
        std::cerr << "[WARN] [LOG] Failed to write session end marker to event log file" << std::endl;
    }

    std::cout << "\n[INFO] [ECU] ECU stopping. Final event history dump:" << std::endl;
    eventLog.dump(std::cout);
}

void EcuApp::requestStop()
{
    running = false;
    logEvent("Stop requested");
}

void EcuApp::handleTimeout()
{
    if (!controller)
    {
        return;
    }

    if (controller->tick(loopTimeoutMs))
    {
        std::cout << "[INFO] [TIMEOUT] Time-driven state update occurred" << std::endl;
        logEvent("Lamp timeout expired");
        printState(*controller);
        sendStatusFrame("timeout");
    }
}

void EcuApp::handleFrame(const struct can_frame& rxFrame)
{
    if (!controller)
    {
        return;
    }

    std::cout << "\n[INFO] [CAN] Received CAN frame" << std::endl;
    std::cout << "[INFO] [CAN] ID  : " << formatCanId(rxFrame.can_id) << std::endl;
    std::cout << "[INFO] [CAN] DLC : " << static_cast<int>(rxFrame.len) << std::endl;

    if (rxFrame.can_id != ProjectConfig::COMMAND_CAN_ID)
    {
        std::cout << "[WARN] [CAN] Ignoring frame: unsupported CAN ID" << std::endl;
        logEvent("Ignored frame: unsupported CAN ID");
        return;
    }

    if (rxFrame.len < 1)
    {
        std::cout << "[WARN] [CAN] Ignoring frame: no command byte present" << std::endl;
        logEvent("Ignored frame: no command byte");
        return;
    }

    Command cmd = decodeCommand(rxFrame.data[0]);

    if (cmd == Command::UNKNOWN)
    {
        std::cout << "[WARN] [CMD] Ignoring frame: unknown command 0x"
                  << std::hex
                  << static_cast<int>(rxFrame.data[0])
                  << std::dec
                  << std::endl;
        logEvent("Ignored frame: unknown command");
        return;
    }

    std::ostringstream beforeMsg;
    beforeMsg << "Command received: " << toString(cmd);
    logEvent(beforeMsg.str());

    // READ_STATUS is a diagnostic-style query only.
    // It must not alter state or clear any active fault.
    if (cmd == Command::READ_STATUS)
    {
        std::cout << "[INFO] [CMD] READ_STATUS requested" << std::endl;
        printState(*controller);
        sendStatusFrame("READ_STATUS");

        std::cout << "[INFO] [LOG] Event history dump on READ_STATUS:" << std::endl;
        eventLog.dump(std::cout);
        return;
    }

    LockState oldLockState = controller->getLockState();

    controller->processCommand(cmd);
    printState(*controller);

    if (controller->getLockState() != oldLockState)
    {
        if (storage.saveLockState(controller->getLockState()))
        {
            std::cout << "[INFO] [PERSIST] Persisted lock state: "
                      << toString(controller->getLockState()) << std::endl;
            logEvent(std::string("Persisted lock state: ") + toString(controller->getLockState()));
        }
        else
        {
            std::cerr << "[ERROR] [PERSIST] Failed to persist lock state" << std::endl;
            logEvent("Failed to persist lock state");
        }
    }

    std::ostringstream afterMsg;
    afterMsg << "State after " << toString(cmd)
             << " => lock=" << toString(controller->getLockState())
             << ", door=" << toString(controller->getDoorState())
             << ", lamp=" << toString(controller->getLampState())
             << ", fault=" << toString(controller->getFaultState());
    logEvent(afterMsg.str());

    sendStatusFrame(toString(cmd));
}

void EcuApp::sendStatusFrame(const std::string& reason)
{
    if (!controller)
    {
        return;
    }

    struct can_frame txFrame = buildStatusFrame(*controller);

    if (can.send(txFrame))
    {
        std::cout << "[INFO] [CAN] Sent status frame" << std::endl;
        printCanFrame(txFrame, "TX");

        std::ostringstream msg;
        msg << "Status sent (" << reason << "): "
            << static_cast<int>(txFrame.data[0]) << " "
            << static_cast<int>(txFrame.data[1]) << " "
            << static_cast<int>(txFrame.data[2]) << " "
            << static_cast<int>(txFrame.data[3]);
        logEvent(msg.str());
    }
    else
    {
        std::cerr << "[ERROR] [CAN] Failed to send status frame" << std::endl;
        logEvent("Failed to send status frame");
    }
}

void EcuApp::logEvent(const std::string& message)
{
    EventEntry entry = eventLog.add(message);

    if (!eventFileLogger.append(entry))
    {
        std::cerr << "[WARN] [LOG] Failed to append event to file log: "
                  << message << std::endl;
    }
}