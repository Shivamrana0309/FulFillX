#include "../include/DeliveryManager.h"
#include <iostream>
#include <string>
#include <sstream>

void printHelp() {
    std::cout << "\nAvailable Commands:\n"
              << "  BOOT <db_path>\n"
              << "  ADD_PKG <id> <dest_node> <prio> <deadline>\n"
              << "  DISPATCH <driver_id> <warehouse_node> <max_deadline>\n"
              << "  LIST_PENDING\n"
              << "  EXIT\n";
}

int main() {
    DeliveryManager manager;
    std::string line;

    std::cout << "=== FulfillX: Delivery Routing Engine ===\n";
    printHelp();

    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, line)) {
            break; // Handle EOF securely
        }

        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "EXIT") {
            break;
        } else if (command == "BOOT") {
            std::string db_path;
            if (iss >> db_path) {
                manager.bootSystem(db_path);
            } else {
                std::cout << "Usage: BOOT <db_path>\n";
            }
        } else if (command == "ADD_PKG") {
            int id, dest, prio;
            int64_t deadline;
            // Robust parsing check to ensure all parameters are valid numbers
            if (iss >> id >> dest >> prio >> deadline) {
                manager.handleAddPackage(id, dest, prio, deadline);
            } else {
                std::cout << "Usage: ADD_PKG <id> <dest_node> <prio> <deadline>\n";
            }
        } else if (command == "DISPATCH") {
            int driver_id, warehouse_node;
            int64_t max_deadline;
            // Robust parsing check for integer arguments
            if (iss >> driver_id >> warehouse_node >> max_deadline) {
                manager.handleDispatch(driver_id, warehouse_node, max_deadline);
            } else {
                std::cout << "Usage: DISPATCH <driver_id> <warehouse_node> <max_deadline>\n";
            }
        } else if (command == "LIST_PENDING") {
            manager.listPending();
        } else if (command == "HELP") {
            printHelp();
        } else {
            std::cout << "Unknown command: " << command << ". Type HELP for a list of commands.\n";
        }
    }

    std::cout << "Exiting FulfillX Engine...\n";
    return 0;
}
