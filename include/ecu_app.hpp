#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "can_interface.hpp"
#include "door_controller.hpp"
#include "event_log.hpp"
#include "event_file_logger.hpp"
#include "state_storage.hpp"
#include "project_config.hpp"

/**
 * @file ecu_app.hpp
 * @brief Declares the main ECU application controller for the smart door ECU.
 */

/**
 * @brief Manages the ECU event loop, CAN communication, persistent storage, and logging.
 */
class EcuApp
{
public:
    /**
     * @brief Constructs a new ECU application instance.
     *
     * @param canInterfaceName Name of the CAN interface to use.
     * @param storageFilePath Path to the persistent lock state storage file.
     * @param eventLogFilePath Path to the event log output file.
     * @param loopTimeoutMs Event loop polling timeout in milliseconds.
     * @param lampTimeoutMs Lamp timeout interval in milliseconds.
     * @param eventHistoryCapacity Maximum number of events retained in memory.
     */
    EcuApp(const std::string& canInterfaceName = "vcan0",
           const std::string& storageFilePath = ProjectConfig::STATE_FILE,
           const std::string& eventLogFilePath = ProjectConfig::EVENT_FILE,
           uint32_t loopTimeoutMs = 100U,
           uint32_t lampTimeoutMs = 3000U,
           std::size_t eventHistoryCapacity = 32U);

    /**
     * @brief Initializes the ECU application.
     *
     * @return true if initialization succeeds.
     * @return false if initialization fails.
     */
    bool init();

    /**
     * @brief Runs the main ECU application loop.
     */
    void run();

    /**
     * @brief Requests the application loop to stop gracefully.
     */
    void requestStop();

private:
    std::string canInterfaceName;
    uint32_t loopTimeoutMs;
    uint32_t lampTimeoutMs;

    bool running;
    bool initialized;

    CanInterface can;
    StateStorage storage;
    EventLog eventLog;
    EventFileLogger eventFileLogger;
    std::unique_ptr<DoorController> controller;

    void printStartupBanner() const;
    void handleTimeout();
    void handleFrame(const struct can_frame& rxFrame);
    void sendStatusFrame(const std::string& reason);
    void logEvent(const std::string& message);
};