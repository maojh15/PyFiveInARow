cmake_minimum_required(VERSION 3.12)
project(MonteCarloTreeSearch)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
find_package(Python3 3.11 EXACT COMPONENTS Interpreter Development REQUIRED)
add_subdirectory(/Users/maojiahao/cppLibraries/package/pybind11 pybind11_build)

pybind11_add_module(py_MCTS "src/cpp/monte_carlo_tree_search.cpp"
    "src/cpp/monte_carlo_tree_search.h" "src/cpp/bindings.cpp")

install(TARGETS py_MCTS DESTINATION ${CMAKE_SOURCE_DIR}/)