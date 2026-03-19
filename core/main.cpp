#include <iostream>
#include <chrono>
#include <vector>
#include "memory/OrderPool.hpp"

struct Timer
{
    std :: string name;
    std :: chrono::high_resolution_clock::time_point start;

    Timer(std::string n): name(n), start(std::chrono::high_resolution_clock::now()) {}
    ~Timer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << name << ": " << duration << " micro s" << std::endl;
    }
};

int main()
{
    const size_t ITERATIONS = 1000000;

    {
        Timer t("Standard new/delete");
        std::vector<Order*> orders;
        orders.reserve(ITERATIONS);

        for (size_t i = 0; i < ITERATIONS; ++i) {
            orders.push_back(new Order{i, 100.0, 10, Side::Buy});
        }
        for (auto o : orders) {
            delete o;
        }
    }


    {
            OrderPool pool(ITERATIONS);
            Timer t("Custom Memory Allocation");
            std::vector<Order*> orders;
            orders.reserve(ITERATIONS);

            for (size_t i = 0; i < ITERATIONS; ++i) 
            {
            orders.push_back(pool.Allocate(i, 100.0, 10, Side::Buy));
            }

            for (auto o : orders) {
            pool.deallocate(o); 
            }
    }

    return 0;
    
}