from rich.console import Console
cons = Console()
import numpy as np

import game_config


def check_is_game_end(board_state: np.ndarray, last_move_pos: tuple[int, int]):
    """
    Return: 0: not end; 1: win; 2. fair
    """
    if np.min(board_state) > 0:
        return 2

    pos_x, pos_y = last_move_pos
    stone_id = board_state[pos_x, pos_y]
    target_num = 5
    # check row
    count = 1
    for i in range(1, target_num):
        if pos_x + i >= game_config.board_size or board_state[pos_x + i, pos_y] != stone_id:
            break
        count += 1
    for i in range(1, target_num):
        if pos_x - i < 0 or board_state[pos_x-i,pos_y] != stone_id:
            break
        count += 1
    if count >= target_num:
        return 1
    
    # check col
    count = 1
    for i in range(1, target_num):
        if pos_y + i >= game_config.board_size or board_state[pos_x, pos_y+i] != stone_id:
            break
        count += 1
    for i in range(1, target_num):
        if pos_y - i < 0 or board_state[pos_x,pos_y-i] != stone_id:
            break
        count += 1
    if count >= target_num:
        return 1
    
    # check skew [1,1]
    count = 1
    for i in range(1, target_num):
        if pos_y + i >= game_config.board_size or pos_x+i >= game_config.board_size or board_state[pos_x+i, pos_y+i] != stone_id:
            break
        count += 1
    for i in range(1, target_num):
        if pos_y - i < 0 or pos_x-i < 0 or board_state[pos_x-i,pos_y-i] != stone_id:
            break
        count += 1
    if count >= target_num:
        return 1

    # check skew [1,-1]
    count = 1
    for i in range(1, target_num):
        if pos_y + i >= game_config.board_size or pos_x-i<0 or board_state[pos_x-i, pos_y+i] != stone_id:
            break
        count += 1
    for i in range(1, target_num):
        if pos_y - i < 0 or pos_x+i >= game_config.board_size or board_state[pos_x+i,pos_y-i] != stone_id:
            break
        count += 1
    if count >= target_num:
        return 1
    
    return 0



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
                return 1
        row_count1[0] = 1 if board_state[row, 0] == stone_id else 0
        row_count2[-1] = 1 if board_state[row, -1] == stone_id else 0
    return 0
                
        
def uniform_playout_policy(board_state: np.ndarray, stone_id: int):
    """
    @return: 0 -> draw, 1 -> win, 2 -> loss
    """
    board = board_state.copy()
    empty_pos = []
    for i in range(game_config.board_size):
        for j in range(game_config.board_size):
            if board[i,j] == 0:
                empty_pos.append((i, j))
    cur_stone_id = stone_id
    for step in range(len(empty_pos)):
        rand_idx = np.random.randint(step, len(empty_pos))
        empty_pos[step], empty_pos[rand_idx] = empty_pos[rand_idx], empty_pos[step]
        board[*(empty_pos[step])] = cur_stone_id
        game_res = check_is_game_end(board, empty_pos[step])
        if game_res == 1:
            return 1 if cur_stone_id == stone_id else 2
        if game_res == 2:
            return 0
        cur_stone_id = 3 - cur_stone_id # exchange id 1 <--> 2
    return 0
