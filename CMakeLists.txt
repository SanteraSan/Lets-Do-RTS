cmake_minimum_required(VERSION 3.24)
project(LetsDoRts VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RTS_ENABLE_SANITIZERS "Enable Address and Undefined sanitizers" ON)

if(RTS_ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(rts_math INTERFACE)
target_include_directories(rts_math INTERFACE "${CMAKE_SOURCE_DIR}/src")
target_compile_features(rts_math INTERFACE cxx_std_17)

enable_testing()
find_package(Catch2 3 CONFIG REQUIRED)

add_executable(rts_tests
    tests/math/Vector2Tests.cpp
    tests/core/LoggerTests.cpp
)
target_link_libraries(rts_tests PRIVATE rts_math Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(rts_tests)
