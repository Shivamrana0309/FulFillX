#include "../include/CityGraph.h"

void CityGraph::addNode(const Node& node) {
    nodes[node.id] = node;
}

void CityGraph::addEdge(int from_node, int to_node, double distance_km, double traffic_factor) {
    // Bidirectional edge
    adjacency_list[from_node].push_back({to_node, distance_km, traffic_factor});
    adjacency_list[to_node].push_back({from_node, distance_km, traffic_factor});
}

void CityGraph::updateTrafficFactor(int from_node, int to_node, double new_traffic_factor) {
    // Update both directions
    if (adjacency_list.find(from_node) != adjacency_list.end()) {
        for (auto& edge : adjacency_list[from_node]) {
            if (edge.to_node_id == to_node) {
                edge.traffic_factor = new_traffic_factor;
            }
        }
    }
    if (adjacency_list.find(to_node) != adjacency_list.end()) {
        for (auto& edge : adjacency_list[to_node]) {
            if (edge.to_node_id == from_node) {
                edge.traffic_factor = new_traffic_factor;
            }
        }
    }
}

std::vector<Edge> CityGraph::getNeighbors(int node_id) const {
    auto it = adjacency_list.find(node_id);
    if (it != adjacency_list.end()) {
        return it->second;
    }
    return {};
}

bool CityGraph::nodeExists(int node_id) const {
    return nodes.find(node_id) != nodes.end();
}

std::string CityGraph::getNodeName(int node_id) const {
    auto it = nodes.find(node_id);
    if (it != nodes.end()) {
        return it->second.name;
    }
    return "";
}
