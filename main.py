from rich.console import Console
cons = Console()

from sdl2 import *
from imgui_bundle import imgui
from imgui_bundle.python_backends.sdl2_backend import SDL2Renderer

import ctypes
import sys
import OpenGL.GL as gl
import importlib

GameUIMod = importlib.import_module("src.game_ui")
# from src.game import Game
GameMod = importlib.import_module("src.game")
Game = GameMod.Game
import game_config


def impl_pysdl2_init(window_name = "minimal ImGui/SDL2 example"):
    width, height = 1280, 720
    width //= 2
    height //= 2

    if SDL_Init(SDL_INIT_EVERYTHING) < 0:
        print(
            "Error: SDL could not initialize! SDL Error: "
            + SDL_GetError().decode("utf-8")
        )
        sys.exit(1)

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24)
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)

    SDL_SetHint(SDL_HINT_MAC_CTRL_CLICK_EMULATE_RIGHT_CLICK, b"1")
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, b"1")

    window = SDL_CreateWindow(
        window_name.encode("utf-8"),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE,
    )

    if window is None:
        print(
            "Error: Window could not be created! SDL Error: "
            + SDL_GetError().decode("utf-8")
        )
        sys.exit(1)

    gl_context = SDL_GL_CreateContext(window)
    if gl_context is None:
        print(
            "Error: Cannot create OpenGL Context! SDL Error: "
            + SDL_GetError().decode("utf-8")
        )
        sys.exit(1)

    SDL_GL_MakeCurrent(window, gl_context)
    if SDL_GL_SetSwapInterval(1) < 0:
        print(
            "Warning: Unable to set VSync! SDL Error: " + SDL_GetError().decode("utf-8")
        )
        sys.exit(1)

    return window, gl_context

def main():
    window, gl_context = impl_pysdl2_init("Five in Row")
    imgui.create_context()
    impl = SDL2Renderer(window)

    running = True
    event = SDL_Event()
    ms_per_frame = 1000/60

    game = Game()
    imgui_io: imgui.IO = imgui.get_io()
    while running and game.status != Game.GameStatus.EXIT:
        tick = SDL_GetTicks64()
        while SDL_PollEvent(ctypes.byref(event)) != 0:
            if event.type == SDL_QUIT or \
                (event.type == SDL_KEYDOWN and event.key.keysym.scancode == SDL_SCANCODE_Q):
                running = False
                break
            impl.process_event(event)
            if imgui_io.want_capture_mouse:
                continue
            if event.type == SDL_MOUSEBUTTONDOWN and event.button.button == 1:
                game.click_event()
        impl.process_inputs()

        imgui.new_frame()

        # render ui
        game.render_ui()

        # debug layout
        # if imgui.button("debug", (50, 0)):
        #     game.log()
        # if imgui.button("reload game module"):
        #     importlib.reload(GameUIMod)
        #     importlib.reload(GameMod)
        #     game = GameMod.Game()
        # changed, game_config.screen_bg_col = imgui.color_edit3("bg col", game_config.screen_bg_col)

        # end debug layout

        gl.glClearColor(*game_config.screen_bg_col, 1.0)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)

        imgui.render()
        impl.render(imgui.get_draw_data())
        SDL_GL_SwapWindow(window)

        tick2 = SDL_GetTicks64()
        if (tick2 - tick < ms_per_frame):
            SDL_Delay(int(ms_per_frame - (tick2 - tick)))


    impl.shutdown()
    SDL_GL_DeleteContext(gl_context)
    SDL_DestroyWindow(window)
    SDL_Quit()


if __name__ == "__main__":
    main()
