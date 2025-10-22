cmake_minimum_required(VERSION 3.20)
project(MonteCarloTreeSearch)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
find_package(Python3 3.11 EXACT COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

pybind11_add_module(py_MCTS "src/cpp/monte_carlo_tree_search.cpp"
    "src/cpp/monte_carlo_tree_search.h" "src/cpp/bindings.cpp")

install(TARGETS py_MCTS DESTINATION ${CMAKE_SOURCE_DIR}/)

add_executable(test "src/cpp/monte_carlo_tree_search.cpp"
    "src/cpp/monte_carlo_tree_search.h" "src/cpp/test.cpp")
target_include_directories(test PUBLIC "${Python3_INCLUDE_DIRS}")
target_include_directories(test PUBLIC "/Users/maojiahao/cppLibraries/package/pybind11/include")
target_link_libraries(test PUBLIC Python3::Python)
install(TARGETS test DESTINATION ${CMAKE_SOURCE_DIR}/bin)