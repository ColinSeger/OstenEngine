#include "osten_engine.cpp"
#include <cstdint>
#include <windows.h>
#include <processthreadsapi.h>
#include <psapi.h>


float value(){
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
OstenEngine start(uint32_t width, uint32_t height, const char* name){
    PlatformLayer platform_layer{
        &value
    };
    return OstenEngine(width, height, name, platform_layer);
}
/*
*/
uint8_t run(OstenEngine& engine){
    engine.main_game_loop();
    return 0;
}
