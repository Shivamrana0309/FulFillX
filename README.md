[📖 Architecture Deep Dive](ARCHITECTURE.md)

# FulfillX: Delivery Routing Engine

## 1. Project Overview
FulfillX is a high-performance, last-mile delivery routing engine built from the ground up in modern C++17. Designed to simulate the backend logistics network of large-scale e-commerce platforms like Amazon, FulfillX calculates optimal delivery routes using graph theory, dynamically prioritizes packages with custom-built search trees, and ensures robust state persistence via SQLite3. 

This project was built without relying on heavy external frameworks or standard graph libraries to explicitly demonstrate raw proficiency in deep C++ concepts, including manual memory management, pointer arithmetic, advanced STL container usage, Object-Oriented Programming, and pure algorithm design (DSA). 

## 2. Core Codebase & Architecture
The system is deeply modularized, strictly separating data layers, algorithms, and orchestration.

- `include/Models.h`: Defines the core data structures and enumerations that flow through the system (`Package`, `Node`, `Edge`, `DeliveryRoute`, and `PackageStatus`).
- `CityGraph`: Represents the physical map. Uses an Adjacency List (`std::unordered_map<int, std::vector<Edge>>`) to map intersections (Nodes) and roads (Edges) with real-time traffic weight multipliers.
- `PathFinder`: The algorithmic brain of the engine. Implements Dijkstra's Shortest Path algorithm utilizing a Min-Heap (`std::priority_queue` with `std::greater`) to calculate the mathematically fastest route between delivery points based on the *effective distance* (physical distance × traffic factor).
- `PackageTree`: A custom-built Binary Search Tree (BST) featuring manual memory management via recursive destructors. It indexes packages strictly by their Unix timestamp delivery deadline, enabling fast $O(\log N)$ range queries to instantly fetch the most urgent packages.
- `DatabaseHelper`: An RAII-compliant SQLite3 wrapper that utilizes strictly bound prepared statements, ensuring that all database operations are thread-safe and entirely immune to SQL injection.
- `DeliveryManager`: The orchestration layer that acts as the primary controller, managing the interactions and synchronization between the CLI interface, the SQLite Database, the City Graph, and the Package BST.
- `main.cpp`: A robust, interactive CLI REPL (Read-Eval-Print Loop) providing the user interface for executing system commands in real-time.

## 3. What `fulfillx.db` Stores (Database Schema)
The SQLite database (`fulfillx.db`) acts as the permanent state recovery mechanism and immutable auditing log for the routing engine. It consists of 4 core tables:

- **`Locations`**: Stores vertex data, mapping a physical intersection to a functional type (Warehouse, Residential, Commercial nodes).
- **`Roads`**: Stores edge data representing roads, including Source ID, Destination ID, physical distance in kilometers, and traffic multipliers.
- **`Packages`**: Stores order data, tracking the destination node, priority level (1-5), Unix timestamp deadline, and the real-time package state (`PENDING`, `DISPATCHED`, `DELIVERED`, `CANCELLED`).
- **`Delivery_Logs`**: An immutable audit trail storing the exact historical record of a dispatch. It records the driver ID, the exact stringified route path taken, total distance traveled, and the dispatch timestamp.

## 4. Step-by-Step Build and Run Guide
Follow these steps to compile and run the engine on an Ubuntu or macOS environment.

**Prerequisites:** Ensure you have `cmake`, `make`, a modern C++ compiler (`gcc` or `clang`), and the SQLite3 development libraries (`libsqlite3-dev` or `sqlite3`) installed.

**1. Compilation**
```bash
mkdir build
cd build
cmake ..
make
```

**2. Database Initialization (Seeding the Graph)**
Before running the system, initialize the map by feeding the seed data into the database:
```bash
sqlite3 fulfillx.db < ../data/seed.sql
```

**3. Running the Engine**
Launch the CLI REPL and boot the system:
```bash
./fulfillx
```

Inside the interactive prompt, run your test sequence:
```text
> BOOT fulfillx.db
> ADD_PKG 101 3 5 1700000000
> ADD_PKG 102 5 2 1700000050
> LIST_PENDING
> DISPATCH 99 1 1700005000
> EXIT
```

## 5. Running the Automated Test Suite
To execute the Google Test suite and verify all core Data Structures and Algorithms independently of the database:
```bash
cd build
./fulfillx_test

# Or run with CTest for structured output:
ctest --output-on-failure
```
