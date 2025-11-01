#include "monte_carlo_tree_search.h"
#include <pybind11/pybind11.h>
#include "pgbar/ProgressBar.hpp"
#include "pgbar/BlockBar.hpp"

#include <algorithm>
#include <iostream>
#include <set>
#include <chrono>

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


std::vector<std::pair<int, int>> GetListOfNearEmptyPlace(int pos_x, int pos_y,
    const MonteCarloTreeSearch::StateType &board_state,
    int distance = 2)
{
    std::vector<std::pair<int, int>> res;
    const int board_sz = board_state.size();
    int left = std::max(0, pos_x - distance);
    int up = std::max(0, pos_y - distance);
    int right = std::min(board_sz - 1, pos_x + distance);
    int down = std::min(board_sz - 1, pos_y + distance);
    for (int i = left; i <= right; ++i) {
        for (int j = up; j <= down; ++j) {
            if (board_state[i][j] == 0) {
                res.emplace_back(i, j);
            }
        }
    }
    return res;
}


std::set<std::pair<int,int>> ScanForEmptyPlace(const MonteCarloTreeSearch::StateType &board_state,
    int distance = 2)
{
    std::set<std::pair<int, int>> res;
    const int board_sz = board_state.size();
    auto append_space_around = [&](int pos_x, int pos_y) {
        auto list = GetListOfNearEmptyPlace(pos_x, pos_y, board_state, distance);
        for (auto &x:list) {
            res.emplace(x);
        }
    };
    
    bool empty_flag = true;
    for (int i = 0; i < board_sz; ++i) {
        for (int j = 0; j < board_sz; ++j) {
            if (board_state[i][j] != 0) {
                empty_flag = false;
                append_space_around(i, j);
            }
        }
    }
    if (empty_flag) {
        append_space_around(board_sz / 2, board_sz / 2);
    }
    return res;
}


/**
 * place stones within certain distance to stones placed on board.
 * @return 0 -> draw, 1 -> stone_id win, 2 -> stone_id loss
 */
int NearPlacePlayoutPolicy(const MonteCarloTreeSearch::StateType &board_state, int stone_id,
                           int distance = 2)
{
    auto state = board_state;
    const size_t board_sz = board_state.size();
    auto record_empty_place = ScanForEmptyPlace(board_state, distance);
    std::vector<std::pair<int, int>> candidate_place(record_empty_place.begin(), record_empty_place.end());

    int cur_stone_id = stone_id;
    while (!candidate_place.empty()) {
        int sz = candidate_place.size();
        std::uniform_int_distribution<int> rand_i(0, sz-1);
        int idx = rand_i(rand_engine);
        std::swap(candidate_place[idx], candidate_place[sz-1]);
        state[candidate_place[sz-1].first][candidate_place[sz-1].second] = cur_stone_id;

        int game_res = CheckIsGameEnd(state, candidate_place[sz-1]);
        if (game_res == 1) {
            return cur_stone_id == stone_id ? 1 : 2;
        }
        if (game_res == 2) {
            return 0;
        }

        auto empty_list = GetListOfNearEmptyPlace(candidate_place[sz-1].first, candidate_place[sz-1].second, state, distance);
        candidate_place.pop_back();
        for (auto &x: empty_list) {
            if (record_empty_place.find(x) == record_empty_place.end()) {
                record_empty_place.emplace(x);
                candidate_place.emplace_back(x);
            }
        }
        cur_stone_id = get_opponent_id(cur_stone_id);
    }
    return 0;
}

}


std::pair<int, int> MonteCarloTreeSearch::SearchMove(int iter_steps)
{
    pybind11::gil_scoped_release gil_release;
    pgbar::BlockBar<> pbar;
    pbar.config().prefix("search move ");
    pbar.config().style( pgbar::config::Line::Entire ).tasks(iter_steps);

    auto time1 = std::chrono::high_resolution_clock::now();

    const int trans_game_res[3] = {0, 2, 1};
    for (int itr = 0; itr < iter_steps; ++itr) {
        auto leaf = Selection();
        if (leaf == nullptr) {
            pbar.tick();
            continue;
        }
        int game_res = CheckIsGameEnd(leaf->state, leaf->from_moving);
        if (game_res != 0) {
            BackPropagation(leaf, leaf->stone_id, game_res == 1 ? 1 : 0);
            pbar.tick();
            continue;
        }
        game_res = RolloutPlay(leaf->state, get_opponent_id(leaf->stone_id));
        // game_res == 1 means (3 - leaf.stone_id) win, thus leaf loss game.
        BackPropagation(leaf, leaf->stone_id, trans_game_res[game_res]);
        pbar.tick();
    }

    const auto &most_total_child = GetMostTotalRoundsChild();
    auto time2 = std::chrono::high_resolution_clock::now();
    std::cout << "win ratio: " << most_total_child->win_rounds << "/"
              << most_total_child->total_rounds << "="
              << (most_total_child->win_rounds / most_total_child->total_rounds)
              << "\tcost time: " << std::chrono::duration<double>(time2-time1).count()
              << "s" << std::endl;
    for (auto &ch : root->children) {
        std::cout << "[" << ch->from_moving.first << "," << ch->from_moving.second
            << "|" << ch->win_rounds << "/" << ch->total_rounds << "="
            << (ch->win_rounds / ch->total_rounds)  <<"], ";
    }
    std::cout << std::endl;
    return most_total_child->from_moving;
}

std::shared_ptr<MonteCarloTreeSearch::TreeNode>MonteCarloTreeSearch::Selection()
{
    std::shared_ptr<TreeNode> leaf = SearchLeaf(root);
    int game_res = leaf->from_moving.first < 0 ? 0 : CheckIsGameEnd(leaf->state, leaf->from_moving);
    if (game_res != 0) {
        BackPropagation(leaf, leaf->stone_id, game_res == 1 ? 1 : 0);
        return nullptr;
    }
    Expansion(leaf);
    return leaf->children[0];
}

/**
 * @param game_res: 0->draw; 1->leaf_stone_id win; 2->leaf_stone_id loss.
 */
void MonteCarloTreeSearch::BackPropagation(std::shared_ptr<TreeNode> node, int leaf_stone_id, int game_res)
{
    std::shared_ptr<TreeNode> cur = node;
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
        cur = cur->parent;
    }
}

void MonteCarloTreeSearch::Expansion(std::shared_ptr<TreeNode> leaf)
{
    const int board_sz = leaf->state.size();
    int opponent_stone_id = get_opponent_id(leaf->stone_id);
    auto expansion_uniform = [&]() {
        for (int i = 0; i < board_sz; ++i) {
            for (int j = 0; j < board_sz; ++j) {
                if (leaf->state[i][j] == 0) {
                    leaf->state[i][j] = opponent_stone_id;
                    auto new_node = std::make_shared<TreeNode>(leaf->state, leaf, opponent_stone_id, std::make_pair(i, j));
                    leaf->children.emplace_back(new_node);
                    leaf->state[i][j] = 0;
                }
            }
        }
    };

    auto expansion_near_place = [&]() {
        auto candidates = ScanForEmptyPlace(leaf->state, near_playout_policy_distance);
        for (auto &pos : candidates) {
            leaf->state[pos.first][pos.second] = opponent_stone_id;
            auto new_node = std::make_shared<TreeNode>(leaf->state, leaf, opponent_stone_id, pos);
            leaf->children.emplace_back(new_node);
            leaf->state[pos.first][pos.second] = 0;
        }
    };

    switch (playout_policy) {
    case PlayoutPolicy::UniformPlayout:
        expansion_uniform();
        break;
    case PlayoutPolicy::NearPlacePlayout:
    default:
        expansion_near_place();
        break;
    }
    std::shuffle(leaf->children.begin(), leaf->children.end(), rand_engine);
}


std::shared_ptr<MonteCarloTreeSearch::TreeNode> MonteCarloTreeSearch::SearchLeaf(
    std::shared_ptr<MonteCarloTreeSearch::TreeNode> node)
{
    std::shared_ptr<TreeNode> cur = node;
    while (!cur->children.empty()) {
        cur = *std::max_element(cur->children.begin(), cur->children.end(),
            [](const std::shared_ptr<TreeNode> &a, const std::shared_ptr<TreeNode> &b) {
                return a->GetExploitPriority() < b->GetExploitPriority();
            });
    }
    return cur;
}

/**
 * @return: 0 -> draw; 1 -> stone_id win; 2 -> stone_id loss.
 */
int MonteCarloTreeSearch::RolloutPlay(const StateType &state, int stone_id)
{
    switch (playout_policy)
    {
    case PlayoutPolicy::UniformPlayout:
        return UniformPlayoutPolicy(state, stone_id);
    case PlayoutPolicy::NearPlacePlayout:
    default:
        return NearPlacePlayoutPolicy(state, stone_id, near_playout_policy_distance);
    }
}

/**
 * Move to most exploited nodes
 */
std::shared_ptr<MonteCarloTreeSearch::TreeNode> MonteCarloTreeSearch::GetMostTotalRoundsChild()
{
    if (root->children.empty()) {
        throw std::runtime_error("No succesive state exist for root node!");
    }
    int max_total_rounds = root->children[0]->total_rounds;
    size_t max_arg = 0;
    for (size_t i = 1; i < root->children.size(); ++i) {
        if (root->children[i]->total_rounds > max_total_rounds) {
            max_total_rounds = root->children[i]->total_rounds;
            max_arg = i;
        }
    }
    return root->children[max_arg];
}

