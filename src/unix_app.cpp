#include "osten_engine.cpp"
#include <cstdint>
#include <stdio.h>
#include <unistd.h>


OstenEngine start(uint32_t width, uint32_t height, const char* name){
    return OstenEngine(width, height, name);
}

float get_size()
{
    long size;

    FILE *f = fopen("/proc/self/statm", "r");
    if (f == NULL) return 1;

    fscanf(f, "%ld",
            &size);
    fclose(f);

    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
    size /= 1024;
    float result = size;
    return result;
}

uint8_t run(OstenEngine& engine){

    engine.main_game_loop(&get_size);
    return 0;
}
