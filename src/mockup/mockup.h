#ifndef MOCKUP_H
#define MOCKUP_H
#include <chrono>
#include <cstdio>
#include <thread>


const auto start = std::chrono::high_resolution_clock::now();

inline struct
{
    void begin(auto) {}
} Serial;


void setup();

void loop();

inline void delay(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline uint32_t millis()
{
    const auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}


#endif //MOCKUP_H
