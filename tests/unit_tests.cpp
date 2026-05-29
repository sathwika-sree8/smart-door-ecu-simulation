#include <iostream>
#include <string>
#include <cstdio>
#include <filesystem>
#include <fstream>

#include "door_controller.hpp"
#include "smartdoor_utils.hpp"
#include "status_frame_builder.hpp"
#include "state_storage.hpp"
#include "event_log.hpp"
#include "event_file_logger.hpp"

static int g_failedTests = 0;
static int g_totalTests = 0;

#define EXPECT_TRUE(condition)                                                          \
    do                                                                                  \
    {                                                                                   \
        ++g_totalTests;                                                                 \
        if (!(condition))                                                               \
        {                                                                               \
            ++g_failedTests;                                                            \
            std::cerr << "[FAIL] " << __FUNCTION__ << ":" << __LINE__                   \
                      << " -> EXPECT_TRUE(" #condition ")" << std::endl;                \
        }                                                                               \
    } while (0)

#define EXPECT_EQ(expected, actual)                                                     \
    do                                                                                  \
    {                                                                                   \
        ++g_totalTests;                                                                 \
        auto expValue = (expected);                                                     \
        auto actValue = (actual);                                                       \
        if (!(expValue == actValue))                                                    \
        {                                                                               \
            ++g_failedTests;                                                            \
            std::cerr << "[FAIL] " << __FUNCTION__ << ":" << __LINE__                   \
                      << " -> EXPECT_EQ(" #expected ", " #actual ")"                    \
                      << " expected=" << static_cast<int>(expValue)                     \
                      << " actual=" << static_cast<int>(actValue) << std::endl;         \
        }                                                                               \
    } while (0)

static void test_decodeCommand()
{
    EXPECT_EQ(Command::LOCK, decodeCommand(0x01));
    EXPECT_EQ(Command::UNLOCK, decodeCommand(0x02));
    EXPECT_EQ(Command::DOOR_OPEN, decodeCommand(0x03));
    EXPECT_EQ(Command::DOOR_CLOSE, decodeCommand(0x04));
    EXPECT_EQ(Command::RESET_FAULT, decodeCommand(0x05));
    EXPECT_EQ(Command::READ_STATUS, decodeCommand(0x06));
    EXPECT_EQ(Command::UNKNOWN, decodeCommand(0x99));
}

static void test_initialState()
{
    DoorController controller;

    EXPECT_EQ(LockState::UNLOCKED, controller.getLockState());
    EXPECT_EQ(DoorState::CLOSED, controller.getDoorState());
    EXPECT_EQ(LampState::OFF, controller.getLampState());
    EXPECT_EQ(FaultState::NONE, controller.getFaultState());
}

static void test_lockCommand()
{
    DoorController controller;

    controller.processCommand(Command::LOCK);

    EXPECT_EQ(LockState::LOCKED, controller.getLockState());
    EXPECT_EQ(DoorState::CLOSED, controller.getDoorState());
    EXPECT_EQ(LampState::OFF, controller.getLampState());
    EXPECT_EQ(FaultState::NONE, controller.getFaultState());
}

static void test_repeatedLockIgnored()
{
    DoorController controller;

    controller.processCommand(Command::LOCK);
    controller.processCommand(Command::LOCK);

    EXPECT_EQ(LockState::LOCKED, controller.getLockState());
    EXPECT_EQ(FaultState::NONE, controller.getFaultState());
}

static void test_unlockIgnoredWhenAlreadyUnlocked()
{
    DoorController controller;

    controller.processCommand(Command::UNLOCK);

    EXPECT_EQ(LockState::UNLOCKED, controller.getLockState());
    EXPECT_EQ(FaultState::NONE, controller.getFaultState());
}

static void test_doorOpenTurnsLampOn()
{
    DoorController controller;

    controller.processCommand(Command::DOOR_OPEN);

    EXPECT_EQ(DoorState::OPEN, controller.getDoorState());
    EXPECT_EQ(LampState::ON, controller.getLampState());
    EXPECT_EQ(FaultState::NONE, controller.getFaultState());
}

static void test_lockRejectedWhenDoorOpen()
{
    DoorController controller;

    controller.processCommand(Command::DOOR_OPEN);
    controller.processCommand(Command::LOCK);

    EXPECT_EQ(LockState::UNLOCKED, controller.getLockState());
    EXPECT_EQ(DoorState::OPEN, controller.getDoorState());
    EXPECT_EQ(LampState::ON, controller.getLampState());
    EXPECT_EQ(FaultState::LOCK_DENIED_DOOR_OPEN, controller.getFaultState());
}

static void test_resetFaultCommand()
{
    DoorController controller;

    controller.processCommand(Command::DOOR_OPEN);
    controller.processCommand(Command::LOCK);

    EXPECT_EQ(FaultState::LOCK_DENIED_DOOR_OPEN, controller.getFaultState());

    controller.processCommand(Command::RESET_FAULT);

    EXPECT_EQ(FaultState::NONE, controller.getFaultState());
    EXPECT_EQ(DoorState::OPEN, controller.getDoorState());
    EXPECT_EQ(LockState::UNLOCKED, controller.getLockState());
    EXPECT_EQ(LampState::ON, controller.getLampState());
}

static void test_doorCloseStartsLampTimeout()
{
    DoorController controller(LockState::UNLOCKED, 3000U);

    controller.processCommand(Command::DOOR_OPEN);
    controller.processCommand(Command::DOOR_CLOSE);

    EXPECT_EQ(DoorState::CLOSED, controller.getDoorState());
    EXPECT_EQ(LampState::ON, controller.getLampState());

    EXPECT_TRUE(controller.tick(1000U) == false);
    EXPECT_EQ(LampState::ON, controller.getLampState());

    EXPECT_TRUE(controller.tick(1000U) == false);
    EXPECT_EQ(LampState::ON, controller.getLampState());

    EXPECT_TRUE(controller.tick(1000U) == true);
    EXPECT_EQ(LampState::OFF, controller.getLampState());
}

static void test_statusFrameBuilder()
{
    DoorController controller;

    controller.processCommand(Command::LOCK);
    controller.processCommand(Command::DOOR_OPEN);
    controller.processCommand(Command::LOCK);

    struct can_frame frame = buildStatusFrame(controller);

    EXPECT_EQ(0x201, frame.can_id);
    EXPECT_EQ(4, frame.len);
    EXPECT_EQ(1, frame.data[0]);
    EXPECT_EQ(1, frame.data[1]);
    EXPECT_EQ(1, frame.data[2]);
    EXPECT_EQ(1, frame.data[3]);
}

static void test_persistedLockState()
{
    const std::string testFile = "test_state_storage.txt";

    std::filesystem::remove(testFile);

    StateStorage storage(testFile);

    LockState loaded = LockState::UNLOCKED;

    EXPECT_TRUE(storage.loadLockState(loaded) == false);

    EXPECT_TRUE(storage.saveLockState(LockState::LOCKED) == true);

    loaded = LockState::UNLOCKED;
    EXPECT_TRUE(storage.loadLockState(loaded) == true);
    EXPECT_EQ(LockState::LOCKED, loaded);

    EXPECT_TRUE(storage.saveLockState(LockState::UNLOCKED) == true);

    loaded = LockState::LOCKED;
    EXPECT_TRUE(storage.loadLockState(loaded) == true);
    EXPECT_EQ(LockState::UNLOCKED, loaded);

    std::filesystem::remove(testFile);
}

static void test_eventLogRingBuffer()
{
    EventLog log(3U);

    log.add("Event A");
    log.add("Event B");
    log.add("Event C");
    log.add("Event D");

    auto entries = log.snapshot();

    EXPECT_EQ(3, static_cast<int>(entries.size()));
    EXPECT_TRUE(entries[0].message == "Event B");
    EXPECT_TRUE(entries[1].message == "Event C");
    EXPECT_TRUE(entries[2].message == "Event D");
}

static void test_eventFileLogger()
{
    const std::string testFile = "test_event_file_logger.log";
    std::filesystem::remove(testFile);

    EventFileLogger fileLogger(testFile);

    EXPECT_TRUE(fileLogger.appendLine("SESSION START") == true);

    EventEntry e1{1U, "Command received: LOCK"};
    EventEntry e2{2U, "Status sent (LOCK): 1 0 0 0"};

    EXPECT_TRUE(fileLogger.append(e1) == true);
    EXPECT_TRUE(fileLogger.append(e2) == true);

    std::ifstream in(testFile);
    EXPECT_TRUE(in.is_open());

    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());

    EXPECT_TRUE(content.find("SESSION START") != std::string::npos);
    EXPECT_TRUE(content.find("[1] Command received: LOCK") != std::string::npos);
    EXPECT_TRUE(content.find("[2] Status sent (LOCK): 1 0 0 0") != std::string::npos);

    std::filesystem::remove(testFile);
}

int main()
{
    std::cout << "Running Smart Door ECU unit tests..." << std::endl;

    test_decodeCommand();
    test_initialState();
    test_lockCommand();
    test_repeatedLockIgnored();
    test_unlockIgnoredWhenAlreadyUnlocked();
    test_doorOpenTurnsLampOn();
    test_lockRejectedWhenDoorOpen();
    test_resetFaultCommand();
    test_doorCloseStartsLampTimeout();
    test_statusFrameBuilder();
    test_persistedLockState();
    test_eventLogRingBuffer();
    test_eventFileLogger();

    std::cout << "Total tests : " << g_totalTests << std::endl;
    std::cout << "Failed tests: " << g_failedTests << std::endl;

    if (g_failedTests == 0)
    {
        std::cout << "[PASS] All unit tests passed." << std::endl;
        return 0;
    }

    std::cerr << "[FAIL] Some unit tests failed." << std::endl;
    return 1;
}