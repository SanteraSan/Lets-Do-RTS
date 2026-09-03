#include "core/Logger.hpp"

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <sstream>
#include <string>

namespace {

std::string fixedTimestamp() {
    return "2026-09-03 12:00:00";
}

} // namespace

TEST_CASE("logLevelName maps each level") {
    REQUIRE(std::string(logLevelName(LogLevel::Error)) == "ERROR");
    REQUIRE(std::string(logLevelName(LogLevel::Warn)) == "WARN");
    REQUIRE(std::string(logLevelName(LogLevel::Info)) == "INFO");
    REQUIRE(std::string(logLevelName(LogLevel::Debug)) == "DEBUG");
}

TEST_CASE("formatTimestamp of epoch is 1970-01-01 00:00:00 UTC") {
    const auto epoch = std::chrono::system_clock::from_time_t(0);
    REQUIRE(formatTimestamp(epoch) == "1970-01-01 00:00:00");
}

TEST_CASE("formatLogLine wraps timestamp, level and message") {
    REQUIRE(
        formatLogLine(LogLevel::Info, "hello", "2026-09-03 12:00:00") ==
        "[2026-09-03 12:00:00] [INFO] hello\n");
}

TEST_CASE("Logger writes the line to the console stream") {
    std::ostringstream console;
    Logger logger{console, nullptr, LogLevel::Debug, fixedTimestamp};
    logger.log(LogLevel::Warn, "low gold");
    REQUIRE(console.str() == "[2026-09-03 12:00:00] [WARN] low gold\n");
}

TEST_CASE("Logger writes the same line to console and file streams") {
    std::ostringstream console;
    std::ostringstream file;
    Logger logger{console, &file, LogLevel::Debug, fixedTimestamp};
    logger.log(LogLevel::Error, "boom");
    REQUIRE(console.str() == "[2026-09-03 12:00:00] [ERROR] boom\n");
    REQUIRE(file.str() == "[2026-09-03 12:00:00] [ERROR] boom\n");
}

TEST_CASE("Logger without a file stream still writes to console") {
    std::ostringstream console;
    Logger logger{console, nullptr, LogLevel::Debug, fixedTimestamp};
    logger.log(LogLevel::Debug, "tick");
    REQUIRE(console.str() == "[2026-09-03 12:00:00] [DEBUG] tick\n");
}

TEST_CASE("shouldLog keeps Error when minimum is Info") {
    REQUIRE(shouldLog(LogLevel::Error, LogLevel::Info));
}

TEST_CASE("shouldLog drops Debug when minimum is Info") {
    REQUIRE_FALSE(shouldLog(LogLevel::Debug, LogLevel::Info));
}

TEST_CASE("Logger skips messages below minLevel") {
    std::ostringstream console;
    std::ostringstream file;
    Logger logger{console, &file, LogLevel::Info, fixedTimestamp};
    logger.log(LogLevel::Debug, "noise");
    REQUIRE(console.str().empty());
    REQUIRE(file.str().empty());
}

TEST_CASE("Logger writes messages at minLevel") {
    std::ostringstream console;
    Logger logger{console, nullptr, LogLevel::Info, fixedTimestamp};
    logger.log(LogLevel::Info, "start");
    REQUIRE(console.str() == "[2026-09-03 12:00:00] [INFO] start\n");
}
