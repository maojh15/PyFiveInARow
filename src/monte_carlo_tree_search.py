import numpy as np
from random import shuffle
from rich.progress import Progress
from rich.console import Console
cons = Console()

from . import game_utils

def exploit_priority(win_rounds, total_rounds, parent_node_total_rounds):
    const = 1.4142135623730951 # sqrt(2)
    if total_rounds == 0:
        return np.inf
    return win_rounds / total_rounds + \
        const * np.sqrt(np.log(parent_node_total_rounds) / total_rounds)


class TreeNode:
    def __init__(self, state: np.ndarray, parent, stone_id: int, from_moving: tuple[int, int]) -> None:
        self.state = state.copy()
        self.parent: TreeNode | None = parent
        self.stone_id = stone_id # id of last moved stone which leads to self.states
        self.from_moving = from_moving
        self.children: list[TreeNode] | None = None
        self.win_rounds = 0
        self.total_rounds = 0
        self.exploit_priority = 0
        self.update_exploit_priority()
    
    def update(self, added_win_rounds: float, added_total_rounds: int):
        self.win_rounds += added_win_rounds
        self.total_rounds += added_total_rounds

    def update_exploit_priority(self):
        self.exploit_priority = exploit_priority(self.win_rounds, self.total_rounds,
                                                 self.parent.total_rounds if self.parent is not None else 1)


class MonteCarloTreeSearch:
    def __init__(self, root_state: np.ndarray, stone_id: int) -> None:
        self.root = TreeNode(root_state, None, 3 - stone_id, None) # type: ignore
        self.stone_id = stone_id
        
    def search_move(self, iter_steps = 5000):
        trans_game_res = [0, 2, 1]
        prog = Progress()
        prog.start()
        task = prog.add_task("ai thinking", total=iter_steps)
        for itr in range(iter_steps):
            leaf = self.selection()
            if leaf is None:
                continue
            game_res = game_utils.check_is_game_end(leaf.state, leaf.from_moving)
            if game_res != 0:
                self.back_propagation(leaf, leaf.stone_id, 1 if game_res == 1 else 0)
                continue
            game_res = self.rollout_play(leaf.state, 3 - leaf.stone_id)
            # game_res == 1 means (3 - leaf.stone_id) win, thus leaf loss game.
            self.back_propagation(leaf, leaf.stone_id, trans_game_res[game_res])
            prog.advance(task)
        prog.stop()
        most_total_child = self.__get_most_total_rounds_child()
        cons.log(f"win ratio: {most_total_child.win_rounds}/{most_total_child.total_rounds}"
                 f"={most_total_child.win_rounds/most_total_child.total_rounds}")
        return most_total_child.from_moving

    
    def __get_most_total_rounds_child(self):
        """
        move to most exploited nodes
        """
        max_total_rounds = self.root.children[0].total_rounds # type: ignore
        max_arg = self.root.children[0] # type: ignore
        for ch in self.root.children: # type: ignore
            if ch.total_rounds > max_total_rounds:
                max_total_rounds = ch.total_rounds
                max_arg = ch
        return max_arg

    def rollout_play(self, board_states: np.ndarray, stone_id):
        """
        @return: 0 -> draw; 1 -> stone_id win; 2 -> stone_id loss.
        """
        return game_utils.uniform_playout_policy(board_states, stone_id)

    def selection(self) -> TreeNode | None:
        leaf = self.__search_leaf(self.root)
        game_res = game_utils.check_is_game_end(leaf.state, leaf.from_moving) if leaf.from_moving is not None else 0
        if game_res != 0:
            self.back_propagation(leaf, leaf.stone_id, 1 if game_res == 1 else 0)
            return None
        self.expansion(leaf)
        return leaf.children[0] # type: ignore

    
    def expansion(self, leaf: TreeNode):
        leaf.children = []
        # stone_id: 1 means black stone, 2 means white stone,
        # thus 3 - stone_id gives stone id of opponent.
        opponent_stone_id = 3 - leaf.stone_id
        for i in range(leaf.state.shape[0]):
            for j in range(leaf.state.shape[1]):
                if leaf.state[i,j] == 0:
                    leaf.state[i,j] = opponent_stone_id
                    leaf.children.append(TreeNode(leaf.state, leaf, opponent_stone_id, (i, j)))
                    leaf.state[i,j] = 0
        shuffle(leaf.children)
        

    def back_propagation(self, node: TreeNode | None, leaf_stone_id, game_res):
        """
        @param game_res: 0 -> draw; 1 -> leaf_stone_id win; 2 -> leaf_stone_id loss.
        """
        path: list[TreeNode] = []
        while node is not None:
            if game_res == 1: # leaf_stone_id win
                node.update(1 if node.stone_id == leaf_stone_id else 0, 1)
            elif game_res == 2:
                node.update(0 if node.stone_id == leaf_stone_id else 1, 1)
            else: # draw
                node.update(0.5, 1)
            path.append(node)
            node = node.parent
        for n in path:
            n.update_exploit_priority()
    
    def __search_leaf(self, node: TreeNode):
        while node.children is not None:
            max_val = node.children[0].exploit_priority
            max_arg = node.children[0]
            for ch in node.children:
                if ch.exploit_priority > max_val:
                    max_val = ch.exploit_priority
                    max_arg = ch
            node = max_arg
        return node
    