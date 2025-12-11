#include "osten_engine.cpp"
#include <cstdint>
#include <stdio.h>
#include <unistd.h>


OstenEngine start(uint32_t width, uint32_t height, const char* name){
    return OstenEngine(width, height, name);
}

long get_size()
{
    long size, resident, shared, text, lib, data, dirty;

    FILE *f = fopen("/proc/self/statm", "r");
    if (f == NULL) return 1;

    fscanf(f, "%ld %ld %ld %ld %ld %ld %ld",
            &size, &resident, &shared, &text, &lib, &data, &dirty);
    fclose(f);

    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;


    // printf("Virtual memory: %ld MB\n", (size * page_size_kb) / 1024);
    // printf("Resident Set Size (RSS): %ld MB\n", (resident * page_size_kb) / 1024);
    // printf("Shared: %ld MB\n", (shared * page_size_kb) / 1024);
    // printf("Data + Stack: %ld MB\n", (data * page_size_kb) / 1024);
    return size / 1024;
}

uint8_t run(OstenEngine& engine){

    engine.main_game_loop(&get_size);
    return 0;
}
