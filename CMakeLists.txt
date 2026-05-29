cmake_minimum_required(VERSION 3.16)

project(SmartDoorEcuSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_executable(smart_door_ecu
    src/main.cpp
    src/can_interface.cpp
    src/door_controller.cpp
    src/state_storage.cpp
    src/smartdoor_utils.cpp
    src/status_frame_builder.cpp
    src/event_log.cpp
    src/event_file_logger.cpp
    src/ecu_app.cpp
)

include(CTest)
enable_testing()

add_executable(smart_door_ecu_tests
    tests/unit_tests.cpp
    src/door_controller.cpp
    src/state_storage.cpp
    src/smartdoor_utils.cpp
    src/status_frame_builder.cpp
    src/event_log.cpp
    src/event_file_logger.cpp
)

add_test(NAME smart_door_ecu_tests COMMAND smart_door_ecu_tests)