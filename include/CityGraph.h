#pragma once

#include "Models.h"
#include <unordered_map>
#include <vector>
#include <string>

class CityGraph {
private:
    std::unordered_map<int, std::vector<Edge>> adjacency_list;
    std::unordered_map<int, Node> nodes;

public:
    void addNode(const Node& node);
    void addEdge(int from_node, int to_node, double distance_km, double traffic_factor);
    void updateTrafficFactor(int from_node, int to_node, double new_traffic_factor);
    std::vector<Edge> getNeighbors(int node_id) const;
    bool nodeExists(int node_id) const;
    std::string getNodeName(int node_id) const;

    size_t getNumNodes() const { return nodes.size(); }
    size_t getNumEdges() const { 
        size_t count = 0;
        for (const auto& pair : adjacency_list) count += pair.second.size();
        return count / 2;
    }
};
