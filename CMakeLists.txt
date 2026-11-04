cmake_minimum_required(VERSION 3.28)

project(CircuitSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(circuit_solver
    src/main.cpp
)

target_sources(circuit_solver
    PRIVATE
        FILE_SET CXX_MODULES
        FILES src/components.ixx
)