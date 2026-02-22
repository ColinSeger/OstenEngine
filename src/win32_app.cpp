#pragma once
#include "engine/message_system/message.h"
#include "platform.h"
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
#include "game/total_cheese.hpp"

static auto start_time = std::chrono::high_resolution_clock::now();

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

size_t get_file_size(const char *filepath){
    PLARGE_INTEGER result{};

    HANDLE file_handle = CreateFile(filepath, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if(file_handle == INVALID_HANDLE_VALUE) return result->QuadPart;

    GetFileSizeEx(file_handle, result);

    return result->QuadPart;
}

void* load_entire_file(const char* filepath){

    HANDLE file_handle = CreateFile(filepath, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    PLARGE_INTEGER size{};
    GetFileSizeEx(file_handle, size);
    /*
    BOOL result = ReadFile(
      file_handle,
      [out]               LPVOID       lpBuffer,
      [in]                DWORD        nNumberOfBytesToRead,
      [out, optional]     LPDWORD      lpNumberOfBytesRead,
      [in, out, optional] LPOVERLAPPED lpOverlapped
    );
    */
    return 0;
}

OstenEngine start(uint32_t width, uint32_t height, const char* name){
    start_time = std::chrono::high_resolution_clock::now();
    return OstenEngine(width, height, name);
}

uint8_t run(OstenEngine& engine){
    load_game_reasources();

    procces_all_commands(&engine.render_pipeline, &engine.heap_stack);

    init_game(engine);
    last_tick = std::chrono::high_resolution_clock::now();

    calculate_colliders(&engine.heap_stack);

    while(!engine.should_close){
        update_game(engine.delta_time, engine);
        engine.draw_frame();
    }
    return 0;
}
