# FulfillX: REST API Migration Guide

This guide details exactly how to transform your FulfillX C++ CLI application into a high-performance, native REST API capable of serving a React frontend directly without relying on an intermediate Node.js server.

We will use two modern, lightweight, header-only C++ libraries:
1. **`cpp-httplib`**: A multi-threaded HTTP server.
2. **`nlohmann/json`**: The gold standard for JSON serialization in modern C++.

---

## 1. CMake Configuration (`CMakeLists.txt`)

You don't need to manually download these libraries. We will use CMake's `FetchContent` to pull them down automatically at build time. 

Add the following to your `CMakeLists.txt` right before your `add_executable(fulfillx ...)` line:

```cmake
include(FetchContent)

# Fetch nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Fetch cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)
```

Then, modify your `target_link_libraries` for the `fulfillx` executable to link these new dependencies along with thread support:

```cmake
target_link_libraries(fulfillx PRIVATE 
    fulfillx_core 
    nlohmann_json::nlohmann_json 
    httplib::httplib 
    Threads::Threads
)
```

---

## 2. C++ JSON Serialization (`include/Models.h`)

`nlohmann/json` uses powerful macros to automatically generate serialization code for your structs. Include the header at the top of `Models.h` and add the macros at the bottom of the file.

```cpp
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp> // <-- Add this

using json = nlohmann::json; // Alias for convenience

enum class PackageStatus { PENDING, DISPATCHED, DELIVERED, CANCELLED };

// ... (Your existing structs remain exactly the same) ...

// =======================================================================
// JSON Serialization Definitions
// =======================================================================

// 1. Serialize the Enum to Strings (not integers)
NLOHMANN_JSON_SERIALIZE_ENUM(PackageStatus, {
    {PackageStatus::PENDING, "PENDING"},
    {PackageStatus::DISPATCHED, "DISPATCHED"},
    {PackageStatus::DELIVERED, "DELIVERED"},
    {PackageStatus::CANCELLED, "CANCELLED"}
})

// 2. Auto-generate struct <-> JSON converters
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Node, id, name, node_type)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Package, id, dest_node_id, priority_level, deadline_timestamp, status)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RouteLeg, from_node, to_node, leg_distance)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeliveryRoute, driver_id, path_nodes, delivered_package_ids, total_distance_km)
```

---

## 3. Native C++ HTTP Server (`src/main.cpp`)

To act as an API, your application can no longer block on `std::cin` waiting for user input. We must replace the REPL with a thread-blocking `httplib::Server` instance. 

*Note: You will need to slightly refactor `DeliveryManager.cpp` so that `handleDispatch` returns a `DeliveryRoute` object instead of printing it to `std::cout`.*

Replace the entire contents of `src/main.cpp` with this:

```cpp
#include "../include/DeliveryManager.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

// Utility to attach CORS headers to every response
void set_cors_headers(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*"); // Allow all origins (e.g., React on port 3000)
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main() {
    DeliveryManager manager;
    
    std::cout << "[System] Booting FulfillX Engine...\n";
    if (!manager.bootSystem("fulfillx.db")) {
        std::cerr << "[System] CRITICAL ERROR: Failed to boot system.\n";
        return 1;
    }

    httplib::Server svr;

    // Global Preflight Handler for CORS
    svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        set_cors_headers(res);
        res.status = 204; // No Content
    });

    // ── Endpoint 1: GET /api/pending ──────────────────────────────────────
    svr.Get("/api/pending", [&manager](const httplib::Request&, httplib::Response& res) {
        set_cors_headers(res);
        
        // Note: Refactor DeliveryManager::listPending() to return std::vector<Package>
        // std::vector<Package> pkgs = manager.getPendingPackages();
        // res.set_content(json(pkgs).dump(), "application/json");
        
        // Placeholder response:
        json response = { {"status", "success"}, {"pending_count", manager.getPendingCount()} };
        res.set_content(response.dump(), "application/json");
    });

    // ── Endpoint 2: POST /api/dispatch ────────────────────────────────────
    svr.Post("/api/dispatch", [&manager](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(res);
        
        try {
            // Parse incoming JSON body
            auto body = json::parse(req.body);
            int driver_id = body.at("driver_id").get<int>();
            int warehouse_node = body.at("warehouse_node").get<int>();
            int64_t max_deadline = body.at("max_deadline").get<int64_t>();

            // Note: Refactor DeliveryManager::handleDispatch to return the DeliveryRoute
            // DeliveryRoute route = manager.handleDispatch(driver_id, warehouse_node, max_deadline);
            // res.set_content(json(route).dump(), "application/json");

            // Placeholder response:
            json response = { {"status", "dispatched"}, {"driver_id", driver_id} };
            res.set_content(response.dump(), "application/json");
            
        } catch (const std::exception& e) {
            res.status = 400; // Bad Request
            json error = { {"error", "Invalid JSON payload or missing required fields."} };
            res.set_content(error.dump(), "application/json");
        }
    });

    std::cout << "[System] REST API running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080); // Blocks indefinitely

    return 0;
}
```

---

## 4. React Frontend Integration

Because we correctly added CORS headers to the C++ server, your React application can communicate with it exactly as if it were a Node.js/Express server.

Here is a minimal React component demonstrating a raw connection using the native `fetch` API:

```jsx
import React, { useState } from 'react';

export default function DispatchConsole() {
  const [routeData, setRouteData] = useState(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);

  const triggerDispatch = async () => {
    setLoading(true);
    setError(null);
    
    try {
      // Send POST request directly to the C++ binary!
      const response = await fetch('http://localhost:8080/api/dispatch', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          driver_id: 99,
          warehouse_node: 1,
          max_deadline: 1700005000
        }),
      });

      if (!response.ok) {
        throw new Error(`API Error: ${response.status}`);
      }

      const data = await response.json();
      setRouteData(data); // data is parsed automatically from C++ DeliveryRoute struct
      
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div style={{ padding: '2rem', fontFamily: 'system-ui' }}>
      <h2>FulfillX Dispatch Console</h2>
      
      <button 
        onClick={triggerDispatch} 
        disabled={loading}
        style={{ padding: '10px 20px', background: '#0070f3', color: 'white', border: 'none', borderRadius: '5px' }}
      >
        {loading ? 'Routing...' : 'Dispatch Driver'}
      </button>

      {error && <p style={{ color: 'red' }}>{error}</p>}

      {routeData && (
        <div style={{ marginTop: '2rem', background: '#f5f5f5', padding: '1rem', borderRadius: '5px' }}>
          <h3>Computed Route</h3>
          <p><strong>Driver ID:</strong> {routeData.driver_id}</p>
          <p><strong>Total Distance:</strong> {routeData.total_distance_km} km</p>
          
          <h4>Path Sequence:</h4>
          <ul>
            {routeData.path_nodes.map((nodeId, idx) => (
              <li key={idx}>Node {nodeId}</li>
            ))}
          </ul>
        </div>
      )}
    </div>
  );
}
```

### Key Takeaway
This architecture proves that you do not *need* a high-level language like Python or Node.js to build a web backend. By embedding an HTTP server into your binary, your React frontend communicates directly with your raw C++ memory structures, unlocking massive performance and latency advantages.
