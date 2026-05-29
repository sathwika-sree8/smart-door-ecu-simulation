#pragma once

/**
 * @file smartdoor_types.hpp
 * @brief Defines the core smart door ECU state and command types.
 */

/**
 * @brief Describes the lock state of the smart door.
 */
enum class LockState
{
    LOCKED, /**< Door is locked. */
    UNLOCKED /**< Door is unlocked. */
};

/**
 * @brief Describes the door position state.
 */
enum class DoorState
{
    OPEN, /**< Door is open. */
    CLOSED /**< Door is closed. */
};

/**
 * @brief Describes the lamp state.
 */
enum class LampState
{
    ON, /**< Lamp is illuminated. */
    OFF /**< Lamp is extinguished. */
};

/**
 * @brief Describes the current fault condition.
 */
enum class FaultState
{
    NONE = 0, /**< No fault is present. */
    LOCK_DENIED_DOOR_OPEN = 1 /**< Lock request denied because the door is open. */
};

/**
 * @brief Represents commands accepted by the smart door controller.
 */
enum class Command
{
    UNKNOWN = 0, /**< Command is not recognized. */
    LOCK = 1, /**< Command to lock the door. */
    UNLOCK = 2, /**< Command to unlock the door. */
    DOOR_OPEN = 3, /**< Command indicating the door opened. */
    DOOR_CLOSE = 4, /**< Command indicating the door closed. */
    RESET_FAULT = 5, /**< Command to reset the current fault. */
    READ_STATUS = 6 /**< Command to request a status update. */
};