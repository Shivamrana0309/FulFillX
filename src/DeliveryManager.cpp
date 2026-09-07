#include "../include/DeliveryManager.h"
#include <iostream>
#include <algorithm>
#include <sstream>

bool DeliveryManager::bootSystem(const std::string& db_path) {
    if (!db.initializeSchema(db_path)) return false;
    if (!db.loadGraph(graph)) return false;
    if (!db.loadPendingPackages(tree)) return false;

    std::cout << "[System] Connected to DB. Loaded " << graph.getNumNodes() 
              << " locations and " << graph.getNumEdges() << " roads.\n";
    return true;
}

void DeliveryManager::handleAddPackage(int id, int dest, int prio, int64_t deadline) {
    if (!graph.nodeExists(dest)) {
        std::cerr << "[System] Warning: Destination node " << dest << " does not exist.\n";
    }

    Package pkg;
    pkg.id = id;
    pkg.dest_node_id = dest;
    pkg.priority_level = prio;
    pkg.deadline_timestamp = deadline;
    pkg.status = PackageStatus::PENDING;

    if (db.savePackage(pkg)) {
        tree.insert(pkg);
        std::cout << "[System] Package " << id << " added. Destination: " << graph.getNodeName(dest) << ". Priority: " << prio << ".\n";
    } else {
        std::cerr << "[System] Failed to save package " << id << " to database.\n";
    }
}

void DeliveryManager::handleDispatch(int driver_id, int warehouse_node, int64_t deadline_threshold) {
    if (!graph.nodeExists(warehouse_node)) {
        std::cerr << "[System] Error: Warehouse node " << warehouse_node << " does not exist.\n";
        return;
    }

    std::cout << "[System] Dispatching Driver #" << driver_id << " from " << graph.getNodeName(warehouse_node) << " (Node " << warehouse_node << ")...\n";

    std::vector<Package> pkgs = tree.getPackagesDueBefore(deadline_threshold);
    std::cout << "[System] Pulled " << pkgs.size() << " packages from Priority BST.\n";

    if (pkgs.empty()) {
        return;
    }

    std::sort(pkgs.begin(), pkgs.end(), [](const Package& a, const Package& b) {
        return a.deadline_timestamp < b.deadline_timestamp;
    });

    std::cout << "[System] Calculating optimal route using Dijkstra...\n";

    DeliveryRoute route;
    route.driver_id = driver_id;
    route.total_distance_km = 0.0;
    route.path_nodes.push_back(warehouse_node);

    int current_node = warehouse_node;
    
    std::ostringstream route_stream;
    route_stream << graph.getNodeName(warehouse_node);

    for (const auto& pkg : pkgs) {
        if (current_node == pkg.dest_node_id) {
            route.delivered_package_ids.push_back(pkg.id);
            db.updatePackageStatus(pkg.id, PackageStatus::DISPATCHED);
            tree.remove(pkg.id);
            route_stream << " (Drop " << pkg.id << ")";
            continue;
        }

        auto [dist, path] = PathFinder::dijkstraShortestPath(graph, current_node, pkg.dest_node_id);
        
        if (dist < 0) {
            std::cerr << "[System] Warning: No path from " << current_node << " to " << pkg.dest_node_id << " for package " << pkg.id << ".\n";
            continue; 
        }

        for (size_t i = 1; i < path.size(); ++i) {
            route.path_nodes.push_back(path[i]);
            route_stream << " -> " << graph.getNodeName(path[i]);
            if (i == path.size() - 1) { // reached destination
                route_stream << " (Drop " << pkg.id << ")";
            }
        }
        
        route.total_distance_km += dist;
        route.delivered_package_ids.push_back(pkg.id);
        
        db.updatePackageStatus(pkg.id, PackageStatus::DISPATCHED);
        tree.remove(pkg.id);
        
        current_node = pkg.dest_node_id;
    }

    if (route.delivered_package_ids.empty()) {
        std::cout << "[System] No packages were successfully dispatched.\n";
        return;
    }

    db.logDelivery(route);

    std::cout << "[System] Route generated: " << route_stream.str() << "\n";
    std::cout << "[System] Total Route Distance: " << route.total_distance_km << " km.\n";
}

void DeliveryManager::listPending() const {
    std::cout << "[System] " << tree.size() << " pending packages in the queue.\n";
}
