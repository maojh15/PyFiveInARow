import numpy as np
import time

import game_config

def AI_step(board_states: np.ndarray, ai_stone_id: int):
    # time.sleep(0.5)
    while True:
        pos_x = np.random.randint(game_config.board_size)
        pos_y = np.random.randint(game_config.board_size)
        if board_states[pos_x, pos_y] == 0:
            return pos_x, pos_y