#pragma once

#include <cstdint>
#include "smartdoor_types.hpp"

/**
 * @file door_controller.hpp
 * @brief Defines the smart door state machine and command handling.
 */

/**
 * @brief Controls the smart door state, lamp timeout, and fault handling.
 */
class DoorController
{
public:
    /**
     * @brief Constructs a new DoorController instance.
     *
     * @param initialLockState Initial lock state of the door.
     * @param lampTimeoutMs Time in milliseconds before the lamp turns off.
     */
    explicit DoorController(LockState initialLockState = LockState::UNLOCKED,
                            uint32_t lampTimeoutMs = 3000U);

    /**
     * @brief Processes an incoming command and updates internal state.
     *
     * @param cmd Command to process.
     */
    void processCommand(Command cmd);

    /**
     * @brief Advances the controller state by the elapsed time.
     *
     * @param elapsedMs Elapsed time in milliseconds since the last tick.
     * @return true if internal state changed due to timeout expiry.
     */
    bool tick(uint32_t elapsedMs);

    /**
     * @brief Gets the current lock state.
     *
     * @return Current lock state.
     */
    LockState getLockState() const;

    /**
     * @brief Gets the current door position state.
     *
     * @return Current door state.
     */
    DoorState getDoorState() const;

    /**
     * @brief Gets the current lamp state.
     *
     * @return Current lamp state.
     */
    LampState getLampState() const;

    /**
     * @brief Gets the current fault state.
     *
     * @return Current fault state.
     */
    FaultState getFaultState() const;

private:
    LockState lockState;
    DoorState doorState;
    LampState lampState;
    FaultState faultState;

    uint32_t lampTimeoutMs;
    uint32_t lampTimerElapsedMs;
    bool lampOffPending;
};