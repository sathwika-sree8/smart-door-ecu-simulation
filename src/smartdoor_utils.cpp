#include "smartdoor_utils.hpp"

#include <iostream>

Command decodeCommand(uint8_t value)
{
    switch (value)
    {
    case 0x01:
        return Command::LOCK;
    case 0x02:
        return Command::UNLOCK;
    case 0x03:
        return Command::DOOR_OPEN;
    case 0x04:
        return Command::DOOR_CLOSE;
    case 0x05:
        return Command::RESET_FAULT;
    case 0x06:
        return Command::READ_STATUS;
    default:
        return Command::UNKNOWN;
    }
}

const char* toString(Command cmd)
{
    switch (cmd)
    {
    case Command::LOCK:
        return "LOCK";
    case Command::UNLOCK:
        return "UNLOCK";
    case Command::DOOR_OPEN:
        return "DOOR_OPEN";
    case Command::DOOR_CLOSE:
        return "DOOR_CLOSE";
    case Command::RESET_FAULT:
        return "RESET_FAULT";
    case Command::READ_STATUS:
        return "READ_STATUS";
    case Command::UNKNOWN:
    default:
        return "UNKNOWN";
    }
}

const char* toString(LockState state)
{
    return (state == LockState::LOCKED) ? "LOCKED" : "UNLOCKED";
}

const char* toString(DoorState state)
{
    return (state == DoorState::OPEN) ? "OPEN" : "CLOSED";
}

const char* toString(LampState state)
{
    return (state == LampState::ON) ? "ON" : "OFF";
}

const char* toString(FaultState state)
{
    switch (state)
    {
    case FaultState::NONE:
        return "NONE";
    case FaultState::LOCK_DENIED_DOOR_OPEN:
        return "LOCK_DENIED_DOOR_OPEN";
    default:
        return "UNKNOWN_FAULT";
    }
}

uint8_t toByte(LockState state)
{
    return (state == LockState::LOCKED) ? 1 : 0;
}

uint8_t toByte(DoorState state)
{
    return (state == DoorState::OPEN) ? 1 : 0;
}

uint8_t toByte(LampState state)
{
    return (state == LampState::ON) ? 1 : 0;
}

uint8_t toByte(FaultState state)
{
    return static_cast<uint8_t>(state);
}

void printState(const DoorController& controller)
{
    std::cout << "Updated Smart Door State" << std::endl;
    std::cout << "Lock State  : " << toString(controller.getLockState()) << std::endl;
    std::cout << "Door State  : " << toString(controller.getDoorState()) << std::endl;
    std::cout << "Lamp State  : " << toString(controller.getLampState()) << std::endl;
    std::cout << "Fault State : " << toString(controller.getFaultState()) << std::endl;
}