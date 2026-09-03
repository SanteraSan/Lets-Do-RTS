#pragma once

#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <ostream>
#include <string>
#include <string_view>

enum class LogLevel { Error, Warn, Info, Debug };

inline const char* logLevelName(LogLevel level) {
    switch (level) {
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Debug:
        return "DEBUG";
    }
    return "UNKNOWN";
}

inline int logLevelRank(LogLevel level) {
    switch (level) {
    case LogLevel::Error:
        return 0;
    case LogLevel::Warn:
        return 1;
    case LogLevel::Info:
        return 2;
    case LogLevel::Debug:
        return 3;
    }
    return 0;
}

inline bool shouldLog(LogLevel message, LogLevel minimum) {
    return logLevelRank(message) <= logLevelRank(minimum);
}

inline std::string formatTimestamp(std::chrono::system_clock::time_point time) {
    const std::time_t seconds = std::chrono::system_clock::to_time_t(time);
    std::tm calendar{};
    if (gmtime_r(&seconds, &calendar) == nullptr) {
        return "0000-00-00 00:00:00";
    }

    char buffer[80];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%04d-%02d-%02d %02d:%02d:%02d",
        calendar.tm_year + 1900,
        calendar.tm_mon + 1,
        calendar.tm_mday,
        calendar.tm_hour,
        calendar.tm_min,
        calendar.tm_sec);
    return buffer;
}

inline std::string formatLogLine(
    LogLevel level, std::string_view message, std::string_view timestamp) {
    std::string line;
    line += '[';
    line += timestamp;
    line += "] [";
    line += logLevelName(level);
    line += "] ";
    line += message;
    line += '\n';
    return line;
}

inline bool openLogFile(std::ofstream& file, std::string_view path) {
    if (file.is_open()) {
        file.close();
    }
    if (path.empty()) {
        return false;
    }
    file.open(std::string{path}, std::ios::out | std::ios::app);
    return file.is_open();
}

using TimestampNow = std::string (*)();

struct Logger {
    std::ostream& console;
    std::ostream* file = nullptr;
    LogLevel minLevel = LogLevel::Debug;
    TimestampNow timestampNow = nullptr;

    void log(LogLevel level, std::string_view message) {
        if (!shouldLog(level, minLevel)) {
            return;
        }
        const std::string timestamp = timestampNow != nullptr
            ? timestampNow()
            : formatTimestamp(std::chrono::system_clock::now());
        const std::string line = formatLogLine(level, message, timestamp);
        console << line << std::flush;
        if (file != nullptr) {
            *file << line << std::flush;
        }
    }
};
