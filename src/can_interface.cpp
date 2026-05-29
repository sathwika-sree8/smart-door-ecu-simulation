#include "can_interface.hpp"

#include <iostream>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <poll.h>

#include <linux/can.h>
#include <linux/can/raw.h>

CanInterface::CanInterface() : socketFd(-1)
{
}

CanInterface::~CanInterface()
{
    close();
}

bool CanInterface::open(const std::string& interfaceName)
{
    socketFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socketFd < 0)
    {
        std::cerr << "ERROR: Failed to create CAN socket" << std::endl;
        return false;
    }

    struct ifreq ifr;
    std::strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(socketFd, SIOCGIFINDEX, &ifr) < 0)
    {
        std::cerr << "ERROR: Failed to get interface index for " << interfaceName << std::endl;
        ::close(socketFd);
        socketFd = -1;
        return false;
    }

    struct sockaddr_can addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socketFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        std::cerr << "ERROR: Failed to bind CAN socket to " << interfaceName << std::endl;
        ::close(socketFd);
        socketFd = -1;
        return false;
    }

    std::cout << "CAN socket opened and bound to interface: " << interfaceName << std::endl;
    return true;
}

/**
 * @brief Receives a CAN frame from the open socket.
 *
 * @param frame Reference to the CAN frame object to populate.
 * @return true if a frame was received successfully.
 * @return false if receive failed or no socket is open.
 */
bool CanInterface::receive(struct can_frame& frame)
{
    int bytesRead = read(socketFd, &frame, sizeof(struct can_frame));

    if (bytesRead < 0)
    {
        std::cerr << "ERROR: Failed to read CAN frame" << std::endl;
        return false;
    }

    if (bytesRead != sizeof(struct can_frame))
    {
        std::cerr << "ERROR: Incomplete CAN frame received" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Receives a CAN frame with a timeout.
 *
 * @param frame Reference to the CAN frame object to populate.
 * @param timeoutMs Receive timeout in milliseconds.
 * @return int Number of bytes received, or -1 on error or timeout.
 */
int CanInterface::receiveWithTimeout(struct can_frame& frame, int timeoutMs)
{
    struct pollfd pfd{};
    pfd.fd = socketFd;
    pfd.events = POLLIN;

    int pollResult = poll(&pfd, 1, timeoutMs);

    if (pollResult < 0)
    {
        std::cerr << "ERROR: poll() failed" << std::endl;
        return -1;
    }

    if (pollResult == 0)
    {
        return 0; // timeout
    }

    if (pfd.revents & POLLIN)
    {
        return receive(frame) ? 1 : -1;
    }

    return -1;
}

/**
 * @brief Sends a CAN frame over the open socket.
 *
 * @param frame The CAN frame to transmit.
 * @return true if the frame was sent successfully.
 * @return false if send failed or no socket is open.
 */
bool CanInterface::send(const struct can_frame& frame)
{
    int bytesWritten = write(socketFd, &frame, sizeof(struct can_frame));

    if (bytesWritten < 0)
    {
        std::cerr << "ERROR: Failed to send CAN frame" << std::endl;
        return false;
    }

    if (bytesWritten != sizeof(struct can_frame))
    {
        std::cerr << "ERROR: Incomplete CAN frame sent" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Closes the currently open CAN socket.
 */
void CanInterface::close()
{
    if (socketFd >= 0)
    {
        ::close(socketFd);
        socketFd = -1;
    }
}