from enum import Enum, auto
import numpy as np
from concurrent.futures import ThreadPoolExecutor

from imgui_bundle import imgui
import sdl2

from rich.console import Console
cons = Console()

from .singleton import Singleton
import game_config
from . import game_ui
from . import game_AI
from . import utils
from . import game_utils

class Game(Singleton):
    class GameStatus(Enum):
        PLAYING = 1
        GAME_END = auto()
        EXIT = auto()
    
    # board state: 0: empty, 1: black stone, 2: white stone
    board_state = np.zeros((game_config.board_size, game_config.board_size), dtype=np.int8)
    status = GameStatus.PLAYING
    who_first = 0 # 0: player first, 1: AI first
    players_turn = True
    next_game_who_first = 0
    record_stone_places = []

    last_tick = -1
    board_shape = [[0,0],[0,0]]
    thread_pool = ThreadPoolExecutor(1)
    thread_future = None
    winner = None

    def log(self):
        for i in range(self.board_state.shape[0]):
            for j in range(self.board_state.shape[1]):
                self.board_state[i, j] = np.random.randint(3)

    
    def render_ui(self):
        # draw board
        self.board_shape = game_ui.draw_board(self.board_state, self.who_first == 0,
                                              None if len(self.record_stone_places) == 0 else self.record_stone_places[-1])

        # draw ui buttons and hint text
        self.draw_buttons_ui()

        if self.status == Game.GameStatus.PLAYING:
            if not self.players_turn and not self.get_AI_step():
                self.draw_ai_thinking_hint()
        if self.status == Game.GameStatus.GAME_END:
            if self.winner is None:
                text = "The game ending in a draw!"
            else:
                text = "You win!" if self.winner == self.who_first + 1 else "You loss!"
            res = game_ui.show_game_result(text)
            if res == 0:
                self.reset_game_status()
            elif res == 1:
                self.status = Game.GameStatus.EXIT
                cons.log(f"self.status={self.status}")


    def draw_ai_thinking_hint(self):
        imgui_io = imgui.get_io()
        imgui.open_popup("AI Thinking")
        popup_sz = (200, 80)
        imgui.set_next_window_size(popup_sz)
        if imgui.begin_popup_modal("AI Thinking", None, imgui.WindowFlags_.no_resize)[0]:
            imgui.text("AI Thinking ... ")
            imgui.end_popup()
        

    def start_AI_step(self):
        self.thread_future = self.thread_pool.submit(game_AI.AI_step, self.board_state, 2-self.who_first)
        cons.log(f"start AI step")

    def get_AI_step(self):
        if self.status != Game.GameStatus.PLAYING:
            return
        if self.thread_future is None:
            # 没有异步任务，启动一个并返回（非阻塞）
            self.start_AI_step()
            return False

        # 检查线程是否完成，未完成则直接返回（等待下一帧再检查）
        if not self.thread_future.done():
            return False
        cons.log(f"thread task done")
        # 任务已完成，取出结果并处理异常
        pos_x, pos_y = self.thread_future.result()
        self.thread_future = None
        self.place_stone(pos_x, pos_y, 2 - self.who_first)
        self.players_turn = True
        return True


    def reset_game_status(self):
        self.status = Game.GameStatus.PLAYING
        self.board_state = np.zeros_like(self.board_state, dtype=self.board_state.dtype)
        self.who_first = self.next_game_who_first
        self.players_turn = (self.who_first == 0)
        self.record_stone_places = []
        self.winner = None
        if not self.players_turn:
            self.get_AI_step()


    def draw_buttons_ui(self):
        imgui.set_next_window_pos((5, 5))
        imgui.set_next_window_size((140, 230), imgui.Cond_.always)
        imgui.begin("UI", None,
                    imgui.WindowFlags_.no_decoration |
                    imgui.WindowFlags_.no_move | imgui.WindowFlags_.no_collapse |
                    imgui.WindowFlags_.no_title_bar | imgui.WindowFlags_.no_scrollbar |
                    imgui.WindowFlags_.no_scroll_with_mouse | imgui.WindowFlags_.no_bring_to_front_on_focus)
        # show fps
        cur_tick = sdl2.SDL_GetTicks64()
        imgui.text(f"fps: {1000.0 / (cur_tick - self.last_tick):.3f}")
        self.last_tick = cur_tick

        btn_sz = (100, 50)
        if imgui.button(" New Game ", btn_sz):
            self.reset_game_status()
        

        hint = ["Player first", "AI first"]
        imgui.text_colored((1, 1, 0, 0.8), "Who first?")
        imgui.same_line()
        utils.show_help_marker("Take effect in next game")
        for n in range(len(hint)):
            clicked, _ = imgui.checkbox(hint[n], v=(self.next_game_who_first == n))
            if clicked:
                self.next_game_who_first = n

        if imgui.button("withdraw a move", (120, btn_sz[1])):
            self.withdraw_a_move()

        imgui.end()

    def withdraw_a_move(self):
        for i in range(2):
            if len(self.record_stone_places) > 0:
                pos = self.record_stone_places[-1]
                self.board_state[pos[0], pos[1]] = 0
                self.record_stone_places.pop()
        if len(self.record_stone_places) == 0 and self.who_first == 1:
            self.players_turn = False
            self.get_AI_step()
            

    def click_event(self):
        if self.status != Game.GameStatus.PLAYING:
            return
        if not self.players_turn:
            return
        grid_x, grid_y = game_ui.get_mouse_grid([self.board_shape[0][0], self.board_shape[0][1]],
                                                (self.board_shape[1][0]-self.board_shape[0][0]) / (game_config.board_size - 1))
        if grid_x >= 0 and grid_x < game_config.board_size and \
            grid_y >= 0 and grid_y < game_config.board_size and \
                self.board_state[grid_x, grid_y] == 0:
            self.place_stone(grid_x, grid_y, 1 + self.who_first)
            self.players_turn = False
            self.get_AI_step()

    def place_stone(self, pos_x, pos_y, stone):
        self.board_state[pos_x, pos_y] = stone
        self.record_stone_places.append([pos_x, pos_y])
        check_res = game_utils.check_is_game_end(self.board_state, (pos_x, pos_y))
        if check_res == 0:
            return
        self.status = self.GameStatus.GAME_END
        if check_res == 1:
            self.winner = stone