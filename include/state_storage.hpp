#pragma once

#include <string>
#include "smartdoor_types.hpp"

/**
 * @file state_storage.hpp
 * @brief Declares persistent lock state storage helpers.
 */

/**
 * @brief Loads and saves the door lock state to a file.
 */
class StateStorage
{
public:
    /**
     * @brief Constructs a new StateStorage object.
     *
     * @param filePath Path to the file used for persistent state.
     */
    explicit StateStorage(const std::string& filePath);

    /**
     * @brief Loads the saved lock state from storage.
     *
     * @param state Reference to store the loaded lock state.
     * @return true if state was successfully loaded.
     * @return false if loading failed.
     */
    bool loadLockState(LockState& state) const;

    /**
     * @brief Saves the current lock state to storage.
     *
     * @param state Lock state to save.
     * @return true if the state was successfully saved.
     * @return false if saving failed.
     */
    bool saveLockState(LockState state) const;

private:
    std::string filePath;
};