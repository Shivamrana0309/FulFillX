#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

// =======================================================================
// JSON Serialization Definitions
// =======================================================================
NLOHMANN_JSON_SERIALIZE_ENUM(PackageStatus, {
    {PackageStatus::PENDING, "PENDING"},
    {PackageStatus::DISPATCHED, "DISPATCHED"},
    {PackageStatus::DELIVERED, "DELIVERED"},
    {PackageStatus::CANCELLED, "CANCELLED"}
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Node, id, name, node_type)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Package, id, dest_node_id, priority_level, deadline_timestamp, status)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RouteLeg, from_node, to_node, leg_distance)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeliveryRoute, driver_id, path_nodes, delivered_package_ids, total_distance_km)
