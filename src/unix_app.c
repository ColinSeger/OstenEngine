#include "platform.h"
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include "../external/RGFW.h"
#include "osten_engine.h"

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

Thread spawn_thread(ThreadFunction function, u32 index){//TODO
    Thread result = {};
    pthread_t thread;

    ThreadInitData data = {};
    data.thread_index = index;
    data.function = function;

    i32 thread_result = pthread_create(&thread, 0, thread_init, &data);

    printf("Thread result %i \n", thread_result);

    result.thread_id = thread;

    return result;
}

void join_thread(Thread thread){//TODO
    thread_amount--;
}

typedef void (*render_game)(
    OstenEngine *,
    OstenWindow *,
    vec2
);

typedef void (*init_engine_f)(
    OstenWindow* engine_window,
    OstenEngine* engine
);

void barrier_sync(void* barrier)
{
    if(total_thread_amount > 0){
        pthread_barrier_wait(barrier);
    }
}

//TEMP DATA TODO MOVE TO SENSIBLE LOCATION
static RGFW_window* main_window = {};
static OstenEngine osten_engine = {};
static OstenWindow engine_window = {
    .window_width = 848,
    .window_height = 480
};

static pthread_barrier_t barrier;

static void* multithread_entry(void* data)
{


    float frame_count = 0;

    float fps = 0;

    float delta_time = 0;

    RGFW_event window_event = {};

    Timer last_tick = platform_get_time_handle();

    Timer frame_timer = {};

    //init(&engine_window, &osten_engine);

    if(thread_context.thread_index == 0){
        init_engine(&engine_window, &osten_engine);

        main_window = RGFW_createWindow(data, 0, 0, engine_window.window_width, engine_window.window_height, RGFW_windowCenter);

        engine_window.window_buffer = arena_alloc_memory(&osten_engine.mem_arena, (uint64_t)(engine_window.window_width * engine_window.window_height * 4));

        engine_window.window_surface = (uint8_t*)RGFW_createSurface(engine_window.window_buffer, engine_window.window_width, engine_window.window_height, RGFW_formatRGBA8);

        ready = 1;
    }

    while(!RGFW_window_shouldClose(main_window)){

        i32 was_resized = 0;
        if(thread_context.thread_index == 0){
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
        }
        barrier_sync(&barrier);

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

        i32 x, y;

        RGFW_window_getMouse(main_window, &x, &y);

        EngineData render_data = {
            .engine = &osten_engine,
            .window = &engine_window,
            .mouse_cords = (vec2){.x = x, .y = y}
        };

        update_render_game(&render_data);

        //renderer(&osten_engine, &engine_window, (vec2){.x = x,.y = y});


        barrier_sync(&barrier);

        if(thread_context.thread_index == 0){
            if(was_resized){
                memset(engine_window.window_buffer, 0, engine_window.window_width * engine_window.window_height * 4);
            }
            RGFW_window_blitSurface(main_window, (RGFW_surface*)engine_window.window_surface);
        }

        frame_count +=1;
        last_tick = platform_get_time_handle();
    }
    //pthread_join(thread, 0);
    if(thread_context.thread_index == 0){
        RGFW_surface_free((RGFW_surface*)engine_window.window_surface);
    }

    return 0;
}


int main(){
/*
 *
    void* library = dlopen("./libosten.so", RTLD_NOW);

    if (!library) {
        printf("dlopen: %s\n", dlerror());
        return 1;
    }

    render_game renderer = dlsym(library, "update_render_game");
    init_engine_f init = dlsym(library, "init_engine");

    if (!renderer) {
        printf("dlopen: %s\n", dlerror());
        return 1;
    }
 */
    i32 thread_amount = total_thread_amount;

    static ThreadFunction function = multithread_entry;

    for (i32 i = 1; i < thread_amount; i++) {

        spawn_thread(function, i);
    }

    i32 res = pthread_barrier_init(&barrier, 0, thread_amount);

    if(res != 0) {
        //Handle failed barrier
    }

    ThreadInitData data = {};

    multithread_entry(&data);

    return 0;
}
