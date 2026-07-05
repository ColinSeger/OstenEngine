#include "platform.h"
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include "../external/RGFW.h"
#include "osten_engine.h"
//#include "engine/game_load.h"
// #include "game/total_cheese.hpp"

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

void* platform_alloc_memory(uint64_t size){
    return mmap((void*)300000000, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void platform_free_memory(void *pointer, uint64_t size){
    munmap(pointer, size);
}

uint32_t platform_set_memory(void *destination, uint8_t value, uint64_t length){
    //memset()
    return 0;
}

Timer platform_get_time_handle(){
    Timer result = {};
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    result.sec = start.tv_sec;
    result.ns = start.tv_nsec;
    return result;
}

double platform_calc_elapsed_time_seconds(Timer timer){
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - timer.sec) + (end.tv_nsec - timer.ns) / 1e9;
}

long long platform_get_file_size(const char* filename) {
    struct stat file_stats;
    if (stat(filename, &file_stats) == -1)
        return -1;
    return file_stats.st_size;
}

FileData platform_load_entire_file(const char* filepath){//Add error handling
    FileData result = {};
    result.filename = filepath;
    long long file_size = platform_get_file_size(result.filename);
    int file = open(result.filename, O_RDONLY);

    result.file_data = mmap(0, file_size, PROT_READ, MAP_PRIVATE, file, 0);
    result.file_size = file_size;
    close(file);
    return result;
}

void platform_free_file(struct FileData file){
    munmap(file.file_data, file.file_size);
}

int main(){

    OstenEngine osten_engine = {};

    OstenWindow engine_window = {
        .window_width = 848,
        .window_height = 480
    };


    float frame_count = 0;

    float fps = 0;

    float delta_time = 0;

    RGFW_event window_event = {};

    Timer last_tick = platform_get_time_handle();

    Timer frame_timer = {};

    init_mem_arena(&osten_engine.mem_arena, 256 * MB);

    RGFW_window* main_window = RGFW_createWindow("OstenEngine", 0, 0, engine_window.window_width, engine_window.window_height, RGFW_windowCenter);

    engine_window.window_buffer = arena_alloc_memory(&osten_engine.mem_arena, (uint64_t)(engine_window.window_width * engine_window.window_height * 4));

    engine_window.window_surface = (uint8_t*)RGFW_createSurface(engine_window.window_buffer, engine_window.window_width, engine_window.window_height, RGFW_formatRGBA8);

    void* library = dlopen("./libosten.so", RTLD_NOW);

    typedef void (*render_game)(
        OstenEngine *,
        OstenWindow *,
        vec2
    );

    typedef void (*debug)(
        OstenWindow* engine_window,
        MemArena* mem_arena
    );

    if (!library) {
        printf("dlopen: %s\n", dlerror());
    }

    render_game test = dlsym(library, "update_render_game");

    if (!test) {
        printf("dlopen: %s\n", dlerror());
    }
    printf("render = %p\n", test);
    debug d = dlsym(library, "debug_init_clay");
    d( &engine_window, &osten_engine.mem_arena);

    while(!RGFW_window_shouldClose(main_window)){

        bool was_resized = 0;
        while (RGFW_window_checkEvent(main_window, &window_event)) {
            switch (window_event.type) {
                case RGFW_eventNone:{
                    break;
                }
                case RGFW_windowResized:{
                    if(window_event.update.w <= 0 || window_event.update.h <= 0) {
                        break;
                    }
                    engine_window.window_width = window_event.update.w;
                    engine_window.window_height = window_event.update.h;

                    was_resized = 1;

                    break;
                }
                case RGFW_windowMinimized:{
                    break;
                }
            }
        }

        if(was_resized){
            RGFW_surface_free((RGFW_surface*)engine_window.window_surface);
            pop_arena(&osten_engine.mem_arena, engine_window.window_buffer);

            engine_window.window_buffer = arena_alloc_memory(&osten_engine.mem_arena, (uint64_t)(engine_window.window_width * engine_window.window_height * 4));
            engine_window.window_surface = (uint8_t*)RGFW_createSurface(engine_window.window_buffer, engine_window.window_width, engine_window.window_height, RGFW_formatRGBA8);
        }


        delta_time = platform_calc_elapsed_time_seconds(last_tick);
        float frame_time = platform_calc_elapsed_time_seconds(frame_timer);

        //If a frame takes longer that 500ms it will cap delta time since this should hopefully only happen when debugging
        if (delta_time > 0.50) delta_time = 0.50;

        if(frame_time > 1) {
            fps = frame_count / frame_time;
            printf("Fps: %f \n", fps);
            frame_timer = platform_get_time_handle();
            frame_count = 0;
        }

        int32_t x, y;

        RGFW_window_getMouse(main_window, &x, &y);

        //update_render_game(&osten_engine, &engine_window, (vec2){.x = x,.y = y});

        test(&osten_engine, &engine_window, (vec2){.x = x,.y = y});

        if(was_resized){
            memset(engine_window.window_buffer, 0, engine_window.window_width * engine_window.window_height * 4);
        }

        RGFW_window_blitSurface(main_window, (RGFW_surface*)engine_window.window_surface);

        frame_count +=1;
        last_tick = platform_get_time_handle();
    }

    RGFW_surface_free((RGFW_surface*)engine_window.window_surface);
    return 0;
}
