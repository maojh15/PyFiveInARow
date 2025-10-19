from rich.console import Console
cons = Console()
import numpy as np

import game_config


def is_game_end(board_state: np.ndarray, stone_id: int):
    """
    Return: 0: not end; 1: win; 2. fair
    """
    if np.min(board_state) > 0:
        return 2

    target_num = 5
    count = 0
    # check row
    for row in range(game_config.board_size):
        count = 1 if board_state[row, 0] == stone_id else 0
        for col in range(1, game_config.board_size):
            if board_state[row, col] != stone_id:
                count = 0
            else:
                count += 1
                if count == target_num:
                    cons.log("check row success")
                    return 1
    # check col
    for col in range(game_config.board_size):
        count = 1 if board_state[0, col] == stone_id else 0
        for row in range(1, game_config.board_size):
            if board_state[row, col] != stone_id:
                count = 0
            else:
                count += 1
                if count == target_num:
                    cons.log("check col success")
                    return 1
    
    # check skew line
    row_count1 = np.zeros(game_config.board_size) # check skew line [+1,+1]
    row_count2 = np.zeros(game_config.board_size) # check skew line [+1, -1]
    for col in range(game_config.board_size):
        row_count1[col] = 1 if board_state[0, col] == stone_id else 0
        row_count2[col] = 1 if board_state[0, col] == stone_id else 0
    for row in range(1, game_config.board_size):
        for col in range(game_config.board_size-1, 0, -1):
            row_count1[col] = 0 if board_state[row, col] != stone_id else row_count1[col-1] + 1
            row_count2[-col-1] = 0 if board_state[row, -col-1] != stone_id else row_count2[-col] + 1
            if row_count1[col] == target_num or row_count2[-col-1] == target_num:
                cons.log("check skew line success")
                return 1
        row_count1[0] = 1 if board_state[row, 0] == stone_id else 0
        row_count2[-1] = 1 if board_state[row, -1] == stone_id else 0
    return 0
                
        
