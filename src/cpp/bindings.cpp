#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "monte_carlo_tree_search.h"

namespace py = pybind11;

PYBIND11_MODULE(py_MCTS, m) {
     py::class_<MonteCarloTreeSearch>(m, "MonteCarloTreeSearch",
          "Monte Carlo Tree Search 对象: 用于在五子棋中搜索最佳落子。")
          .def(py::init<const py::array_t<int>&, int>(),
               py::arg("root_state"), py::arg("stone_id"),
               "构造函数: root_state 为棋盘状态的 numpy 数组, stone_id 表示当前玩家的棋子 ID: 1->black; 2->white。")
          // public data members (read/write)
          .def_readwrite("near_playout_policy_distance", &MonteCarloTreeSearch::near_playout_policy_distance)
          .def_readwrite("playout_policy", &MonteCarloTreeSearch::playout_policy)
          .def("SearchMove", &MonteCarloTreeSearch::SearchMove, py::arg("iter_steps") = 5000,
               "运行 MCTS 搜索并返回最佳落子。\n参数: iter_steps (可选) - 迭代步数，默认 5000。\n返回值: 最佳落子的位置或动作表示。")
          .def("Selection", &MonteCarloTreeSearch::Selection,
               "选择阶段: 按 UCT 策略从根节点向下选择待扩展的节点。\n返回: 被选择的节点（或其索引/指针）。")
          .def("Expansion", &MonteCarloTreeSearch::Expansion,
               "扩展阶段: 对所选节点生成子节点（可行动作）。")
          .def("BackPropagation", &MonteCarloTreeSearch::BackPropagation,
               "回传阶段: 将模拟/对局结果回传到根节点，更新经过节点的胜负与访问次数统计。")
          .def("GetTreeNodesNumbers", &MonteCarloTreeSearch::GetTreeNodesNumbers,
               "Get numbers of nodes in tree")
          .def("GetTreeDepth", &MonteCarloTreeSearch::GetTreeDepth,
               "Get depth of tree")
          .def("StaticDepthNodesNumbers", &MonteCarloTreeSearch::StaticDepthNodesNumbers,
               "返回每个深度上的节点数量（静态统计）。返回值为 std::vector<int>，会被转换为 Python 列表。");

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