#pragma once
#include <map>
#include "PriceLevel.hpp"
#include <unordered_map>

class OrderBook
{
public:
    // Bids: Sorted high to low (std::greater)
    std::map<double, PriceLevel, std::greater<double>> bids;

    /// Asks: Sorted low to high (std::less)
    std::map<double, PriceLevel, std::less<double>> asks;

    // Map for tracking OrderID -> memoryAddress
    std::unordered_map<uint64_t, Order *> orderLookup;

    void addOrder(Order *order)
    {
        if (order->side == Side::Buy)
        {
            bids[order->price].price = order->price;
            bids[order->price].addOrder(order);
        }
        else
        {
            asks[order->price].price = order->price;
            asks[order->price].addOrder(order);
        }
    }

    void cancelOrder(uint64_t orderId)
    {
        auto it = orderLookup.find(orderId);
        if (it == orderLookup.end())
            return;

        Order *orderToCancel = it->second;
        double price = orderToCancel->price;

        if (orderToCancel->side == Side::Buy)
        {
            bids[price].removeOrder(orderToCancel);
            if (bids[price].head == nullptr)
                bids.erase(price);
        }
        else
        {
            asks[price].removeOrder(orderToCancel);
            if (asks[price].head == nullptr)
                asks.erase(price);
        }

        orderLookup.erase(it);
    }
};