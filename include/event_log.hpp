#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <ostream>
#include <string>
#include <vector>

/**
 * @file event_log.hpp
 * @brief Defines the in-memory event log and event entry type.
 */

/**
 * @brief Represents a single entry in the event log.
 */
struct EventEntry
{
    uint64_t sequence; /**< Sequence number for ordering events. */
    std::string message; /**< Event message text. */
};

/**
 * @brief Maintains a bounded history of event entries.
 */
class EventLog
{
public:
    /**
     * @brief Constructs an EventLog with the given capacity.
     *
     * @param capacity Maximum number of events to retain.
     */
    explicit EventLog(std::size_t capacity = 32);

    /**
     * @brief Adds a new event message to the log.
     *
     * @param message Event message text.
     * @return EventEntry The stored event entry.
     */
    EventEntry add(const std::string& message);

    /**
     * @brief Returns a snapshot of the current stored events.
     *
     * @return Vector of event entries.
     */
    std::vector<EventEntry> snapshot() const;

    /**
     * @brief Dumps all stored events to the provided output stream.
     *
     * @param os Output stream to receive dumped event text.
     */
    void dump(std::ostream& os) const;

    /**
     * @brief Returns the number of stored events.
     *
     * @return Current event count.
     */
    std::size_t size() const;

private:
    std::size_t capacity;
    uint64_t nextSequence;
    std::deque<EventEntry> entries;
};