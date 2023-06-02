cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

# For the compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the Kconfig file
set(CONF_FILE "prj.conf")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/drivetrain)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)
