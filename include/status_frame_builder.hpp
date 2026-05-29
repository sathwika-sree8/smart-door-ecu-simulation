#pragma once

#include <linux/can.h>
#include "door_controller.hpp"

/**
 * @file status_frame_builder.hpp
 * @brief Declares helpers for building and printing CAN status frames.
 */

/**
 * @brief Builds a CAN status frame from the current door controller state.
 *
 * @param controller DoorController instance providing state information.
 * @return CAN status frame representing the controller state.
 */
struct can_frame buildStatusFrame(const DoorController& controller);

/**
 * @brief Prints a CAN frame and associated label to standard output.
 *
 * @param frame CAN frame to print.
 * @param label Descriptive label for the printed frame.
 */
void printCanFrame(const struct can_frame& frame, const char* label);
