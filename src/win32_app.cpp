#pragma once
#include "platform.h"
#include "engine/message_system/message.h"
#include <stddef.h>
#include <stdint.h>
#include "osten_engine.cpp"
#include <windows.h>
#include <minwindef.h>
#include <handleapi.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <psapi.h>
#include <winnt.h>
#include "engine/game_load.h"
#include "game/total_cheese.hpp"

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


size_t get_file_size(const char* filepath){
    LARGE_INTEGER size;
        HANDLE file = CreateFileA(
            filepath,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (file == INVALID_HANDLE_VALUE)
            return -1;

        if (!GetFileSizeEx(file, &size)) {
            CloseHandle(file);
            return -1;
        }

        CloseHandle(file);
        return (long long)size.QuadPart;
}

struct FileData platform_load_entire_file(const char* filepath){
    FileData result = {};
    result.filename = filepath;
    HANDLE file_handle = CreateFileA(
        filepath,
        GENERIC_READ,
        FILE_READ_DATA,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    LARGE_INTEGER size{};
    BOOL s = GetFileSizeEx(file_handle, &size);

    if(!s) return {};//Todo

    result.file_size = size.QuadPart;

    result.file_data = VirtualAlloc(0, result.file_size, MEM_COMMIT, PAGE_READWRITE);
    bool success = ReadFile(
      file_handle,
      result.file_data,
      result.file_size,
      0,
      0
    );
    return result;
}

void free_file(struct FileData file){
    VirtualFree(file.file_data, file.file_size, MEM_DECOMMIT);
}

struct Timer platform_get_time_handle(){
    Timer result = {};

    LARGE_INTEGER ticks;

    if (!QueryPerformanceCounter(&ticks)){
        return {};
    }

    result.sec =  (double)ticks.QuadPart;

    result.ns = ticks.LowPart;
    return result;
}

double platform_calc_elapsed_time_seconds(struct Timer timer){
    LARGE_INTEGER frequency;
    LARGE_INTEGER ticks;
    QueryPerformanceFrequency(&frequency);

    if (!QueryPerformanceCounter(&ticks)){
        return 0;
    }
    return (double)(ticks.QuadPart - timer.sec) / frequency.QuadPart;
}

OstenEngine start(uint32_t width, uint32_t height, const char* name){
    return OstenEngine(width, height, name);
}

uint8_t run(OstenEngine& engine){
    load_game_data((char*)"src/game/game_data.txt");
    load_game_reasources();

    procces_all_commands(&engine.render_pipeline, &engine.heap_stack);

    init_game(engine);

    last_tick = platform_get_time_handle();

    while(!engine.should_close){
        procces_all_commands(&engine.render_pipeline, &engine.heap_stack);
        size_t free = calculate_colliders(&engine.heap_stack);
        update_game(engine.delta_time, engine);
        engine.draw_frame();
        free_arena(&engine.heap_stack, free);
    }
    return 0;
}
