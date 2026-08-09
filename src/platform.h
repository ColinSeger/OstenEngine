#pragma once
#include <stdint.h>

#if defined(__clang__)
    #define CLANG_COMPILER 1

#elif defined(__GNUC__) || defined(__GNUG__)
    #define GCC_COMPILER 1

#elif defined(_MSC_VER)
    #define MSVC_COMPILER 1
#endif


#if CLANG_COMPILER || GCC_COMPILER
    #define thread_static __thread
#elif MSVC_COMPILER
    #define thread_static __declspec(thread)
#else
    #error "No thread_static support"
#endif


typedef struct Timer{
    long sec;
    long ns;
} Timer;

typedef struct FileData{
    const char* filename;
    void* file_data;
    long long file_size;
} FileData;


//static pthread_mutex_t thread_amount_mutex;

typedef void* (*ThreadFunction)(void *data_ptr);

typedef struct Thread{//TODO
    uint32_t thread_index;
    unsigned long long int thread_id;
} Thread;

static thread_static Thread thread_context;
static uint32_t thread_amount = 0;

typedef struct ThreadInitData{//TODO
    uint32_t thread_index;
    ThreadFunction function;
} ThreadInitData;

float platform_memory_mb();

void* platform_alloc_memory(uint64_t amount);

void platform_free_memory(void* pointer, uint64_t size);

uint32_t platform_set_memory(void* destination, uint8_t value, uint64_t length);

long long platform_get_file_size(const char* filename);

FileData platform_load_entire_file(const char* filepath);

void platform_free_file(FileData file);

Timer platform_get_time_handle();

double platform_calc_elapsed_time_seconds(Timer timer);


Thread spawn_thread(ThreadFunction function, uint32_t index);//TODO

void join_thread(Thread thread);//TODO

static inline uint32_t get_thread_index(){
    return thread_context.thread_index;
}

static int32_t ready = 0;
static uint32_t total_thread_amount = 8;

static inline void* thread_init(void* data)
{
    ThreadInitData init_data = *(ThreadInitData*)data;
    thread_amount++;
    thread_context.thread_index = thread_amount;
    while (!ready) {

    }
    void* res = init_data.function(data);
    return res;
}
