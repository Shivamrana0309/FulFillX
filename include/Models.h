#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class PackageStatus { PENDING, DISPATCHED, DELIVERED, CANCELLED };

struct Edge {
    int to_node_id;
    double distance_km;
    double traffic_factor;
};

struct Node {
    int id;
    std::string name;
    std::string node_type;
};

struct Package {
    int id;
    int dest_node_id;
    int priority_level;
    int64_t deadline_timestamp;
    PackageStatus status;
};

struct RouteLeg {
    int from_node;
    int to_node;
    double leg_distance;
};

struct DeliveryRoute {
    int driver_id;
    std::vector<int> path_nodes;
    std::vector<int> delivered_package_ids;
    double total_distance_km;
};
