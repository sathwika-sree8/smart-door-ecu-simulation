#pragma once

#include <string>
#include "event_log.hpp"

/**
 * @file event_file_logger.hpp
 * @brief Declares a logger for appending event data to a file.
 */

/**
 * @brief Writes event log entries to a persistent file.
 */
class EventFileLogger
{
public:
    /**
     * @brief Constructs a new event file logger.
     *
     * @param filePath Path to the file used for event output.
     */
    explicit EventFileLogger(const std::string& filePath);

    /**
     * @brief Appends an event entry to the log file.
     *
     * @param entry Event entry to write.
     * @return true if the entry was successfully written.
     * @return false if writing failed.
     */
    bool append(const EventEntry& entry) const;

    /**
     * @brief Appends a raw text line to the log file.
     *
     * @param line Line of text to append.
     * @return true if the line was successfully written.
     * @return false if writing failed.
     */
    bool appendLine(const std::string& line) const;

    /**
     * @brief Gets the configured event log file path.
     *
     * @return Reference to the file path string.
     */
    const std::string& getFilePath() const;

private:
    std::string filePath;
};