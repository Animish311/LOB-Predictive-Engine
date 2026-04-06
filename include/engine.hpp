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

    double calculateMicroPrice(const OrderBookSnapshot& snapshot);
    double calculateDepthMicroPrice(const OrderBookSnapshot& snapshot, double alpha = 0.5);
    double calculateOFI(const OrderBookSnapshot& current);
};
