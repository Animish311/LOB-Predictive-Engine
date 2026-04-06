#include "engine.hpp"
#include <cmath>

double LOBEngine::calculateMicroPrice(const OrderBookSnapshot& snapshot){
    if (snapshot.bids.empty() || snapshot.asks.empty()) return 0.0; 

    double best_bid_price = snapshot.bids[0].price;
    double best_bid_vol   = snapshot.bids[0].quantity;
    double best_ask_price = snapshot.asks[0].price;
    double best_ask_vol   = snapshot.asks[0].quantity;

    double total_vol = best_bid_vol + best_ask_vol;
    if (total_vol == 0.0) return (best_bid_price + best_ask_price) / 2.0;

    return ((best_bid_vol * best_ask_price) + (best_ask_vol * best_bid_price)) / total_vol;
}

//Calculates price based on depth using exponential decay
double LOBEngine::calculateDepthMicroPrice(const OrderBookSnapshot& snapshot, double alpha){
    if (snapshot.bids.empty() || snapshot.asks.empty()) return 0.0;

    double weighted_bid_sum = 0.0;
    double weighted_ask_sum = 0.0;
    double total_bid_weight = 0.0;
    double total_ask_weight = 0.0;

    //Calculates depth for bids
    for (size_t i = 0; i < snapshot.bids.size(); ++i) {
        double weight = std::exp(-alpha * i);
        weighted_bid_sum += snapshot.bids[i].quantity * weight;
        total_bid_weight += weight;
    }

    //Calculates depth for asks
    for (size_t i = 0; i < snapshot.asks.size(); ++i) {
        double weight = std::exp(-alpha * i);
        weighted_ask_sum += snapshot.asks[i].quantity * weight;
        total_ask_weight += weight;
    }

    double V_b_eff = weighted_bid_sum;
    double V_a_eff = weighted_ask_sum;

    double P_b = snapshot.bids[0].price;
    double P_a = snapshot.asks[0].price;

    double total_eff_vol = V_b_eff + V_a_eff;
    if (total_eff_vol == 0.0) return (P_b + P_a) / 2.0;

    //Pulling the price to the heavier side
    return ((V_b_eff * P_a) + (V_a_eff * P_b)) / total_eff_vol;
}

double LOBEngine::calculateOFI(const OrderBookSnapshot& current) {
    if (!has_previous_state || current.bids.empty() || current.asks.empty()) {
        previous_snapshot = current;
        has_previous_state = true;
        return 0.0; 
    }

    double current_bid_price = current.bids[0].price;
    double current_bid_vol   = current.bids[0].quantity;
    double prev_bid_price    = previous_snapshot.bids[0].price;
    double prev_bid_vol      = previous_snapshot.bids[0].quantity;

    double current_ask_price = current.asks[0].price;
    double current_ask_vol   = current.asks[0].quantity;
    double prev_ask_price    = previous_snapshot.asks[0].price;
    double prev_ask_vol      = previous_snapshot.asks[0].quantity;

    double bid_imbalance = 0.0;
    double ask_imbalance = 0.0;

    if (current_bid_price > prev_bid_price) bid_imbalance = current_bid_vol;
    else if (current_bid_price == prev_bid_price) bid_imbalance = current_bid_vol - prev_bid_vol;
    else bid_imbalance = -prev_bid_vol;

    if (current_ask_price < prev_ask_price) ask_imbalance = current_ask_vol;
    else if (current_ask_price == prev_ask_price) ask_imbalance = current_ask_vol - prev_ask_vol;
    else ask_imbalance = -prev_ask_vol;

    previous_snapshot = current;
    return bid_imbalance - ask_imbalance;
}
