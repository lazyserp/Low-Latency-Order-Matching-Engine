// A price level = all orders that have the SAME price
#pragma once
#include "common/Types.hpp"

struct PriceLevel
{
    // the Orders are stored as double linked list: 1.to maintain the insertion order , 2. to delete a specific order
    double price;
    Order* head = nullptr;
    Order* tail = nullptr;

    void addOrder(Order* order)
    {
        if (!head)
        {
            head = tail = order;
        }
        else
        {
            tail->next = order;
            order->prev = tail;
            tail = order;
        }
    }

    void removeOrder(Order* order)
    {
        if ( order->prev)
        {
            order->prev->next = order->next;
        } 
        if ( order->next)
        {
            order->next->prev = order->prev;
        }
        if( order == head) head = order->next;
        if ( order == tail) tail = order->prev;

        order->next = order->prev = nullptr;
    }
};