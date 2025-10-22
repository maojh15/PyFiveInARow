import py_MCTS

board_size = 15 # numbers of rows / cols of chess board

# config AI
tree_search_steps = 20000
list_policy = [py_MCTS.NearPlacePlayout, py_MCTS.UniformPlayout]
tree_search_policy = list_policy[0]


# UI related
screen_bg_col = [76/255,68/255,139/255]