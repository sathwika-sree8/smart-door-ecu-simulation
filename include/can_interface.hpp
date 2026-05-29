#pragma once

#include <string>
#include <linux/can.h>

/**
 * @file can_interface.hpp
 * @brief Declares a wrapper for Linux CAN socket communication.
 *
 * This header defines the public API for opening a CAN socket, binding it to
 * an interface, transmitting and receiving CAN frames, and closing the socket.
 */

/**
 * @brief Encapsulates a Linux CAN socket interface.
 *
 * The class manages opening, closing, sending, and receiving CAN frames
 * over a specified CAN network interface.
 */
class CanInterface
{
public:
    /**
     * @brief Constructs a new CAN interface manager.
     */
    CanInterface();

    /**
     * @brief Destroys the CAN interface manager and closes any open socket.
     */
    ~CanInterface();

    /**
     * @brief Opens a CAN socket on the given interface.
     *
     * @param interfaceName Name of the CAN interface (for example, "vcan0").
     * @return true if the socket opened successfully.
     * @return false if the socket could not be opened.
     */
    bool open(const std::string& interfaceName);

    /**
     * @brief Closes the currently open CAN socket.
     */
    void close();

    /**
     * @brief Receives a CAN frame from the open socket.
     *
     * @param frame Reference to the CAN frame object to populate.
     * @return true if a frame was received successfully.
     * @return false if receive failed or no socket is open.
     */
    bool receive(struct can_frame& frame);

    /**
     * @brief Receives a CAN frame with a timeout.
     *
     * @param frame Reference to the CAN frame object to populate.
     * @param timeoutMs Receive timeout in milliseconds.
     * @return int Number of bytes received, or -1 on error or timeout.
     */
    int receiveWithTimeout(struct can_frame& frame, int timeoutMs);

    /**
     * @brief Sends a CAN frame over the open socket.
     *
     * @param frame The CAN frame to transmit.
     * @return true if the frame was sent successfully.
     * @return false if send failed or no socket is open.
     */
    bool send(const struct can_frame& frame);

private:
    int socketFd;
};