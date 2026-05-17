#pragma once
#include <stdint.h>

typedef struct Timer{
    double sec;
    double ns;
} Timer;

typedef struct FileData{
    const char* filename;
    void* file_data;
    long long file_size;
} FileData;

float platform_memory_mb();

void* platform_alloc_memory(uint64_t amount);

void platform_free_memory(void* pointer, uint64_t size);

uint32_t platform_set_memory(void* destination, uint8_t value, uint64_t length);

long long platform_get_file_size(const char* filename);

FileData platform_load_entire_file(const char* filepath);

void platform_free_file(FileData file);

Timer platform_get_time_handle();

double platform_calc_elapsed_time_seconds(Timer timer);
