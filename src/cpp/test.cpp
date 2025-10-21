#include "monte_carlo_tree_search.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>

void output_date_time(const char *hint_str="") {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    char buf[9];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", tm);
    std::cout << "datetime: " << buf << ": " << hint_str << std::endl;
}

void TestFunc(int board_sz = 11) {
    std::vector<std::vector<int>> state(board_sz, std::vector<int>(board_sz, 0));
    state[5][5]=1;
    for (auto &row : state) {
        for (auto &cell : row) {
            std::cout << cell << " ";
        }
        std::cout << "\n";
    }
    MonteCarloTreeSearch tree(state, 1);

    std::cout << "start search...\n";
    auto time1 = std::chrono::steady_clock::now();
    auto best_move = tree.SearchMove(100000);
    auto time2 = std::chrono::steady_clock::now();
    std::cout << "Best move: (" << best_move.first << ", " << best_move.second << ")\n";
    std::cout << "Search time: "
              << std::chrono::duration<double>(time2 - time1).count()
              << "s\n";
    std::cout << "Tree size: " << tree.GetTreeNodesNumbers()
              << ", depth: " << tree.GetTreeDepth() << "\n";
    output_date_time("end test func");
}

int main(int argc, char** argv) {
    output_date_time();
    TestFunc(11);
    output_date_time();
    output_date_time();
    TestFunc(11);
    output_date_time();
    std::cout << "done!" << std::endl;
    return 0;
}