#pragma once

struct Timer{
    double sec;
    double ns;
};

struct FileData{
    const char* filename;
    void* file_data;
    unsigned long long file_size;
};

float platform_memory_mb();

void* platform_alloc_memory(unsigned long long amount);

void platform_free_memory(void* pointer, unsigned long long size);

long long platform_get_file_size(const char* filename);

struct FileData platform_load_entire_file(const char* filepath);

void free_file(struct FileData file);

struct Timer platform_get_time_handle();

double platform_calc_elapsed_time_seconds(struct Timer timer);
