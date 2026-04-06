#pragma once

#include <vector>
#include <cstdint>

struct Level{
    double price;
    double quantity;
};

struct OrderBookSnapshot{
    uint64_t timestamp;
    std::vector<Level> bids;
    std::vector<Level> asks;
};

class LOBEngine{
private:
    OrderBookSnapshot previous_snapshot;
    bool has_previous_state = false;

public:
    LOBEngine() = default;

    // Standard Level 1 Micro-Price
    double calculateMicroPrice(const OrderBookSnapshot& snapshot);
    
    // alpha controls the decay rate
    double calculateDepthMicroPrice(const OrderBookSnapshot& snapshot, double alpha = 0.5);
    
    // Order Flow Imbalance
    double calculateOFI(const OrderBookSnapshot& current);
};
