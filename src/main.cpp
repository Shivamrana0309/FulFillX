#include "../include/DeliveryManager.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

// Utility to attach CORS headers to every response
void set_cors_headers(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
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
        res.status = 204;
    });

    // Endpoint: GET /api/pending
    svr.Get("/api/pending", [&manager](const httplib::Request&, httplib::Response& res) {
        set_cors_headers(res);
        json response = { 
            {"status", "success"}, 
            {"pending_count", manager.getPendingCount()} 
        };
        res.set_content(response.dump(), "application/json");
    });

    // Endpoint: POST /api/dispatch
    svr.Post("/api/dispatch", [&manager](const httplib::Request& req, httplib::Response& res) {
        set_cors_headers(res);
        
        try {
            auto body = json::parse(req.body);
            int driver_id = body.at("driver_id").get<int>();
            int warehouse_node = body.at("warehouse_node").get<int>();
            int64_t max_deadline = body.at("max_deadline").get<int64_t>();

            std::optional<DeliveryRoute> route = manager.handleDispatch(driver_id, warehouse_node, max_deadline);
            
            if (route) {
                json response = { 
                    {"status", "dispatched"}, 
                    {"route", *route} 
                };
                res.set_content(response.dump(), "application/json");
            } else {
                json response = { 
                    {"status", "failed"}, 
                    {"message", "No packages dispatched"} 
                };
                res.status = 404;
                res.set_content(response.dump(), "application/json");
            }
            
        } catch (const std::exception& e) {
            res.status = 400; // Bad Request
            json error = { {"error", "Invalid JSON payload or missing required fields."} };
            res.set_content(error.dump(), "application/json");
        }
    });

    std::cout << "[System] REST API running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}
