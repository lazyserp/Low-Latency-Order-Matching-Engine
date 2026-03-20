// We dont wait for OS to allocate memory for a new Order, Instead we create a 'pool' beforehand to avoid delay

#pragma once
#include "common/Types.hpp"
#include <vector>
#include <stdexcept> //exception classes

class OrderPool
{
    private:
    void* rawMemory = nullptr;
    Order* pool = nullptr;
    Order* freeListHead = nullptr;
    size_t poolSize;
    
    public:
        explicit OrderPool(size_t size) : poolSize(size) // explicit avoids automatic type conversions of size passed in the function call
        {
            //Pre-allocate the entire block on the heap ONCE.
            //Using 'operator new' to allocate raw memory eliminaing 2 steps of 'new' keyword.
            rawMemory = ::operator new[](size * sizeof(Order));
            pool = static_cast<Order*>(rawMemory); // Casting void* to Order* to iterate and allocate memory.

            
            //now the "Free List". 
            //We use the 'next' pointer of the Order to point to the next free slot.
            for (size_t i = 0; i < size - 1; ++i)
            {
                pool[i].next = &pool[i+1];
            }
            pool[size - 1].next = nullptr;
            freeListHead = &pool[0];

        }

        // O(1) Memory Allocation by just moving the pointer
        Order* Allocate(uint64_t id, double price, uint32_t qty , Side side)
        {
            if (!freeListHead)
            {
                throw std::runtime_error("Order Pool Exhausted");
            }

            Order* order = freeListHead;
            freeListHead = freeListHead->next;

            order->id = id;
            order->price = price;
            order->quantity = qty;
            order->side = side;
            order->next = nullptr;
            order->prev = nullptr;

            return order;
        }

        // O(1) Deallocation: Put the object back at the front of the free list.
        void deallocate(Order* order)
        {
            order->next = freeListHead;
            freeListHead = order;
        }

        // Destructor to free up memory
        ~OrderPool() {
        ::operator delete[](rawMemory);
        }
};
