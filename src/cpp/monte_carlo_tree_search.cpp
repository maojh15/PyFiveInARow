#include "monte_carlo_tree_search.h"
#include <pybind11/pybind11.h>

#include <algorithm>
#include <iostream>

namespace {

std::default_random_engine rand_engine;

bool IsBoardEmpty(const MonteCarloTreeSearch::StateType &state) {
    const int board_sz = state.size();
    for (int i = 0; i < board_sz; ++i) {
        for (int j = 0; j < board_sz; ++j) {
            if (state[i][j] != 0) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @return: 0 -> not end; 1 -> stone_id win; 2 -> game draw.
 */
int CheckIsGameEnd(const MonteCarloTreeSearch::StateType &state, const std::pair<int, int> &last_move_pos)
{
    const int board_sz = state.size();
    // check any empty pos left.
    bool no_empty = true;
    for (int i = 0; i < board_sz; ++i) {
        for (int j = 0; j < board_sz; ++j) {
            if (state[i][j] == 0) {
                no_empty = false;
                break;
            }
        }
        if (!no_empty) {
            break;
        }
    }
    if (no_empty) {
        return 2;
    }

    auto [pos_x, pos_y] = last_move_pos;
    int stone_id = state[pos_x][pos_y];
    const int target_num = 5;
    // check row
    int count = 1;
    for (int i = 1; i < target_num; ++i) {
        if (pos_x + i >= board_sz || state[pos_x + i][pos_y] != stone_id) {
            break;
        }
        ++count;
    }
    for (int i = 1; i < target_num; ++i) {
        if (pos_x - i < 0 || state[pos_x - i][pos_y] != stone_id) {
            break;
        }
        ++count;
    }
    if (count >= target_num) {
        return 1;
    }

    // check col
    count = 1;
    for (int i = 1; i < target_num; ++i) {
        if (pos_y + i >= board_sz || state[pos_x][pos_y+i] != stone_id) {
            break;
        }
        ++count;
    }
    for (int i = 1; i < target_num; ++i) {
        if (pos_y - i < 0 || state[pos_x][pos_y-i] != stone_id) {
            break;
        }
        ++count;
    }
    if (count >= target_num) {
        return 1;
    }

    // check skew [1, 1]
    count = 1;
    for (int i = 1; i < target_num; ++i) {
        if (pos_x+i>=board_sz || pos_y+i>=board_sz || state[pos_x+i][pos_y+i]!=stone_id) {
            break;
        }
        ++count;
    }
    for (int i = 1; i < target_num; ++i) {
        if (pos_x-i<0 || pos_y-i<0 || state[pos_x-i][pos_y-i]!=stone_id) {
            break;
        }
        ++count;
    }
    if (count >= target_num) {
        return 1;
    }

    // check skew [+1,-1]
    count = 1;
    for (int i = 1; i < target_num; ++i) {
        if (pos_x+i>=board_sz || pos_y-i<0 || state[pos_x+i][pos_y-i]!=stone_id) {
            break;
        }
        ++count;
    }
    for (int i = 1; i < target_num; ++i) {
        if (pos_x-i<0 || pos_y+i>=board_sz || state[pos_x-i][pos_y+i]!=stone_id) {
            break;
        }
        ++count;
    }
    if (count >= target_num) {
        return 1;
    }
    return 0;
}

/**
 * @return 0 -> draw, 1 -> stone_id win, 2 -> stone_id loss
 */
int UniformPlayoutPolicy(const MonteCarloTreeSearch::StateType &board_state, int stone_id)
{
    auto state = board_state;
    const int board_sz = state.size();
    std::vector<std::pair<int, int>> empty_pos;
    for (int i = 0; i < board_sz; ++i) {
        for (int j = 0; j < board_sz; ++j) {
            if (state[i][j] == 0) {
                empty_pos.emplace_back(i, j);
            }
        }
    }

    int cur_stone_id = stone_id;
    int sz = empty_pos.size();
    for (int step = 0; step < sz; ++step) {
        std::uniform_int_distribution<int> randi(step, sz - 1);
        int rand_idx = randi(rand_engine);
        std::swap(empty_pos[step], empty_pos[rand_idx]);
        state[empty_pos[step].first][empty_pos[step].second] = cur_stone_id;
        int game_res = CheckIsGameEnd(state, empty_pos[step]);
        if (game_res == 1) {
            return cur_stone_id == stone_id ? 1 : 2;
        }
        if (game_res == 2) {
            return 0;
        }
        cur_stone_id = get_opponent_id(cur_stone_id);
    }
    return 0;
}

}


std::pair<int, int> MonteCarloTreeSearch::SearchMove(int iter_steps)
{
    if (IsBoardEmpty(root.state)) {
        const int board_sz = root.state.size();
        return std::make_pair(board_sz / 2, board_sz / 2);
    }

    pybind11::gil_scoped_release gil_release;
    const int trans_game_res[3] = {0, 2, 1};
    int output_mark = iter_steps / 10;
    int percent = 0;
    for (int itr = 0; itr < iter_steps; ++itr) {
        TreeNode *leaf = Selection();
        if (leaf == nullptr) {
            continue;
        }
        int game_res = CheckIsGameEnd(leaf->state, leaf->from_moving);
        if (game_res != 0) {
            BackPropagation(*leaf, leaf->stone_id, game_res == 1 ? 1 : 0);
            continue;
        }
        game_res = RolloutPlay(leaf->state, get_opponent_id(leaf->stone_id));
        // game_res == 1 means (3 - leaf.stone_id) win, thus leaf loss game.
        BackPropagation(*leaf, leaf->stone_id, trans_game_res[game_res]);
        if (itr % output_mark == 0) {
            percent += 10;
            std::cout << percent << "% " << std::flush;
        }
    }
    std::cout << "100%" << std::endl;

    const auto &most_total_child = GetMostTotalRoundsChild();
    std::cout << "win ratio: " << most_total_child.win_rounds << "/"
              << most_total_child.total_rounds << "="
              << (most_total_child.win_rounds / most_total_child.total_rounds) << std::endl;
    return most_total_child.from_moving;
}

MonteCarloTreeSearch::TreeNode *MonteCarloTreeSearch::Selection()
{
    TreeNode &leaf = SearchLeaf(root);
    int game_res = leaf.from_moving.first < 0 ? 0 : CheckIsGameEnd(leaf.state, leaf.from_moving);
    if (game_res != 0) {
        BackPropagation(leaf, leaf.stone_id, game_res == 1 ? 1 : 0);
        return nullptr;
    }
    Expansion(leaf);
    return &(leaf.children[0]);
}

/**
 * @param game_res: 0->draw; 1->leaf_stone_id win; 2->leaf_stone_id loss.
 */
void MonteCarloTreeSearch::BackPropagation(TreeNode& node, int leaf_stone_id, int game_res)
{
    std::vector<TreeNode *> path;
    TreeNode *cur = &node;
    while (cur != nullptr) {
        switch (game_res)
        {
        case 1:
            cur->UpdateRounds(cur->stone_id == leaf_stone_id ? 1 : 0, 1);
            break;
        case 2:
            cur->UpdateRounds(cur->stone_id == leaf_stone_id ? 0 : 1, 1);
            break;
        default:
            cur->UpdateRounds(0.5, 1);
            break;
        }
        path.emplace_back(cur);
        cur = cur->parent;
    }
    for (auto &n : path) {
        n->UpdateExploitPriority();
    }
}

void MonteCarloTreeSearch::Expansion(TreeNode &leaf)
{
    int opponent_stone_id = get_opponent_id(leaf.stone_id);
    const int board_sz = leaf.state.size();
    for (int i = 0; i < board_sz; ++i) {
        for (int j = 0; j < board_sz; ++j) {
            if (leaf.state[i][j] == 0) {
                leaf.state[i][j] = opponent_stone_id;
                leaf.children.emplace_back(leaf.state, &leaf, opponent_stone_id, std::make_pair(i, j));
                leaf.state[i][j] = 0;
            }
        }
    }
    std::shuffle(leaf.children.begin(), leaf.children.end(), rand_engine);
}


MonteCarloTreeSearch::TreeNode &MonteCarloTreeSearch::SearchLeaf(
    MonteCarloTreeSearch::TreeNode &node)
{
    TreeNode *cur = &node;
    while (!cur->children.empty()) {
        double max_exploit = cur->children[0].exploit_priority;
        size_t max_arg = 0;
        for (size_t i = 1; i < cur->children.size(); ++i) {
            if (cur->children[i].exploit_priority > max_exploit) {
                max_exploit = cur->children[i].exploit_priority;
                max_arg = i;
            }
        }
        cur = &(cur->children[max_arg]);
    }
    return *cur;
}

/**
 * @return: 0 -> draw; 1 -> stone_id win; 2 -> stone_id loss.
 */
int MonteCarloTreeSearch::RolloutPlay(const StateType &state, int stone_id)
{
    return UniformPlayoutPolicy(state, stone_id);
}

/**
 * Move to most exploited nodes
 */
MonteCarloTreeSearch::TreeNode &MonteCarloTreeSearch::GetMostTotalRoundsChild()
{
    if (root.children.empty()) {
        throw std::runtime_error("No succesive state exist for root node!");
    }
    int max_total_rounds = root.children[0].total_rounds;
    size_t max_arg = 0;
    for (size_t i = 1; i < root.children.size(); ++i) {
        if (root.children[i].total_rounds > max_total_rounds) {
            max_total_rounds = root.children[i].total_rounds;
            max_arg = i;
        }
    }
    return root.children[max_arg];
}

