#include "core/Logger.hpp"

#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>

TEST_CASE("logLevelName maps each level") {
    REQUIRE(std::string(logLevelName(LogLevel::Error)) == "ERROR");
    REQUIRE(std::string(logLevelName(LogLevel::Warn)) == "WARN");
    REQUIRE(std::string(logLevelName(LogLevel::Info)) == "INFO");
    REQUIRE(std::string(logLevelName(LogLevel::Debug)) == "DEBUG");
}

TEST_CASE("formatLogLine wraps level and message") {
    REQUIRE(formatLogLine(LogLevel::Info, "hello") == "[INFO] hello\n");
}

TEST_CASE("Logger writes the line to the console stream") {
    std::ostringstream console;
    Logger logger{console};
    logger.log(LogLevel::Warn, "low gold");
    REQUIRE(console.str() == "[WARN] low gold\n");
}

TEST_CASE("Logger writes the same line to console and file streams") {
    std::ostringstream console;
    std::ostringstream file;
    Logger logger{console, &file};
    logger.log(LogLevel::Error, "boom");
    REQUIRE(console.str() == "[ERROR] boom\n");
    REQUIRE(file.str() == "[ERROR] boom\n");
}

TEST_CASE("Logger without a file stream still writes to console") {
    std::ostringstream console;
    Logger logger{console, nullptr};
    logger.log(LogLevel::Debug, "tick");
    REQUIRE(console.str() == "[DEBUG] tick\n");
}
