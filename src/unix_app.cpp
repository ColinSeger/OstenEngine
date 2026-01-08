#include "platform.h"
#include "osten_engine.cpp"
#include <cstdint>
#include <unistd.h>

inline float platform_memory_mb()
{//https://libstatgrab.org/ Look Into
    FILE *file = fopen("/proc/self/statm", "r");
    if (!file)
        return 0.0f;

    long size = 0;
    long resident = 0;

    if (fscanf(file, "%ld %ld", &size, &resident) != 2) {
        fclose(file);
        return 0.0f;
    }

    fclose(file);

    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;

    // Resident Set Size in KB
    long rss_kb = resident * page_size_kb;

    // Convert to MB
    return rss_kb / 1024.0f;
}

inline void* platform_alloc_memory(unsigned long long size)
{
    return malloc(size);
}

OstenEngine start(uint32_t width, uint32_t height, const char* name){
    return OstenEngine(width, height, name);
}

uint8_t run(OstenEngine& engine){

    engine.main_game_loop();
    return 0;
}
