#ifndef __MONTE_CARLO_TREE_SEARCH_H__
#define __MONTE_CARLO_TREE_SEARCH_H__

#include <vector>
#include <numeric>
#include <cmath>
#include <random>
#include <pybind11/numpy.h>
#include <iostream>
#include <memory>

inline int get_opponent_id(int stone_id) {
    return 3 - stone_id;
}

class MonteCarloTreeSearch {
public:
    enum class PlayoutPolicy {
        UniformPlayout = 0,
        NearPlacePlayout = 1
    };

    int near_playout_policy_distance = 2;
    PlayoutPolicy playout_policy = PlayoutPolicy::NearPlacePlayout;

    using StateType = std::vector<std::vector<int>>;
    struct TreeNode {
        StateType state;
        std::shared_ptr<TreeNode> parent;
        int stone_id;
        std::pair<int, int> from_moving;
        std::vector<std::shared_ptr<TreeNode>> children;
        double win_rounds = 0;
        int total_rounds = 0;

        TreeNode(const StateType &state, std::shared_ptr<TreeNode> parent, int stone_id,
                 const std::pair<int, int> &from_moving) : state{state},
                 parent{parent}, stone_id{stone_id}, from_moving{from_moving} {
        }

        void UpdateRounds(double added_win_rounds, int added_total_rounds) {
            win_rounds += added_win_rounds;
            total_rounds += added_total_rounds;
        }

        double GetExploitPriority() const {
            if (parent == nullptr) {
                return 0.0;
            }
            return ComputeExploitPriority(parent->total_rounds);
        }

    private:
        double ComputeExploitPriority(int parent_total_rounds) const {
            const double coef = 1.4142135623730951; // sqrt(2)
            if (total_rounds == 0) {
                return std::numeric_limits<double>::infinity();
            }
            double win_ratio = win_rounds / total_rounds;
            double exploit = coef * std::sqrt(std::log<double>(parent_total_rounds) / total_rounds);
            return win_ratio + exploit;
        }
    };

    
    std::shared_ptr<TreeNode> root;
    int stone_id;

    MonteCarloTreeSearch(const StateType &root_state, int stone_id) : 
        stone_id{stone_id} {
        root = std::make_shared<TreeNode>(root_state, nullptr, get_opponent_id(stone_id), std::pair{-1, -1});
    }

    MonteCarloTreeSearch(const pybind11::array_t<int> &root_state, int stone_id) :
        MonteCarloTreeSearch(ConvertNumpyToStateType(root_state), stone_id){
    }

    std::pair<int, int> SearchMove(int iter_steps = 5000);

    std::shared_ptr<TreeNode> Selection();

    void Expansion(std::shared_ptr<TreeNode> leaf);

    void BackPropagation(std::shared_ptr<TreeNode> node, int leaf_stone_id, int game_res);

    int GetTreeNodesNumbers() const {
        return GetTreeNodesNumbers_(*root);
    }

    int GetTreeDepth() const {
        return GetTreeDepth_(*root);
    }

    std::vector<int> StaticDepthNodesNumbers() const {
        std::vector<int> depth_nodes_numbers;
        std::function<void(const TreeNode &, int)> dfs =
            [&](const TreeNode &node, int depth) {
                if (depth >= depth_nodes_numbers.size()) {
                    depth_nodes_numbers.push_back(0);
                }
                depth_nodes_numbers[depth] += 1;
                for (const auto &ch : node.children) {
                    dfs(*ch, depth + 1);
                }
            };
        dfs(*root, 0);
        return depth_nodes_numbers;
    }

private:
    int GetTreeNodesNumbers_(const TreeNode &node) const {
        int count = 1;
        for (auto &ch : node.children) {
            count += GetTreeNodesNumbers_(*ch);
        }
        return count;
    }

    int GetTreeDepth_(const TreeNode &node) const {
        int depth = 0;
        for (auto &ch : node.children) {
            depth = std::max(depth, GetTreeDepth_(*ch));
        }
        return depth + 1;
    }


    std::shared_ptr<TreeNode> GetMostTotalRoundsChild();
    int RolloutPlay(const StateType &state, int stone_id);
    std::shared_ptr<TreeNode> SearchLeaf(std::shared_ptr<TreeNode> node);

    static StateType ConvertNumpyToStateType(const pybind11::array_t<int> &array) {
        auto buf = array.unchecked<2>(); // 2D array
        StateType state(buf.shape(0), std::vector<int>(buf.shape(1)));
        for (size_t i = 0; i < buf.shape(0); ++i) {
            for (size_t j = 0; j < buf.shape(1); ++j) {
                state[i][j] = buf(i, j);
            }
        }
        return state;
    }
};

#endif // __MONTE_CARLO_TREE_SEARCH_H__