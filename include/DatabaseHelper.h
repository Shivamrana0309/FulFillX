#pragma once

#include "Models.h"
#include "CityGraph.h"
#include "PackageTree.h"
#include <string>
#include <sqlite3.h>

class DatabaseHelper {
private:
    sqlite3* db;

public:
    DatabaseHelper();
    ~DatabaseHelper();

    // Disable copy for safety around db pointer
    DatabaseHelper(const DatabaseHelper&) = delete;
    DatabaseHelper& operator=(const DatabaseHelper&) = delete;

    bool initializeSchema(const std::string& db_path);
    bool loadGraph(CityGraph& graph);
    bool loadPendingPackages(PackageTree& tree);
    bool savePackage(const Package& pkg);
    bool updatePackageStatus(int package_id, PackageStatus status);
    bool logDelivery(const DeliveryRoute& route);
};
