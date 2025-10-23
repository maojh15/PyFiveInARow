cmake_minimum_required(VERSION 3.20)
project(MonteCarloTreeSearch)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include(FetchContent)

# Try to find an installed pybind11 first
find_package(pybind11 QUIET)

# If not found, fetch pybind11 from its GitHub repository
if(NOT pybind11_FOUND AND NOT TARGET pybind11::module)
    message(STATUS "pybind11 not found - fetching from https://github.com/pybind/pybind11.git")
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.11.0  # adjust tag (or use main) as needed
    )
    FetchContent_MakeAvailable(pybind11)
endif()

pybind11_add_module(py_MCTS "src/cpp/monte_carlo_tree_search.cpp"
    "src/cpp/monte_carlo_tree_search.h" "src/cpp/bindings.cpp")

install(TARGETS py_MCTS DESTINATION ${CMAKE_SOURCE_DIR}/)