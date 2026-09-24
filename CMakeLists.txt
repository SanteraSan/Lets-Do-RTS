cmake_minimum_required(VERSION 3.24)
project(LetsDoRts VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RTS_ENABLE_SANITIZERS "Enable Address and Undefined sanitizers on tests" ON)

add_library(rts_math INTERFACE)
target_include_directories(rts_math INTERFACE "${CMAKE_SOURCE_DIR}/src")
target_compile_features(rts_math INTERFACE cxx_std_17)

find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
add_library(rts_platform
    src/platform/Window.cpp
    src/platform/DebugDraw.cpp
    src/platform/GlDraw.cpp
    src/render/DrawWorld.cpp
)
target_include_directories(rts_platform PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(rts_platform PUBLIC SDL2::SDL2 OpenGL::GL)

add_executable(rts_game src/main.cpp)
target_link_libraries(rts_game PRIVATE rts_platform)

enable_testing()
find_package(Catch2 3 CONFIG REQUIRED)

add_executable(rts_tests
    tests/math/Vector2Tests.cpp
    tests/math/MoveTowardsTests.cpp
    tests/core/LoggerTests.cpp
    tests/core/FixedTimestepTests.cpp
    tests/platform/WindowTests.cpp
    tests/platform/CameraTests.cpp
    tests/platform/BindingTests.cpp
    tests/ecs/WorldTests.cpp
    tests/ecs/SelectionTests.cpp
    tests/nav/GridTests.cpp
    tests/nav/ApproachTests.cpp
    tests/nav/PathTests.cpp
    tests/nav/GridMoveTests.cpp
    tests/render/AppearanceTests.cpp
    tests/render/UnitSpriteTests.cpp
    tests/render/UnitCubeTests.cpp
    tests/render/StandLiftTests.cpp
)
target_link_libraries(rts_tests PRIVATE rts_math rts_platform Catch2::Catch2WithMain)

if(RTS_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(rts_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(rts_tests PRIVATE -fsanitize=address,undefined)
endif()

include(Catch)
catch_discover_tests(rts_tests)
