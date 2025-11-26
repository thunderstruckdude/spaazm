cmake_minimum_required(VERSION 3.16)
project(FlightReservation VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(SQLite3 REQUIRED)

# Enable automoc for Qt's meta-object compiler
set(CMAKE_AUTOMOC ON)

# Add executable
add_executable(FlightReservation
    main_gui.cpp
    flight_system.cpp
    flight_system.h
)

# Link Qt libraries
target_link_libraries(FlightReservation
    Qt6::Core
    Qt6::Widgets
    SQLite::SQLite3
)

# Set output directory
set_target_properties(FlightReservation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# For Windows: copy Qt DLLs to output directory
if(WIN32)
    add_custom_command(TARGET FlightReservation POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Core>
        $<TARGET_FILE:Qt6::Widgets>
        $<TARGET_FILE_DIR:FlightReservation>
    )
endif()
