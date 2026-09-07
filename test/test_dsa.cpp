/**
 * FulfillX: Automated Unit Test Suite
 * 
 * Tests the core DSA modules (CityGraph, PathFinder, PackageTree)
 * independently of the Database layer using Google Test.
 */

#include <gtest/gtest.h>
#include "../include/CityGraph.h"
#include "../include/PathFinder.h"
#include "../include/PackageTree.h"
#include "../include/Models.h"

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 1: CityGraph — Adjacency List Tests
// ═══════════════════════════════════════════════════════════════════════════

TEST(CityGraphTest, AddNodesAndVerifyExistence) {
    CityGraph graph;
    graph.addNode({1, "Warehouse", "WAREHOUSE"});
    graph.addNode({2, "Downtown", "COMMERCIAL"});
    graph.addNode({3, "Suburb", "RESIDENTIAL"});

    EXPECT_TRUE(graph.nodeExists(1));
    EXPECT_TRUE(graph.nodeExists(2));
    EXPECT_TRUE(graph.nodeExists(3));
    EXPECT_FALSE(graph.nodeExists(999));
}

TEST(CityGraphTest, AddEdgesAndVerifyNeighbors) {
    CityGraph graph;
    graph.addNode({1, "Warehouse", "WAREHOUSE"});
    graph.addNode({2, "Downtown", "COMMERCIAL"});
    graph.addNode({3, "Suburb", "RESIDENTIAL"});

    // Bidirectional edges: Warehouse <-> Downtown (5km), Downtown <-> Suburb (3km)
    graph.addEdge(1, 2, 5.0, 1.0);
    graph.addEdge(2, 3, 3.0, 1.5);

    // Verify Warehouse neighbors
    auto neighbors_1 = graph.getNeighbors(1);
    ASSERT_EQ(neighbors_1.size(), 1);
    EXPECT_EQ(neighbors_1[0].to_node_id, 2);
    EXPECT_DOUBLE_EQ(neighbors_1[0].distance_km, 5.0);
    EXPECT_DOUBLE_EQ(neighbors_1[0].traffic_factor, 1.0);

    // Verify Downtown neighbors (should have edges to both Warehouse and Suburb)
    auto neighbors_2 = graph.getNeighbors(2);
    ASSERT_EQ(neighbors_2.size(), 2);

    // Verify Suburb neighbors
    auto neighbors_3 = graph.getNeighbors(3);
    ASSERT_EQ(neighbors_3.size(), 1);
    EXPECT_EQ(neighbors_3[0].to_node_id, 2);
}

TEST(CityGraphTest, GetNodeNameReturnsCorrectName) {
    CityGraph graph;
    graph.addNode({1, "Central Warehouse", "WAREHOUSE"});
    graph.addNode({2, "East Tech Park", "COMMERCIAL"});

    EXPECT_EQ(graph.getNodeName(1), "Central Warehouse");
    EXPECT_EQ(graph.getNodeName(2), "East Tech Park");
    EXPECT_EQ(graph.getNodeName(999), ""); // Nonexistent node returns empty string
}

TEST(CityGraphTest, NumNodesAndEdges) {
    CityGraph graph;
    graph.addNode({1, "A", "WAREHOUSE"});
    graph.addNode({2, "B", "COMMERCIAL"});
    graph.addNode({3, "C", "RESIDENTIAL"});

    graph.addEdge(1, 2, 5.0, 1.0);
    graph.addEdge(2, 3, 3.0, 1.0);

    EXPECT_EQ(graph.getNumNodes(), 3);
    EXPECT_EQ(graph.getNumEdges(), 2); // bidirectional pairs counted once
}

TEST(CityGraphTest, EmptyGraphReturnsNoNeighbors) {
    CityGraph graph;
    auto neighbors = graph.getNeighbors(1);
    EXPECT_TRUE(neighbors.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 2: PathFinder — Dijkstra's Algorithm Tests
// ═══════════════════════════════════════════════════════════════════════════

class PathFinderTest : public ::testing::Test {
protected:
    CityGraph graph;

    void SetUp() override {
        // Build the exact FulfillX seed graph:
        //
        //   (1) Warehouse ──5km──► (2) Downtown ──3.5km*1.5──► (3) North Suburb
        //        │                      │                            │
        //      8km                    4km                          6km
        //        │                      │                            │
        //        ▼                      ▼                            ▼
        //   (4) West End          (5) East Tech Park ◄──────────────┘
        //
        graph.addNode({1, "Central Warehouse", "WAREHOUSE"});
        graph.addNode({2, "Downtown", "COMMERCIAL"});
        graph.addNode({3, "North Suburb", "RESIDENTIAL"});
        graph.addNode({4, "West End", "RESIDENTIAL"});
        graph.addNode({5, "East Tech Park", "COMMERCIAL"});

        graph.addEdge(1, 2, 5.0, 1.0);   // Warehouse <-> Downtown
        graph.addEdge(2, 3, 3.5, 1.5);   // Downtown <-> North Suburb (traffic!)
        graph.addEdge(1, 4, 8.0, 1.0);   // Warehouse <-> West End
        graph.addEdge(2, 5, 4.0, 1.0);   // Downtown <-> East Tech Park
        graph.addEdge(3, 5, 6.0, 1.0);   // North Suburb <-> East Tech Park
    }
};

TEST_F(PathFinderTest, ShortestPathWarehouseToNorthSuburb) {
    // Warehouse(1) -> Downtown(2) -> North Suburb(3)
    // Effective distance: 5.0*1.0 + 3.5*1.5 = 5.0 + 5.25 = 10.25
    auto [dist, path] = PathFinder::dijkstraShortestPath(graph, 1, 3);

    EXPECT_DOUBLE_EQ(dist, 10.25);
    ASSERT_EQ(path.size(), 3);
    EXPECT_EQ(path[0], 1);
    EXPECT_EQ(path[1], 2);
    EXPECT_EQ(path[2], 3);
}

TEST_F(PathFinderTest, ShortestPathWarehouseToEastTechPark) {
    // Warehouse(1) -> Downtown(2) -> East Tech Park(5)
    // Effective distance: 5.0*1.0 + 4.0*1.0 = 9.0
    auto [dist, path] = PathFinder::dijkstraShortestPath(graph, 1, 5);

    EXPECT_DOUBLE_EQ(dist, 9.0);
    ASSERT_EQ(path.size(), 3);
    EXPECT_EQ(path[0], 1);
    EXPECT_EQ(path[1], 2);
    EXPECT_EQ(path[2], 5);
}

TEST_F(PathFinderTest, ShortestPathWarehouseToWestEnd) {
    // Direct: Warehouse(1) -> West End(4)
    // Effective distance: 8.0*1.0 = 8.0
    auto [dist, path] = PathFinder::dijkstraShortestPath(graph, 1, 4);

    EXPECT_DOUBLE_EQ(dist, 8.0);
    ASSERT_EQ(path.size(), 2);
    EXPECT_EQ(path[0], 1);
    EXPECT_EQ(path[1], 4);
}

TEST_F(PathFinderTest, ShortestPathToSelf) {
    // Routing from a node to itself should return distance 0 and a single-node path
    auto [dist, path] = PathFinder::dijkstraShortestPath(graph, 1, 1);

    EXPECT_DOUBLE_EQ(dist, 0.0);
    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path[0], 1);
}

TEST_F(PathFinderTest, NoPathToDisconnectedNode) {
    // Add an isolated node with no edges
    graph.addNode({99, "Island", "RESIDENTIAL"});

    auto [dist, path] = PathFinder::dijkstraShortestPath(graph, 1, 99);

    EXPECT_LT(dist, 0); // Negative distance signals failure
    EXPECT_TRUE(path.empty());
}

TEST_F(PathFinderTest, PathIsSymmetric) {
    // Distance Warehouse -> East Tech Park should equal East Tech Park -> Warehouse
    // (all edges are bidirectional with identical weights)
    auto [dist_forward, path_forward] = PathFinder::dijkstraShortestPath(graph, 1, 5);
    auto [dist_reverse, path_reverse] = PathFinder::dijkstraShortestPath(graph, 5, 1);

    EXPECT_DOUBLE_EQ(dist_forward, dist_reverse);
    EXPECT_EQ(path_forward.size(), path_reverse.size());
}

// ═══════════════════════════════════════════════════════════════════════════
// SECTION 3: PackageTree — Custom BST Tests
// ═══════════════════════════════════════════════════════════════════════════

// Helper to create a Package quickly
static Package makePackage(int id, int dest, int prio, int64_t deadline) {
    return Package{id, dest, prio, deadline, PackageStatus::PENDING};
}

TEST(PackageTreeTest, InsertAndSize) {
    PackageTree tree;
    EXPECT_EQ(tree.size(), 0);

    tree.insert(makePackage(1, 2, 5, 1000));
    tree.insert(makePackage(2, 3, 3, 2000));
    tree.insert(makePackage(3, 4, 1, 3000));

    EXPECT_EQ(tree.size(), 3);
}

TEST(PackageTreeTest, InsertDuplicateDeadlineGroupsPackages) {
    PackageTree tree;
    // Two packages with the exact same deadline should be grouped in one BSTNode
    tree.insert(makePackage(10, 2, 5, 5000));
    tree.insert(makePackage(11, 3, 3, 5000));

    EXPECT_EQ(tree.size(), 2);

    // Querying should return both
    auto result = tree.getPackagesDueBefore(5001);
    EXPECT_EQ(result.size(), 2);
}

TEST(PackageTreeTest, RangeQueryReturnsCorrectPackages) {
    PackageTree tree;

    // Insert 5 packages with staggered timestamps
    tree.insert(makePackage(101, 2, 5, 1000));  // Due at 1000
    tree.insert(makePackage(102, 3, 3, 2000));  // Due at 2000
    tree.insert(makePackage(103, 4, 1, 3000));  // Due at 3000
    tree.insert(makePackage(104, 5, 2, 4000));  // Due at 4000
    tree.insert(makePackage(105, 2, 4, 5000));  // Due at 5000

    // Query: all packages due before 3500
    // Should return packages 101, 102, 103 (deadlines 1000, 2000, 3000)
    auto result = tree.getPackagesDueBefore(3500);
    ASSERT_EQ(result.size(), 3);

    // Verify IDs — in-order traversal should yield ascending deadline order
    EXPECT_EQ(result[0].id, 101);
    EXPECT_EQ(result[1].id, 102);
    EXPECT_EQ(result[2].id, 103);
}

TEST(PackageTreeTest, RangeQueryWithExactBoundary) {
    PackageTree tree;
    tree.insert(makePackage(1, 2, 5, 1000));
    tree.insert(makePackage(2, 3, 3, 2000));
    tree.insert(makePackage(3, 4, 1, 3000));

    // Threshold exactly equals a deadline — implementation uses <= (inclusive)
    auto result = tree.getPackagesDueBefore(2000);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].id, 1);
    EXPECT_EQ(result[1].id, 2);
}

TEST(PackageTreeTest, RangeQueryReturnsNothingWhenAllAfterThreshold) {
    PackageTree tree;
    tree.insert(makePackage(1, 2, 5, 5000));
    tree.insert(makePackage(2, 3, 3, 6000));

    auto result = tree.getPackagesDueBefore(1000);
    EXPECT_TRUE(result.empty());
}

TEST(PackageTreeTest, RemovePackageById) {
    PackageTree tree;
    tree.insert(makePackage(1, 2, 5, 1000));
    tree.insert(makePackage(2, 3, 3, 2000));
    tree.insert(makePackage(3, 4, 1, 3000));

    EXPECT_EQ(tree.size(), 3);

    EXPECT_TRUE(tree.remove(2));  // Remove the middle package
    EXPECT_EQ(tree.size(), 2);

    // Verify it's gone from range queries
    auto result = tree.getPackagesDueBefore(5000);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].id, 1);
    EXPECT_EQ(result[1].id, 3);
}

TEST(PackageTreeTest, RemoveNonexistentPackageReturnsFalse) {
    PackageTree tree;
    tree.insert(makePackage(1, 2, 5, 1000));

    EXPECT_FALSE(tree.remove(999));
    EXPECT_EQ(tree.size(), 1); // Size unchanged
}

TEST(PackageTreeTest, ClearEmptiesTree) {
    PackageTree tree;
    tree.insert(makePackage(1, 2, 5, 1000));
    tree.insert(makePackage(2, 3, 3, 2000));

    tree.clear();
    EXPECT_EQ(tree.size(), 0);

    auto result = tree.getPackagesDueBefore(99999);
    EXPECT_TRUE(result.empty());
}
