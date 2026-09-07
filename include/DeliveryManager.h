#pragma once

#include "CityGraph.h"
#include "PackageTree.h"
#include "DatabaseHelper.h"
#include "PathFinder.h"
#include <string>
#include <vector>

class DeliveryManager {
private:
    CityGraph graph;
    PackageTree tree;
    DatabaseHelper db;

public:
    DeliveryManager() = default;

    bool bootSystem(const std::string& db_path);
    void handleAddPackage(int id, int dest, int prio, int64_t deadline);
    void handleDispatch(int driver_id, int warehouse_node, int64_t deadline_threshold);
    void listPending() const;
};
