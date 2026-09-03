#include "core/Logger.hpp"
#include "platform/Window.hpp"

#include <iostream>
#include <string>

int main() {
    std::ofstream logFile;
    const bool fileOpen = openLogFile(logFile, "engine.log");

    Logger logger{std::cout, fileOpen ? &logFile : nullptr};
    if (!fileOpen) {
        logger.log(LogLevel::Warn, "could not open engine.log, console only");
    }
    logger.log(LogLevel::Info, "engine start");

    Window window;
    if (!window.open({"Lets Do RTS", 1280, 720})) {
        std::string line = "failed to open window";
        const char* sdlError = SDL_GetError();
        if (sdlError != nullptr && sdlError[0] != '\0') {
            line += ": ";
            line += sdlError;
        }
        logger.log(LogLevel::Error, line);
        return 1;
    }
    logger.log(LogLevel::Info, "window open");

    while (window.isOpen()) {
        window.pollEvents();
    }

    logger.log(LogLevel::Info, "engine stop");
    return 0;
}
