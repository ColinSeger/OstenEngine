#pragma once
#include <stdint.h>
#include "../arena.h"
#include "../../external/osten_math.h"

typedef struct OstenWindow {
    u8* window_buffer;
    u8* window_surface;
    u32 window_width;
    u32 window_height;
} OstenWindow;

typedef enum RenderModes {
    RENDER_RECTANGLE,
    RENDER_TEXT,
    RENDER_CIRCLE,
    RENDER_COUNT
} RenderType;

typedef struct BoundingBox {
    u32 x;
    u32 y;
    u32 width;
    u32 height;
} BoundingBox;

typedef struct OstColor {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} OstColor;

typedef struct Vertex {
    vec3 position;
    vec2 uv;
    OstColor color;
} Vertex;

typedef struct VertexBuffer {
    vec3* position;
    OstColor* color;
    vec2* uv;
    u32 count;
} VertexBuffer;

typedef struct RenderCommand {
    BoundingBox bounding_box;
    RenderType command_type;
    OstColor color;
} RenderCommand;


static inline u32 get_pixel_index(u32 x, u32 y, OstenWindow* window){
    const u32 pixel_size = 4;
    u32 result = (y * (pixel_size * window->window_width)) + (x * pixel_size);
    return result;
}

static inline OstColor* get_pixel(u32 x, u32 y, OstenWindow* window){
    u32 index = get_pixel_index(x, y, window);
    return (OstColor*)&window->window_buffer[index];
}

static inline void render_rectangle(OstenWindow* window, BoundingBox bounds, OstColor color){

    u32 width = 0;
    width = bounds.width - bounds.x;
    if(bounds.width > window->window_width && bounds.x < window->window_width){
        width = window->window_width - bounds.x;
    }

    if(bounds.height > window->window_height){
        if(bounds.y > window->window_height){
            bounds.height = 0;
        }else{
            bounds.height = window->window_height - bounds.y;
        }
    }

    for (u32 y = bounds.y; y < bounds.height; y++) {

        OstColor* pixel = get_pixel(bounds.x, y, window);

        for(u32 x = 0; x < width; x++){
            pixel[x] = color;
        }
    }
}

static inline float triangle_thing(vec3 point, vec3 p2, vec3 p3)
{
    return (point.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (point.y - p3.y);
}

static inline void render_circle(OstenWindow* window, vec2 position){
    float radius = 50;

    i32 center_x = position.x;
    i32 center_y = position.y;

    for(i32 x_cord = center_x - radius; x_cord < center_x + radius; x_cord++){
        vec2 cord = {};
        for(i32 y_cord = center_y - radius; y_cord < center_y + radius*2; y_cord++){
            cord.x = x_cord;
            cord.y = y_cord;
            float squared_distance = power_to_f(center_x - cord.x, 2) + power_to_f(center_y - cord.y, 2);

            if(squared_distance < power_to_f(radius, 2)){
                if(cord.x > window->window_width) continue;
                if(cord.y > window->window_height) continue;
                if(cord.x < 0) continue;
                if(cord.y < 0) continue;
                OstColor* pixel = get_pixel(cord.x, cord.y, window);

                *pixel = (OstColor){0, 255, 0, 255};
            }
        }
    }
}

static inline RenderCommand create_box(u32 x, u32 y, u32 width, u32 height, OstColor color){
    RenderCommand result = {};
    result.command_type = RENDER_RECTANGLE;
    result.bounding_box = (BoundingBox) {
        .x = x,
        .y = y,
        .width = width,
        .height = height
    };

    result.color = color;

    return result;
}

static inline u32 render_frame(OstenWindow* main_window, vec2 mouse_cords, MemArena* frame_arena){

    //TODO make a thing that sections up the rendering to rows or colums and then let's the threads do their section
    u32 thread_index = get_thread_index();
    u32 width = main_window->window_width;
    u32 height = main_window->window_height;
    u8 thread_color = thread_index * 40;

    u32 work_size = height / total_thread_amount;
    u32 work_residue = height % total_thread_amount;


    u32 start_x = work_size * thread_index;
    u32 end_x = work_size * (thread_index +1);

    end_x += work_residue;
    if(thread_index != 0){
        start_x += work_residue;
    }

    Vertex vertex1 = {};
    Vertex vertex2 = {};
    Vertex vertex3 = {};
    Vertex vertex4 = {};
    Vertex vertex5 = {};
    Vertex vertex6 = {};

    vertex1.position = vec3_f(0, 0, 0);
    vertex1.color = (OstColor){255, thread_color, 0, 255};

    vertex2.position = vec3_f((float)main_window->window_width, 0, 0);
    vertex2.color = (OstColor){0, thread_color, 100, 255};

    vertex3.position = vec3_f(0, (float)main_window->window_height, 0);
    vertex3.color = (OstColor){0, thread_color, 100, 255};

    vertex4.position = vec3_f((float)main_window->window_width, (float)main_window->window_height, 0);
    vertex4.color = (OstColor){0, thread_color, 255, 255};

    vertex5.position = vec3_f((float)main_window->window_width, 0, 0);

    vertex6.position = vec3_f(0, (float)main_window->window_height, 0);

    Vertex vertexes_buffer[] = {vertex1, vertex2, vertex3, vertex4, vertex5, vertex6};

    const u32 pixel_size = 4;

    for(u32 y = start_x; y < end_x; y++){
        for(u32 x = 0; x < width; x++){
            u32 result = (y * (pixel_size * width)) + (x * pixel_size);
            for(u32 i = 0; i < 6; i+=3){
                i32 has_neg, has_pos;

                vec3 v1 = vertexes_buffer[i].position;
                vec3 v2 = vertexes_buffer[i+1].position;
                vec3 v3 = vertexes_buffer[i+2].position;

                vec3 point = vec3_f((float)x, (float)y, 0);
                float t1 = triangle_thing(point, v1, v2);
                float t2 = triangle_thing(point, v2, v3);
                float t3 = triangle_thing(point, v3, v1);

                has_neg = (t1 < 0) || (t2 < 0) || (t3 < 0);
                has_pos = (t1 > 0) || (t2 > 0) || (t3 > 0);

                if(!(has_neg && has_pos)){
                    OstColor* pixel = (OstColor*)&main_window->window_buffer[result];
                    *pixel = vertexes_buffer[i].color;
                }else{
                    //pixel[x] = (OstColor){0, 255, 0, 255};
                }
            }
        }
    }

    return 0;
}

static inline uint32_t cleanup_renderer(){
    return 0;
}
