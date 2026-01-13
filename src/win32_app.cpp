#include "platform.h"
#include "osten_engine.cpp"
#include <cstdint>
#include <windows.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <psapi.h>

auto start_time = std::chrono::high_resolution_clock::now();

float platform_memory_mb(){
    PROCESS_MEMORY_COUNTERS memory_counters;
    auto handle = GetCurrentProcess();
    if(GetProcessMemoryInfo(handle , &memory_counters, sizeof(memory_counters))){
        long t = memory_counters.PagefileUsage;
        t/= 1024;;
        float result = t;
        return result / 1024;
    }

    return 0;
}

void* platform_alloc_memory(unsigned long long size)
{
    return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
}

void platform_free_memory(void* pointer, unsigned long long size)
{
    VirtualFree(pointer, size, MEM_DECOMMIT);
}

double get_time_since_start(){
    auto current_time = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::chrono::seconds::period>(current_time - start_time).count();
}

OstenEngine start(uint32_t width, uint32_t height, const char* name){
    start_time = std::chrono::high_resolution_clock::now();
    return OstenEngine(width, height, name);
}

uint8_t run(OstenEngine& engine){
    engine.main_game_loop();
    return 0;
}
