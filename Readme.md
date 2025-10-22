# py_five_in_row

Simple Python implementation of a Five-in-a-Row (Gomoku) game with configurable players (human, local AI). With Monte Carlo Tree Search implemented for AI. Designed as a small, readable codebase for learning and extension.

## Features
- Turn-based Gomoku game on configurable board size
- Human vs AI
- Clear, modular code for game logic and board management

## Requirements
- Python 3.8+
- imgui_bundle
- pysdl2
- rich
  
pybind11 is required to compile the C++ code that implements Monte Carlo Tree Search.

## Quick start
1. Clone the repository:
   
    `git clone https://github.com/maojh15/PyFiveInARow.git`
2. Install dependencies (if any):
   
    `python -m pip install -r requirements.txt`
3. Compile c++ code for AI:
   
    `mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j && make install`

## Run
- From project root:
  - If there is a top-level runner:
     `python main.py`
  - Or as a package:
     `python -m py_five_in_row`
- Choose **board size** and config **AI parameters** in `game_config.py`.

## Contributing
- Open issues for bugs or feature requests
- Create feature branches, add tests, and submit pull requests

## Contact
Project maintained in-repo. Use issues or PRs for discussion.
