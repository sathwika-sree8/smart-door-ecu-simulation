#include "event_file_logger.hpp"

#include <fstream>

EventFileLogger::EventFileLogger(const std::string& filePath)
    : filePath(filePath)
{
}

bool EventFileLogger::append(const EventEntry& entry) const
{
    std::ofstream out(filePath, std::ios::app);
    if (!out.is_open())
    {
        return false;
    }

    out << "[" << entry.sequence << "] " << entry.message << "\n";
    return true;
}

bool EventFileLogger::appendLine(const std::string& line) const
{
    std::ofstream out(filePath, std::ios::app);
    if (!out.is_open())
    {
        return false;
    }

    out << line << "\n";
    return true;
}

const std::string& EventFileLogger::getFilePath() const
{
    return filePath;
}