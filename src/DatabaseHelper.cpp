#include "../include/DatabaseHelper.h"
#include <iostream>
#include <sstream>

DatabaseHelper::DatabaseHelper() : db(nullptr) {}

DatabaseHelper::~DatabaseHelper() {
    if (db) {
        sqlite3_close(db);
    }
}

bool DatabaseHelper::initializeSchema(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Aligned entirely with seed.sql schema
    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS Locations (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            type TEXT NOT NULL CHECK(type IN ('WAREHOUSE', 'RESIDENTIAL', 'COMMERCIAL'))
        );

        CREATE TABLE IF NOT EXISTS Roads (
            source_id INTEGER NOT NULL,
            dest_id INTEGER NOT NULL,
            distance_km REAL NOT NULL,
            traffic_multiplier REAL DEFAULT 1.0,
            PRIMARY KEY (source_id, dest_id),
            FOREIGN KEY (source_id) REFERENCES Locations(id),
            FOREIGN KEY (dest_id) REFERENCES Locations(id)
        );

        CREATE TABLE IF NOT EXISTS Packages (
            id INTEGER PRIMARY KEY,
            dest_location_id INTEGER NOT NULL,
            priority_level INTEGER NOT NULL CHECK(priority_level BETWEEN 1 AND 5),
            deadline_timestamp INTEGER NOT NULL,
            status TEXT NOT NULL CHECK(status IN ('PENDING', 'DISPATCHED', 'DELIVERED', 'CANCELLED')),
            FOREIGN KEY (dest_location_id) REFERENCES Locations(id)
        );

        CREATE TABLE IF NOT EXISTS Delivery_Logs (
            log_id INTEGER PRIMARY KEY AUTOINCREMENT,
            driver_id INTEGER NOT NULL,
            route_path TEXT NOT NULL,
            delivered_package_ids TEXT NOT NULL,
            total_distance_km REAL NOT NULL,
            dispatch_timestamp INTEGER NOT NULL
        );
    )";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, schema, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Schema Init Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseHelper::loadGraph(CityGraph& graph) {
    if (!db) return false;

    // Load Locations using the correct column 'type'
    const char* node_query = "SELECT id, name, type FROM Locations;";
    sqlite3_stmt* node_stmt;
    if (sqlite3_prepare_v2(db, node_query, -1, &node_stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL Error in loadGraph (Locations): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    while (sqlite3_step(node_stmt) == SQLITE_ROW) {
        Node node;
        node.id = sqlite3_column_int(node_stmt, 0);
        
        const unsigned char* nameText = sqlite3_column_text(node_stmt, 1);
        const unsigned char* typeText = sqlite3_column_text(node_stmt, 2);
        
        node.name = nameText ? reinterpret_cast<const char*>(nameText) : "";
        node.node_type = typeText ? reinterpret_cast<const char*>(typeText) : "";
        
        graph.addNode(node);
    }
    sqlite3_finalize(node_stmt);

    // Load Roads using correct columns 'source_id', 'dest_id', 'traffic_multiplier'
    const char* edge_query = "SELECT source_id, dest_id, distance_km, traffic_multiplier FROM Roads;";
    sqlite3_stmt* edge_stmt;
    if (sqlite3_prepare_v2(db, edge_query, -1, &edge_stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL Error in loadGraph (Roads): " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    while (sqlite3_step(edge_stmt) == SQLITE_ROW) {
        int from_node = sqlite3_column_int(edge_stmt, 0);
        int to_node = sqlite3_column_int(edge_stmt, 1);
        double dist = sqlite3_column_double(edge_stmt, 2);
        double traffic = sqlite3_column_double(edge_stmt, 3);
        
        graph.addEdge(from_node, to_node, dist, traffic);
    }
    sqlite3_finalize(edge_stmt);

    return true;
}

bool DatabaseHelper::loadPendingPackages(PackageTree& tree) {
    if (!db) return false;

    // Fixed 'dest_node_id' to 'dest_location_id'
    const char* query = "SELECT id, dest_location_id, priority_level, deadline_timestamp, status FROM Packages WHERE status = 'PENDING';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL Error in loadPendingPackages: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Package pkg;
        pkg.id = sqlite3_column_int(stmt, 0);
        pkg.dest_node_id = sqlite3_column_int(stmt, 1);
        pkg.priority_level = sqlite3_column_int(stmt, 2);
        pkg.deadline_timestamp = sqlite3_column_int64(stmt, 3);
        pkg.status = PackageStatus::PENDING; 
        tree.insert(pkg);
    }
    sqlite3_finalize(stmt);

    return true;
}

bool DatabaseHelper::savePackage(const Package& pkg) {
    if (!db) return false;

    // Fixed 'dest_node_id' to 'dest_location_id'
    const char* query = "INSERT INTO Packages (id, dest_location_id, priority_level, deadline_timestamp, status) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL Error in savePackage: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, pkg.id);
    sqlite3_bind_int(stmt, 2, pkg.dest_node_id);
    sqlite3_bind_int(stmt, 3, pkg.priority_level);
    sqlite3_bind_int64(stmt, 4, pkg.deadline_timestamp);
    
    // Status is now TEXT
    std::string statusStr = "PENDING";
    if (pkg.status == PackageStatus::DISPATCHED) statusStr = "DISPATCHED";
    else if (pkg.status == PackageStatus::DELIVERED) statusStr = "DELIVERED";
    else if (pkg.status == PackageStatus::CANCELLED) statusStr = "CANCELLED";
    
    sqlite3_bind_text(stmt, 5, statusStr.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Execution Error in savePackage: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseHelper::updatePackageStatus(int package_id, PackageStatus status) {
    if (!db) return false;

    const char* query = "UPDATE Packages SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL Error in updatePackageStatus: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    std::string statusStr = "PENDING";
    if (status == PackageStatus::DISPATCHED) statusStr = "DISPATCHED";
    else if (status == PackageStatus::DELIVERED) statusStr = "DELIVERED";
    else if (status == PackageStatus::CANCELLED) statusStr = "CANCELLED";

    sqlite3_bind_text(stmt, 1, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, package_id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Execution Error in updatePackageStatus: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseHelper::logDelivery(const DeliveryRoute& route) {
    if (!db) return false;

    // Fixed 'path_nodes' to 'route_path' and supplied default 'dispatch_timestamp'
    const char* query = "INSERT INTO Delivery_Logs (driver_id, route_path, delivered_package_ids, total_distance_km, dispatch_timestamp) VALUES (?, ?, ?, ?, strftime('%s', 'now'));";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL Error in logDelivery: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    auto join = [](const std::vector<int>& v) {
        std::ostringstream oss;
        for (size_t i = 0; i < v.size(); ++i) {
            if (i > 0) oss << ",";
            oss << v[i];
        }
        return oss.str();
    };

    std::string path_str = join(route.path_nodes);
    std::string pkgs_str = join(route.delivered_package_ids);

    sqlite3_bind_int(stmt, 1, route.driver_id);
    sqlite3_bind_text(stmt, 2, path_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, pkgs_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, route.total_distance_km);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Execution Error in logDelivery: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}
