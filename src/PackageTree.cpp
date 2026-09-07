#include "../include/PackageTree.h"
#include <algorithm>

PackageTree::PackageTree() : root(nullptr), total_packages(0) {}

PackageTree::~PackageTree() {
    clear();
}

void PackageTree::clear() {
    destroyTree(root);
    root = nullptr;
    total_packages = 0;
}

void PackageTree::destroyTree(BSTNode* node) {
    if (node) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

size_t PackageTree::size() const {
    return total_packages;
}

void PackageTree::insert(const Package& pkg) {
    root = insertImpl(root, pkg);
    total_packages++;
}

PackageTree::BSTNode* PackageTree::insertImpl(BSTNode* node, const Package& pkg) {
    if (!node) {
        return new BSTNode(pkg.deadline_timestamp, pkg);
    }

    if (pkg.deadline_timestamp < node->deadline) {
        node->left = insertImpl(node->left, pkg);
    } else if (pkg.deadline_timestamp > node->deadline) {
        node->right = insertImpl(node->right, pkg);
    } else {
        node->packages.push_back(pkg);
    }
    return node;
}

std::vector<Package> PackageTree::getPackagesDueBefore(int64_t threshold_time) const {
    std::vector<Package> result;
    getPackagesDueBeforeImpl(root, threshold_time, result);
    return result;
}

void PackageTree::getPackagesDueBeforeImpl(BSTNode* node, int64_t threshold, std::vector<Package>& result) const {
    if (!node) return;

    // Traverse left subtree (earlier deadlines)
    getPackagesDueBeforeImpl(node->left, threshold, result);

    if (node->deadline <= threshold) {
        result.insert(result.end(), node->packages.begin(), node->packages.end());
        // Since current deadline <= threshold, there could be valid elements in right subtree
        getPackagesDueBeforeImpl(node->right, threshold, result);
    }
    // Optimization: Prune! If current deadline > threshold, we skip the right subtree entirely.
}

bool PackageTree::remove(int package_id) {
    bool removed = false;
    root = removeImpl(root, package_id, removed);
    if (removed) {
        total_packages--;
    }
    return removed;
}

PackageTree::BSTNode* PackageTree::findMin(BSTNode* node) const {
    while (node && node->left != nullptr) {
        node = node->left;
    }
    return node;
}

PackageTree::BSTNode* PackageTree::removeImpl(BSTNode* node, int package_id, bool& removed) {
    if (!node) return nullptr;

    // Try to find in the left subtree
    if (!removed) {
        node->left = removeImpl(node->left, package_id, removed);
    }

    // Check current node
    if (!removed) {
        auto it = std::find_if(node->packages.begin(), node->packages.end(),
                               [package_id](const Package& p) { return p.id == package_id; });
        if (it != node->packages.end()) {
            node->packages.erase(it);
            removed = true;

            // If node has no packages left, delete the BST node
            if (node->packages.empty()) {
                if (!node->left) {
                    BSTNode* right_child = node->right;
                    delete node;
                    return right_child;
                } else if (!node->right) {
                    BSTNode* left_child = node->left;
                    delete node;
                    return left_child;
                } else {
                    BSTNode* min_right = findMin(node->right);
                    node->deadline = min_right->deadline;
                    node->packages = min_right->packages;
                    
                    bool dummy = false;
                    node->right = removeByDeadline(node->right, min_right->deadline, dummy);
                }
            }
        }
    }

    // Try to find in the right subtree
    if (!removed) {
        node->right = removeImpl(node->right, package_id, removed);
    }

    return node;
}

PackageTree::BSTNode* PackageTree::removeByDeadline(BSTNode* node, int64_t deadline, bool& removed) {
    if (!node) return nullptr;

    if (deadline < node->deadline) {
        node->left = removeByDeadline(node->left, deadline, removed);
    } else if (deadline > node->deadline) {
        node->right = removeByDeadline(node->right, deadline, removed);
    } else {
        removed = true;
        if (!node->left) {
            BSTNode* right_child = node->right;
            delete node;
            return right_child;
        } else if (!node->right) {
            BSTNode* left_child = node->left;
            delete node;
            return left_child;
        } else {
            BSTNode* min_right = findMin(node->right);
            node->deadline = min_right->deadline;
            node->packages = min_right->packages;
            node->right = removeByDeadline(node->right, min_right->deadline, removed);
        }
    }
    return node;
}
