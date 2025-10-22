import numpy as np
import time
from rich.console import Console
cons = Console()

import game_config
from . import monte_carlo_tree_search
import py_MCTS

def AI_step(board_states: np.ndarray, ai_stone_id: int):
    time1 = time.time()
    # tree = monte_carlo_tree_search.MonteCarloTreeSearch(board_states, ai_stone_id)
    # best_move = tree.search_move(50000)

    tree = py_MCTS.MonteCarloTreeSearch(board_states, ai_stone_id)
    tree.playout_policy = game_config.tree_search_policy
    tree.near_playout_policy_distance = 1
    best_move = tree.SearchMove(game_config.tree_search_steps)

    time2 = time.time()
    cons.log(f"search step cost time: {time2-time1}s.")
    cons.log(f"tree size: {tree.GetTreeNodesNumbers()}, depth: {tree.GetTreeDepth()}")
    cons.log(f"num nodes in each depth: {tree.StaticDepthNodesNumbers()}")
    return best_move