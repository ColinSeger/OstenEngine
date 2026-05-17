#include "platform.h"
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
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

void* platform_alloc_memory(unsigned long long size){
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void platform_free_memory(void *pointer, unsigned long long size){
    munmap(pointer, size);
}

struct Timer platform_get_time_handle(){
    Timer result = {};
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    result.sec = start.tv_sec;
    result.ns = start.tv_nsec;
    return result;
}

double platform_calc_elapsed_time_seconds(struct Timer timer){
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

struct FileData platform_load_entire_file(const char* filepath){//Add error handling
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

void start(uint32_t width, uint32_t height, const char* name, OstenEngine* engine){
    create_osten_engine(width, height, name, engine);
}

uint8_t run(OstenEngine* engine){
    //load_game_data((char*)"src/game/game_data.txt");
    // GameData game_data = {};

    // game_data.game_code = load_game_resources;

    last_tick = platform_get_time_handle();

    // UIData ui_data = {};
    // ui_data.inspecting = &engine.inspecting;
    // ui_data.paused_state = &engine.paused_update;
    // ui_data.render_pipe = &engine.render_pipeline;
    // ui_data.target_point = &engine.target_point;
    // ui_data.file_explorer = &engine.file_explorer;
    // ui_data.target_index = &target_index;

    // game_data.paused_state = &engine.paused_update;

    while(!engine->should_close){
        //procces_all_commands(&engine->render_pipeline, &engine->heap_stack);
        //begin_imgui_editor_poll(engine.main_window, &ui_data, engine.open_window, engine.fps, &engine.heap_stack);
        if(!engine->paused_update){
            //calculate_colliders();

            // game_data.game_code(&engine, &game_data);
        }
        draw_frame(engine);
    }
    return 0;
}
