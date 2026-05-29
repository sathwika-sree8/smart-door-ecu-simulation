#include "state_storage.hpp"

#include <fstream>

StateStorage::StateStorage(const std::string& filePath)
    : filePath(filePath)
{
}

bool StateStorage::loadLockState(LockState& state) const
{
    std::ifstream in(filePath);
    if (!in.is_open())
    {
        return false;
    }

    std::string value;
    in >> value;

    if (value == "LOCKED")
    {
        state = LockState::LOCKED;
        return true;
    }

    if (value == "UNLOCKED")
    {
        state = LockState::UNLOCKED;
        return true;
    }

    return false;
}

bool StateStorage::saveLockState(LockState state) const
{
    std::ofstream out(filePath, std::ios::trunc);
    if (!out.is_open())
    {
        return false;
    }

    if (state == LockState::LOCKED)
    {
        out << "LOCKED";
    }
    else
    {
        out << "UNLOCKED";
    }

    return true;
}