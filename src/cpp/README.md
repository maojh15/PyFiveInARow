# README for py_MCTS

## Overview

`py_MCTS` is a Python interface for the Monte Carlo Tree Search (MCTS) algorithm implemented in C++. This project allows users to utilize the MCTS algorithm for game-playing AI in Python, specifically for the game of Five in a Row.

## Features

- **Monte Carlo Tree Search**: Efficiently searches the game tree using random simulations.
- **Python Interface**: Exposes the MCTS functionality to Python using `pybind11`.
- **Numpy Integration**: Accepts game states as 2D numpy arrays, making it easy to interface with Python data structures.

## Installation

To build and install the project, ensure you have the following dependencies:

- CMake
- A C++ compiler (e.g., g++, clang++)
- Python 3.x
- `numpy`
- `pybind11`

### Steps to Build

1. Clone the repository:

   ```
   git clone <repository-url>
   cd py_five_in_row_pybind
   ```

2. Create a build directory:

   ```
   mkdir build
   cd build
   ```

3. Run CMake to configure the project:

   ```
   cmake ..
   ```

4. Build the project:

   ```
   make
   ```

5. Install the package:

   ```
   python setup.py install
   ```

## Usage

Once installed, you can use the Monte Carlo Tree Search in your Python code as follows:

```python
import numpy as np
from py_five_in_row_pybind import MonteCarloTreeSearch

# Create a game state as a 2D numpy array
initial_state = np.array([[0, 0, 0], [0, 0, 0], [0, 0, 0]])

# Initialize the MCTS with the game state and stone ID
mcts = MonteCarloTreeSearch(initial_state, stone_id=1)

# Perform a search for the best move
best_move = mcts.SearchMove(iter_steps=5000)
print("Best move:", best_move)
```

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.