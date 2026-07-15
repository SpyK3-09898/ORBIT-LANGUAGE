
cmake_minimum_required(VERSION 3.20)

project(ORBIT
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output executable directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/_bin)
# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ORBIT_CORE module sources
set(ORBIT_CORE
    _src/core/lexer/lexer.cpp
)

# __ORBIT module sources
set(_ORBIT
    _src/main/main.cpp
    _src/core/template.cpp
)

# Create final executable
add_executable(ORBIT
    ${ORBIT_CORE}
    ${_ORBIT}
)       

# Include directories
target_include_directories(ORBIT
    PRIVATE
        ${CMAKE_SOURCE_DIR}/_include
)