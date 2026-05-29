#include "event_log.hpp"

EventLog::EventLog(std::size_t capacity)
    : capacity(capacity),
      nextSequence(1U)
{
}

EventEntry EventLog::add(const std::string& message)
{
    if (entries.size() >= capacity)
    {
        entries.pop_front();
    }

    EventEntry entry{nextSequence++, message};
    entries.push_back(entry);
    return entry;
}

std::vector<EventEntry> EventLog::snapshot() const
{
    return std::vector<EventEntry>(entries.begin(), entries.end());
}

void EventLog::dump(std::ostream& os) const
{
    os << "Recent Event History (" << entries.size() << " entries)" << std::endl;
    for (const auto& entry : entries)
    {
        os << "  [" << entry.sequence << "] " << entry.message << std::endl;
    }
}

std::size_t EventLog::size() const
{
    return entries.size();
}