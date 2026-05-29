#include "door_controller.hpp"
#include <iostream>

DoorController::DoorController(LockState initialLockState, uint32_t lampTimeoutMs)
    : lockState(initialLockState),
      doorState(DoorState::CLOSED),
      lampState(LampState::OFF),
      faultState(FaultState::NONE),
      lampTimeoutMs(lampTimeoutMs),
      lampTimerElapsedMs(0U),
      lampOffPending(false)
{
}

void DoorController::processCommand(Command cmd)
{
    // RESET_FAULT is special: clear fault and keep state unchanged
    if (cmd == Command::RESET_FAULT)
    {
        faultState = FaultState::NONE;
        std::cout << "[DoorController] RESET_FAULT applied: fault cleared" << std::endl;
        return;
    }

    // Clear old fault at the beginning of a normal valid command cycle
    faultState = FaultState::NONE;

    switch (cmd)
    {
    case Command::LOCK:
        if (doorState == DoorState::OPEN)
        {
            faultState = FaultState::LOCK_DENIED_DOOR_OPEN;
            std::cout << "[DoorController] LOCK rejected: door is open" << std::endl;
        }
        else if (lockState == LockState::LOCKED)
        {
            std::cout << "[DoorController] LOCK ignored: already locked" << std::endl;
        }
        else
        {
            lockState = LockState::LOCKED;
            std::cout << "[DoorController] LOCK applied" << std::endl;
        }
        break;

    case Command::UNLOCK:
        if (lockState == LockState::UNLOCKED)
        {
            std::cout << "[DoorController] UNLOCK ignored: already unlocked" << std::endl;
        }
        else
        {
            lockState = LockState::UNLOCKED;
            std::cout << "[DoorController] UNLOCK applied" << std::endl;
        }
        break;

    case Command::DOOR_OPEN:
        if (doorState == DoorState::OPEN)
        {
            std::cout << "[DoorController] DOOR_OPEN ignored: already open" << std::endl;
        }
        else
        {
            doorState = DoorState::OPEN;
            lampState = LampState::ON;
            lampOffPending = false;
            lampTimerElapsedMs = 0U;
            std::cout << "[DoorController] DOOR_OPEN applied: lamp ON" << std::endl;
        }
        break;

    case Command::DOOR_CLOSE:
        if (doorState == DoorState::CLOSED)
        {
            std::cout << "[DoorController] DOOR_CLOSE ignored: already closed" << std::endl;
        }
        else
        {
            doorState = DoorState::CLOSED;

            // Lamp remains ON for timeout period after door close
            lampState = LampState::ON;
            lampOffPending = true;
            lampTimerElapsedMs = 0U;

            std::cout << "[DoorController] DOOR_CLOSE applied: lamp timeout started" << std::endl;
        }
        break;

    case Command::UNKNOWN:
    default:
        std::cout << "[DoorController] UNKNOWN command ignored" << std::endl;
        break;
    }
}

bool DoorController::tick(uint32_t elapsedMs)
{
    if (!lampOffPending)
    {
        return false;
    }

    lampTimerElapsedMs += elapsedMs;

    if (lampTimerElapsedMs >= lampTimeoutMs)
    {
        lampState = LampState::OFF;
        lampOffPending = false;
        lampTimerElapsedMs = 0U;

        std::cout << "[DoorController] Lamp timeout expired: lamp OFF" << std::endl;
        return true;
    }

    return false;
}

LockState DoorController::getLockState() const
{
    return lockState;
}

DoorState DoorController::getDoorState() const
{
    return doorState;
}

LampState DoorController::getLampState() const
{
    return lampState;
}

FaultState DoorController::getFaultState() const
{
    return faultState;
}