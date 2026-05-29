/**
 * @file status_frame_builder.cpp
 * @brief Builds and prints Smart Door ECU status CAN frames.
 *
 * This file converts the internal ECU state into a CAN status frame that is
 * transmitted on the configured status CAN ID.
 *
 * The frame format is:
 * - Byte 0 -> Lock state
 * - Byte 1 -> Door state
 * - Byte 2 -> Lamp state
 * - Byte 3 -> Fault state
 */

#include "status_frame_builder.hpp"
#include "project_config.hpp"
#include "smartdoor_utils.hpp"

#include <iomanip>
#include <iostream>

struct can_frame buildStatusFrame(const DoorController& controller)
{
    struct can_frame txFrame{};
    txFrame.can_id = ProjectConfig::STATUS_CAN_ID;
    txFrame.len = 4;
    txFrame.data[0] = toByte(controller.getLockState());
    txFrame.data[1] = toByte(controller.getDoorState());
    txFrame.data[2] = toByte(controller.getLampState());
    txFrame.data[3] = toByte(controller.getFaultState());

    return txFrame;
}

void printCanFrame(const struct can_frame& frame, const char* label)
{
    std::cout << label << " ID: 0x"
              << std::uppercase << std::hex << frame.can_id << std::dec
              << std::endl;

    std::cout << label << " Data: ";
    for (int i = 0; i < frame.len; ++i)
    {
        std::cout << "0x"
                  << std::uppercase
                  << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << static_cast<int>(frame.data[i])
                  << " ";
    }

    std::cout << std::dec << std::endl;
}
