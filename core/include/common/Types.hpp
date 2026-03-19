// Custom Data Types are defined here in order to save memory and make applicatin more cache friendly.

#pragma once //<- header file is included only once.
#include <cstdint> //<- Gives fixed-size integer types.

enum class Side : uint8_t { Buy = 0 , Sell = 1} ; // using enum Class for Type Safety ( everything is assinged a integer value in a ENUM)

struct Order
{
    uint64_t id;        //8 byte (millions of transcations each day)
    double price;       //8 byte
    uint32_t quantity;  //4 byte
    Side side;          //1 byte

    //pointers for Time-Priority Queue ( each order knows which was previous Order and which is next order in queue)
    Order* next = nullptr;
    Order* prev = nullptr;
};



