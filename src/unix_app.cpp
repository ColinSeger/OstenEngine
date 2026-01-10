#include "platform.h"
#include "osten_engine.cpp"
#include <cstdint>
#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>


auto start_time = std::chrono::high_resolution_clock::now();

float platform_memory_mb()
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

void* platform_alloc_memory(unsigned long long size){
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void platform_free_memory(void *pointer, unsigned long long size){
    munmap(pointer, size);
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
