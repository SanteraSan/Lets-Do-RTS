#pragma once

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

inline std::string formatLogLine(LogLevel level, std::string_view message) {
    std::string line;
    line += '[';
    line += logLevelName(level);
    line += "] ";
    line += message;
    line += '\n';
    return line;
}

struct Logger {
    std::ostream& console;
    std::ostream* file = nullptr;
    LogLevel minLevel = LogLevel::Debug;

    void log(LogLevel level, std::string_view message) {
        if (!shouldLog(level, minLevel)) {
            return;
        }
        const std::string line = formatLogLine(level, message);
        console << line << std::flush;
        if (file != nullptr) {
            *file << line << std::flush;
        }
    }
};
