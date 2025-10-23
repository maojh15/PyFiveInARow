#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "monte_carlo_tree_search.h"

namespace py = pybind11;

PYBIND11_MODULE(py_MCTS, m) {
     py::class_<MonteCarloTreeSearch>(m, "MonteCarloTreeSearch",
          "Monte Carlo Tree Search object: used to search for the best move in Gomoku.")
          .def(py::init<const py::array_t<int>&, int>(),
               py::arg("root_state"), py::arg("stone_id"),
               "Constructor: root_state is the numpy array of board state, stone_id indicates current player's stone ID: 1->black; 2->white.")
          // public data members (read/write)
          .def_readwrite("near_playout_policy_distance", &MonteCarloTreeSearch::near_playout_policy_distance)
          .def_readwrite("playout_policy", &MonteCarloTreeSearch::playout_policy)
          .def("SearchMove", &MonteCarloTreeSearch::SearchMove, py::arg("iter_steps") = 5000,
               "Run MCTS search and return the best move.\nParameters: iter_steps (optional) - number of iteration steps, default 5000.\nReturn value: position or action representation of the best move.")
          .def("Selection", &MonteCarloTreeSearch::Selection,
               "Selection phase: Select nodes to expand from root node downward according to UCT policy.\nReturns: Selected node (or its index/pointer).")
          .def("Expansion", &MonteCarloTreeSearch::Expansion,
               "Expansion phase: Generate child nodes (possible moves) for the selected node.")
          .def("BackPropagation", &MonteCarloTreeSearch::BackPropagation,
               "Backpropagation phase: Propagate simulation/game result back to root node, updating win/loss statistics and visit counts for passed nodes.")
          .def("GetTreeNodesNumbers", &MonteCarloTreeSearch::GetTreeNodesNumbers,
               "Get numbers of nodes in tree")
          .def("GetTreeDepth", &MonteCarloTreeSearch::GetTreeDepth,
               "Get depth of tree")
          .def("StaticDepthNodesNumbers", &MonteCarloTreeSearch::StaticDepthNodesNumbers,
               "Return the number of nodes at each depth (static statistics). Return value is std::vector<int>, which will be converted to Python list.");

     // bind enum
     py::enum_<MonteCarloTreeSearch::PlayoutPolicy>(m, "PlayoutPolicy")
          .value("UniformPlayout", MonteCarloTreeSearch::PlayoutPolicy::UniformPlayout)
          .value("NearPlacePlayout", MonteCarloTreeSearch::PlayoutPolicy::NearPlacePlayout)
          .export_values();
    
     py::class_<MonteCarloTreeSearch::TreeNode>(m, "TreeNode")
          .def_readonly("state", &MonteCarloTreeSearch::TreeNode::state)
          .def_readonly("parent", &MonteCarloTreeSearch::TreeNode::parent)
          .def_readonly("stone_id", &MonteCarloTreeSearch::TreeNode::stone_id)
          .def_readonly("from_moving", &MonteCarloTreeSearch::TreeNode::from_moving)
          .def_readonly("win_rounds", &MonteCarloTreeSearch::TreeNode::win_rounds)
          .def_readonly("total_rounds", &MonteCarloTreeSearch::TreeNode::total_rounds)
          .def_readonly("exploit_priority", &MonteCarloTreeSearch::TreeNode::exploit_priority);
}