#include "../include/PathFinder.h"
#include <queue>
#include <unordered_map>
#include <functional>
#include <algorithm>

std::pair<double, std::vector<int>> PathFinder::dijkstraShortestPath(const CityGraph& graph, int start_node, int end_node) {
    if (!graph.nodeExists(start_node) || !graph.nodeExists(end_node)) {
        return {-1.0, {}};
    }

    // State tracks (effective_distance, node_id)
    using State = std::pair<double, int>;
    
    // Min-Heap using std::greater
    std::priority_queue<State, std::vector<State>, std::greater<State>> min_heap;

    std::unordered_map<int, double> distances;
    std::unordered_map<int, int> parent;

    min_heap.push({0.0, start_node});
    distances[start_node] = 0.0;

    while (!min_heap.empty()) {
        auto [current_dist, current_node] = min_heap.top();
        min_heap.pop();

        if (current_node == end_node) {
            break;
        }

        // Optimization: skip if we found a shorter path already
        if (distances.find(current_node) != distances.end() && current_dist > distances[current_node]) {
            continue;
        }

        for (const auto& edge : graph.getNeighbors(current_node)) {
            // Calculate effective distance
            double effective_dist = edge.distance_km * edge.traffic_factor;
            double new_total_dist = current_dist + effective_dist;

            if (distances.find(edge.to_node_id) == distances.end() || new_total_dist < distances[edge.to_node_id]) {
                distances[edge.to_node_id] = new_total_dist;
                parent[edge.to_node_id] = current_node;
                min_heap.push({new_total_dist, edge.to_node_id});
            }
        }
    }

    // If end_node wasn't reached, no path exists
    if (distances.find(end_node) == distances.end()) {
        return {-1.0, {}};
    }

    // Reconstruct exact path via reverse traversal
    std::vector<int> path;
    int curr = end_node;
    while (curr != start_node) {
        path.push_back(curr);
        curr = parent[curr];
    }
    path.push_back(start_node);
    
    std::reverse(path.begin(), path.end());

    return {distances[end_node], path};
}
