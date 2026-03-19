#pragma once
#include <map>
#include "PriceLevel.hpp"

class OrderBook
{
    public:
        // Bids: Sorted high to low (std::greater)
        std::map<double, PriceLevel, std::greater<double>> bids;

        //// Asks: Sorted low to high (std::less)
        std::map<double, PriceLevel, std::less<double>> asks;

        void addOrder(Order* order)
        {
            if ( order->side == Side::Buy)
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


};