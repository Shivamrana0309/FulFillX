[🏠 Main README](README.md)

# FulfillX: Project Architecture & Deep Dive

> *The ultimate master guide — from pizza delivery analogies to Dijkstra's Min-Heap.*

---

## Section 1: FulfillX — The Non-Technical Explanation

Imagine you run the busiest delivery shop in your city. Every minute, new packages arrive with sticky notes on them that say things like *"Deliver to Oak Street by 4:00 PM — URGENT"*. You have a fleet of drivers, a giant paper map of the city on your wall, and a filing cabinet full of delivery records. Your job? Get every package to the right door, by the right time, using the shortest route through traffic.

**That is exactly what FulfillX does — except it does it with code instead of humans.**

Let's break down each piece of the system using this analogy:

### 🗄️ The Database — *The Manager's Filing Cabinet*
In your shop, you have a steel filing cabinet. Every package that comes in gets a paper record: where it's going, how urgent it is, and whether it's been delivered yet. If the power goes out and your computer crashes, the filing cabinet survives. When you reopen the shop the next morning, you pull the records out and pick up exactly where you left off.

In FulfillX, **SQLite** is that filing cabinet. It's a real database file sitting on disk (`fulfillx.db`). Even if the program crashes or you shut your computer down, every package, every road, and every delivery log is safely stored and ready to be loaded back into memory.

### 🗺️ The City Graph — *The Map on the Wall*
On the wall of your delivery shop, there's a big city map. Red pins mark every important location — the warehouse, the suburbs, the tech park. Colored strings connect the pins, representing roads. Some strings are short (a quick drive), some are long (a highway across town). Some strings have a little tag that says *"Rush hour: 2× slower"*.

In FulfillX, the **CityGraph** is that map. Every pin is a **Node** (a location), and every string is an **Edge** (a road with a distance and a traffic multiplier). The system loads this entire map into memory so the routing algorithm can instantly look up any connection.

### 📦 The Package Tree — *The Magical Sorting Conveyor Belt*
Now imagine a magical conveyor belt in your shop. Every time a package arrives, you place it on the belt, and it automatically slides into the correct position — sorted by its delivery deadline. The most urgent packages are always at one end. When a driver is ready to leave, you don't have to dig through a pile of hundreds of packages. You simply walk to one end of the belt and grab everything due in the next two hours. Instantly.

In FulfillX, the **PackageTree** is that conveyor belt. It's a custom-built Binary Search Tree that sorts packages by their Unix timestamp deadline. When a driver dispatches, the system performs a *range query* — it walks the tree and efficiently collects only the packages that are due before the driver's cutoff time, skipping everything else.

### 🧭 The PathFinder — *The Veteran Driver*
Finally, there's the veteran driver — the one who's been driving these streets for 30 years. You hand him a list of addresses, and he instantly maps out the absolute fastest route through the city, dodging traffic and taking shortcuts. He doesn't guess. He *calculates*.

In FulfillX, the **PathFinder** is that driver. It implements **Dijkstra's Algorithm** — one of the most famous algorithms in computer science — to mathematically compute the shortest path between any two points on the city graph, factoring in real-time traffic weights.

---

## Section 2: Folder Structure Breakdown

```
FulfillX/
├── include/          ← The Blueprints (Header files)
│   ├── Models.h
│   ├── CityGraph.h
│   ├── PathFinder.h
│   ├── PackageTree.h
│   ├── DatabaseHelper.h
│   └── DeliveryManager.h
├── src/              ← The Workers (Source files)
│   ├── CityGraph.cpp
│   ├── PathFinder.cpp
│   ├── PackageTree.cpp
│   ├── DatabaseHelper.cpp
│   ├── DeliveryManager.cpp
│   └── main.cpp
├── data/             ← The Storage Room
│   └── seed.sql
├── test/             ← The QA Department
├── build/            ← The Factory Floor
│   └── fulfillx      (compiled executable)
├── CMakeLists.txt    ← Build configuration
├── Makefile          ← Build automation
└── README.md
```

### `include/` — The Blueprints
This folder holds all `.h` (header) files. Think of these as **architectural blueprints**. They declare *what* each module can do — the function names, the data types, the class interfaces — without revealing *how* the work is actually done. This separation is a cornerstone of professional C++ engineering because it allows any module to reference another module's capabilities without needing to see its internal implementation.

### `src/` — The Workers
This folder holds all `.cpp` (source) files. These are the **actual workers** that contain the step-by-step logic. Every function that was *promised* in a header file is *fulfilled* here. When you compile the project, these are the files that get translated into machine code.

### `data/` — The Storage Room
This folder contains the `seed.sql` file — the initial dataset that populates the database with locations, roads, and the city map. It also serves as the home for `fulfillx.db` when the application runs from the `build/` directory.

### `test/` — The QA Department
Reserved for automated unit tests. In a production environment, this folder would contain test cases that verify every module independently — ensuring that changes to the PathFinder don't accidentally break the PackageTree, for example.

### `build/` — The Factory Floor
This is where raw C++ source code is transformed into a runnable binary. CMake generates build instructions here, `make` compiles the object files, and the final `fulfillx` executable lives here. This directory is intentionally separate from the source code so that build artifacts never pollute the clean codebase.

---

## Section 3: The C++ Build Process

How does a folder full of text files become a running program? In C++, this is a multi-stage process, and understanding it is essential.

### Stage 1: Header Files (`.h`) — The Promises

A header file is a **contract**. It tells the rest of the codebase: *"A function called `dijkstraShortestPath` exists. It takes a graph and two node IDs. It returns a distance and a path."*

It does **not** say *how* the shortest path is computed. It only promises that the function exists and defines its signature. This is what allows `DeliveryManager.cpp` to call `PathFinder::dijkstraShortestPath()` without needing to see or include the actual Dijkstra implementation.

```
┌─────────────────────────────┐
│      PathFinder.h           │
│  "I promise a function      │
│   called dijkstraShortestPath│
│   exists and returns a      │
│   pair<double, vector<int>>"│
└─────────────────────────────┘
```

### Stage 2: Source Files (`.cpp`) — The Implementations

The `.cpp` file is where the promise is fulfilled. `PathFinder.cpp` contains the actual 60+ lines of Dijkstra's algorithm — the priority queue, the distance tracking, the parent reconstruction. This is the real work.

### Stage 3: Compilation (`.cpp` → `.o`) — Individual Translation

The compiler (`g++` or `clang++`) takes each `.cpp` file **independently** and translates it into an **Object file** (`.o`). An object file is machine code — raw binary instructions that the CPU can understand — but *only* for that single source file.

```
CityGraph.cpp       ──►  CityGraph.o
PathFinder.cpp      ──►  PathFinder.o
PackageTree.cpp     ──►  PackageTree.o
DatabaseHelper.cpp  ──►  DatabaseHelper.o
DeliveryManager.cpp ──►  DeliveryManager.o
main.cpp            ──►  main.o
```

At this stage, each `.o` file has **unresolved references**. For example, `DeliveryManager.o` contains a call to `PathFinder::dijkstraShortestPath`, but it doesn't know *where* that function's machine code actually lives yet. It just has a placeholder that says: *"Someone will fill this in later."*

### Stage 4: Linking (`.o` files → `fulfillx`) — Stitching It Together

The **Linker** is the final stage. It takes every `.o` file, resolves all those placeholders, connects them to each other, and also connects them to external libraries like `libsqlite3` and `libpthread`. The output is a single, fully self-contained executable binary: `fulfillx`.

```
┌──────────────┐
│ CityGraph.o  │──┐
│ PathFinder.o │──┤
│ PackageTree.o│──┤
│ DBHelper.o   │──┼──► LINKER ──► fulfillx (executable)
│ Manager.o    │──┤       ▲
│ main.o       │──┘       │
│              │    ┌─────┴──────┐
│              │    │ libsqlite3 │
│              │    │ libpthread │
│              │    └────────────┘
└──────────────┘
```

### Why Not Compile Everything At Once?

**Incremental compilation.** If you change a single line in `PackageTree.cpp`, only `PackageTree.o` needs to be recompiled. The linker then re-stitches just that one updated piece with the unchanged `.o` files. On a large codebase with hundreds of files, this saves *minutes* of build time on every change.

---

## Section 4: Architecture & Module Connections

The following diagram illustrates the chain of command — how data flows through the system from user input to database persistence.

```
┌─────────────────────────────────────────────────────────────┐
│                        USER (Terminal)                       │
│              Types: BOOT, ADD_PKG, DISPATCH, etc.            │
└──────────────────────────┬──────────────────────────────────┘
                           │ stdin
                           ▼
┌──────────────────────────────────────────────────────────────┐
│                     main.cpp (Front Desk)                     │
│  • Parses command strings                                     │
│  • Validates argument counts                                  │
│  • Routes commands to the DeliveryManager                     │
└──────────────────────────┬───────────────────────────────────┘
                           │ function calls
                           ▼
┌──────────────────────────────────────────────────────────────┐
│               DeliveryManager (The Orchestrator)              │
│                                                               │
│  Owns and coordinates three subsystems:                       │
│                                                               │
│  ┌────────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │  CityGraph     │  │ PackageTree  │  │ DatabaseHelper   │  │
│  │  (In-Memory    │  │ (In-Memory   │  │ (Disk-Backed     │  │
│  │   Map)         │  │  BST Index)  │  │  Persistence)    │  │
│  └───────┬────────┘  └──────┬───────┘  └────────┬─────────┘  │
│          │                  │                    │             │
│          │    ┌─────────────┘                    │             │
│          │    │                                  │             │
│          ▼    ▼                                  ▼             │
│  ┌────────────────┐                    ┌──────────────────┐   │
│  │   PathFinder   │                    │   fulfillx.db    │   │
│  │  (Dijkstra's   │                    │   (SQLite File)  │   │
│  │   Algorithm)   │                    │                  │   │
│  └────────────────┘                    └──────────────────┘   │
└──────────────────────────────────────────────────────────────┘
```

### The Flow of a `DISPATCH` Command

1. **main.cpp** parses `DISPATCH 99 1 1700005000` and calls `manager.handleDispatch(99, 1, 1700005000)`.
2. **DeliveryManager** asks the **PackageTree**: *"Give me every package due before timestamp 1700005000."*
3. The **PackageTree** performs an optimized in-order traversal, pruning branches, and returns 2 packages.
4. **DeliveryManager** sorts them by deadline and hands the **CityGraph** + destination list to the **PathFinder**.
5. **PathFinder** runs **Dijkstra's Algorithm** sequentially for each destination, computing the shortest weighted path.
6. **DeliveryManager** collects the full route, updates each package's status to `DISPATCHED` via the **DatabaseHelper**, and logs the completed delivery route to the `Delivery_Logs` table.
7. The formatted result is printed back to the user in the terminal.

---

## Section 5: The Role of DSA (Deep Dive)

This section explains *why* specific data structures and algorithms were chosen — not merely *what* they are — and demonstrates how each one directly solves a concrete problem in the delivery domain.

---

### 5.1 Adjacency List (Graph Representation)

**The Problem:** We need to represent a city map — locations connected by roads — in memory so that our routing algorithm can traverse it efficiently.

**Two Options Exist:**

| Approach | Storage Cost | Edge Lookup |
|---|---|---|
| **Adjacency Matrix** (2D array) | O(V²) | O(1) |
| **Adjacency List** (map of vectors) | O(V + E) | O(degree) |

**Why We Chose the Adjacency List:**

Real-world city maps are **sparse graphs**. A city with 10,000 intersections does not have 100,000,000 roads (which is what V² would imply). Each intersection connects to maybe 3–6 neighboring roads. An Adjacency Matrix would allocate a 10,000 × 10,000 grid — **100 million entries** — with 99.9% of them being empty zeros. That is a catastrophic waste of RAM.

The Adjacency List stores *only* the roads that actually exist. For our 5-node city with 10 directional edges, it stores exactly 10 entries — not 25.

**FulfillX Implementation:**
```cpp
std::unordered_map<int, std::vector<Edge>> adjacency_list;
//    Node ID ──────────► List of outgoing roads
```

Each `Edge` stores:
- `to_node_id`: Where the road leads.
- `distance_km`: Physical length of the road.
- `traffic_factor`: A real-time multiplier (1.0 = clear, 2.0 = gridlock).

The `unordered_map` provides **O(1) average-case** lookup to retrieve all neighbors of any given node — exactly what Dijkstra's algorithm needs at every step.

---

### 5.2 Binary Search Tree (Package Indexing)

**The Problem:** When a driver is ready to leave the warehouse at 2:00 PM, we need to instantly retrieve all packages with deadlines before 4:00 PM. This is a **range query**: *"Give me everything less than threshold X."*

**Why Not Use a Simple Array or Vector?**

If packages are stored in an unsorted `std::vector`, answering the range query requires scanning **every single package** — an O(N) operation. With 100,000 pending packages, this means 100,000 comparisons every time a driver dispatches. Under heavy load, this becomes a bottleneck.

**Why a BST Solves This:**

A Binary Search Tree stores packages sorted by their `deadline_timestamp`. The tree's structure inherently partitions the data:

```
                    [Deadline: 1700000050]
                   /                      \
       [Deadline: 1700000000]       [Deadline: 1700000100]
              (Pkg 101)                    (Pkg 103)
```

To find all packages due before `1700000060`:
1. Visit the root (`1700000050`). It qualifies → collect it.
2. The **entire left subtree** has smaller deadlines → collect everything there.
3. The right subtree has `1700000100` which is *past* our threshold → **prune the entire branch**. Don't even look at it.

This **branch pruning** is the key insight. Instead of checking N packages, we skip entire subtrees that cannot possibly contain qualifying results. The effective time complexity drops to **O(log N + K)**, where K is the number of matching results.

**Manual Memory Management:**
The `PackageTree` uses raw `BSTNode*` pointers with a recursive destructor — no `std::map`, no `std::set`. This demonstrates explicit ownership semantics: the tree *owns* its nodes, allocates them with `new`, and deallocates them with `delete` in a post-order traversal during destruction. This is precisely the kind of low-level memory discipline expected in systems-level C++.

---

### 5.3 Dijkstra's Algorithm & the Min-Heap

**The Problem:** Given a starting warehouse and a destination node, find the path with the smallest total *effective distance* (physical distance × traffic factor) through a weighted graph.

**Why Dijkstra's Algorithm?**

Dijkstra's algorithm is the gold standard for single-source shortest path problems on graphs with **non-negative edge weights**. It guarantees mathematical optimality — the path it returns is provably the shortest possible.

**The Role of the Min-Heap:**

At every step, Dijkstra's algorithm must answer the question: *"Of all the nodes I haven't finalized yet, which one currently has the smallest tentative distance?"*

A naïve approach would scan all unvisited nodes to find the minimum — an O(V) operation at every step, yielding O(V²) overall.

The **Min-Heap** (`std::priority_queue` with `std::greater`) solves this decisively:

| Operation | Without Heap | With Min-Heap |
|---|---|---|
| Extract minimum node | O(V) | **O(log V)** |
| Update tentative distance | O(1) | **O(log V)** |
| **Overall complexity** | **O(V²)** | **O((V + E) log V)** |

For a city with 10,000 nodes and 30,000 roads:
- Without heap: ~100,000,000 operations.
- With heap: ~**430,000** operations.

That is a **230× speedup** — the difference between a route computation taking seconds versus milliseconds.

**FulfillX Implementation:**
```cpp
// Min-Heap: smallest effective_distance always on top
std::priority_queue<
    std::pair<double, int>,           // (effective_distance, node_id)
    std::vector<std::pair<double, int>>,
    std::greater<>                     // Min-Heap comparator
> pq;
```

At every iteration:
1. **Pop** the node with the smallest known distance in O(log V).
2. **Relax** all its neighboring edges: if `current_dist + (edge.distance_km × edge.traffic_factor)` is shorter than the neighbor's current best, update it and push the new distance onto the heap.
3. **Reconstruct** the full path using a `parent` map that tracks how we reached each node.

The algorithm terminates the instant the destination node is popped from the heap — guaranteeing that no shorter path exists.

---

*Built with C++17. No external graph libraries. No training wheels.*
