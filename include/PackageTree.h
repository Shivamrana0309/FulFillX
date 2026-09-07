#pragma once

#include "Models.h"
#include <vector>
#include <cstdint>
#include <cstddef>

class PackageTree {
private:
    struct BSTNode {
        int64_t deadline;
        std::vector<Package> packages;
        BSTNode* left;
        BSTNode* right;

        BSTNode(int64_t dl, const Package& pkg) 
            : deadline(dl), packages{pkg}, left(nullptr), right(nullptr) {}
    };

    BSTNode* root;
    size_t total_packages;

    void destroyTree(BSTNode* node);
    BSTNode* insertImpl(BSTNode* node, const Package& pkg);
    BSTNode* removeImpl(BSTNode* node, int package_id, bool& removed);
    BSTNode* removeByDeadline(BSTNode* node, int64_t deadline, bool& removed);
    BSTNode* findMin(BSTNode* node) const;
    void getPackagesDueBeforeImpl(BSTNode* node, int64_t threshold, std::vector<Package>& result) const;

public:
    PackageTree();
    ~PackageTree();

    // Disable copy semantics for safe manual memory management
    PackageTree(const PackageTree&) = delete;
    PackageTree& operator=(const PackageTree&) = delete;

    void insert(const Package& pkg);
    bool remove(int package_id);
    std::vector<Package> getPackagesDueBefore(int64_t threshold_time) const;
    void clear();
    size_t size() const;
};
