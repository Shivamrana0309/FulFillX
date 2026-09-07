#pragma once

#include "CityGraph.h"
#include <vector>
#include <utility>

class PathFinder {
public:
    static std::pair<double, std::vector<int>> dijkstraShortestPath(const CityGraph& graph, int start_node, int end_node);
};
