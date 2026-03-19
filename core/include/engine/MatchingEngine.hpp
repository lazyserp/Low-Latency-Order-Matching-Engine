#pragma once
#include "OrderBook.hpp"
#include <iostream>


class MatchingEngine
{
    private:
        OrderBook book;

    public:
        void ProcesOrder(Order* newOrder)
        {
            if ( newOrder->side == Side::Buy)
            {
                matchOrder(newOrder,book.asks);
            }
            else matchOrder(newOrder, book.bids);

            if (newOrder->quantity > 0) {
            book.addOrder(newOrder);
        }

        }

    private:
        template <typename T>
        void matchOrder(Order* newOrder, T& oppositeSide)
        {
            auto it = oppositeSide.begin();

            while ( it != oppositeSide.end() && newOrder.price > 0)
            {
                PriceLevel& level = it->second;

            // Check if price allows a match        
            // For Buy: NewPrice >= SellPrice | For Sell: NewPrice <= BuyPrice
            bool canMatch = (newOrder->side == Side::Buy) ? (newOrder->price >= level.price) : (newOrder->price <= level.price);

            if(!canMatch) break;

            // Match against orders at this price level (Time Priority)
            Order* currentOppositeOrder = level.head;
            while ( currentOppositeOrder && newOrder->quantity > 0)
            {
                uint32_t matchQty = std::min(newOrder->quantity, currentOppositeOrder->quantity);

                std::cout << "MATCH: Order " << newOrder->id << " with " 
                          << currentOppositeOrder->id << " for " << matchQty << " units\n";


                newOrder->quantity -= matchQty;
                currentOppositeOrder->quantity -= matchQty;


                if ( currentOppositeOrder->quantity == 0)
                {
                    Order* completedOrder = currentOppositeOrder;
                    currentOppositeOrder = currentOppositeOrder->next;
                    level.removeOrder(completedOrder);
                    // In the future, we would return completedOrder to the Memory Pool here
                }
                else{
                    currentOppositeOrder = currentOppositeOrder->next;
                }
            }
            if (level.head == nullptr) {
                it = oppositeSide.erase(it);
            } else {
                ++it;
            }

            }
            
        }
};
