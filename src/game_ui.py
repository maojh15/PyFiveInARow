from imgui_bundle import imgui
import numpy as np
import math

import game_config
from rich.console import Console
cons = Console()


def get_mouse_grid(left_top_coord: list[float], line_interval: float):
    mouse_x, mouse_y = imgui.get_mouse_pos()
    grid_x = math.floor((mouse_x - left_top_coord[0]) / line_interval + 0.5)
    grid_y = math.floor((mouse_y - left_top_coord[1]) / line_interval + 0.5)
    return grid_x, grid_y


def draw_board(board_state: np.ndarray, player_use_black: bool):
    io = imgui.get_io()
    draw_list: imgui.ImDrawList = imgui.get_window_draw_list()
    board_sz = int(min(*io.display_size) * 0.9)
    board_sz = (board_sz // (game_config.board_size - 1)) * (game_config.board_size - 1)
    center_coord = (io.display_size[0] / 2, io.display_size[1] / 2)

    rect_shape_pmin = [center_coord[0] - board_sz / 2, center_coord[1] - board_sz / 2]
    rect_shape_pmax = [center_coord[0] + board_sz / 2, center_coord[1] + board_sz / 2]
    line_col32 = imgui.get_color_u32((0.5,1,1,1))
    draw_list.add_rect(rect_shape_pmin, rect_shape_pmax,
                       line_col32)
    draw_list.add_rect((rect_shape_pmin[0]-5, rect_shape_pmin[1]-5),
                       (rect_shape_pmax[0]+5, rect_shape_pmax[1]+5),
                       line_col32,thickness=3)
    
    line_interval = board_sz / (game_config.board_size - 1)
    for i in range(1, game_config.board_size - 1):
        line_xpos = rect_shape_pmin[0] + i * line_interval
        draw_list.add_line((line_xpos, rect_shape_pmin[1]), (line_xpos, rect_shape_pmax[1]),
                           line_col32)
        line_ypos = rect_shape_pmin[1] + i * line_interval
        draw_list.add_line((rect_shape_pmin[0], line_ypos), (rect_shape_pmax[0], line_ypos),
                           line_col32)
    
    circle_dot_r = min(5, line_interval * 0.2)
    if game_config.board_size % 2 == 1:
        draw_list.add_circle_filled(center_coord, circle_dot_r, line_col32)
    if game_config.board_size >= 8:
        corner_dot = (game_config.board_size // 4) * line_interval

        draw_list.add_circle_filled((rect_shape_pmin[0] + corner_dot, rect_shape_pmin[1] + corner_dot),
                                    circle_dot_r, line_col32)
        draw_list.add_circle_filled((rect_shape_pmin[0] + corner_dot, rect_shape_pmax[1] - corner_dot),
                                    circle_dot_r, line_col32)
        draw_list.add_circle_filled((rect_shape_pmax[0] - corner_dot, rect_shape_pmin[1] + corner_dot),
                                    circle_dot_r, line_col32)
        draw_list.add_circle_filled((rect_shape_pmax[0] - corner_dot, rect_shape_pmax[1] - corner_dot),
                                    circle_dot_r, line_col32)
    

    # draw stones
    black_col = imgui.get_color_u32((0,0,0,1))
    white_col = imgui.get_color_u32((1,1,1,1))
    stone_r = line_interval * 0.46

    # draw stone at mouse postion for hint
    grid_x, grid_y = get_mouse_grid([rect_shape_pmin[0], rect_shape_pmin[1]], line_interval)
    if grid_x >= 0 and grid_x < game_config.board_size and \
        grid_y >= 0 and grid_y < game_config.board_size:
        stone_pos = [rect_shape_pmin[0] + grid_x * line_interval, rect_shape_pmin[1] + grid_y * line_interval]
        draw_list.add_circle_filled((stone_pos[0], stone_pos[1]),
                                    stone_r,
                                    black_col if player_use_black else white_col)
        rect_w = line_interval / 2
        draw_list.add_rect((stone_pos[0] - rect_w, stone_pos[1] - rect_w),
                           (stone_pos[0] + rect_w, stone_pos[1] + rect_w),
                           imgui.get_color_u32((1,0,0,1)))

    # draw stones on board
    for i in range(board_state.shape[0]):
        for j in range(board_state.shape[1]):
            if board_state[i, j] == 0:
                continue
            draw_list.add_circle_filled((rect_shape_pmin[0] + i * line_interval,
                                         rect_shape_pmin[1] + j * line_interval),
                                        stone_r,
                                        black_col if board_state[i,j] == 1 else white_col)

    return rect_shape_pmin, rect_shape_pmax