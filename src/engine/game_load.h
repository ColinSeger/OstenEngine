#pragma once
#include "message_system/message.h"
#include "../platform.h"
#include <stddef.h>
#include <stdint.h>

struct Line{
    char string[255];
    unsigned short length;
};

enum LoadType : uint8_t{
    Version,
    Models,
    Textures,
    Render
};

static inline bool get_line(struct Line* line,struct FileData data, long long* offset){
    if(*offset > data.file_size) return false;
    line->length = 0;

    long long test = line->length + *offset;

    while (((char*)data.file_data)[test] != '\n') {
        test++;
    }

    line->length = test - *offset;
    if(line->length <= 0 || line->length > 255)  {
        return false;
    }

    memcpy(line->string, &((char*)data.file_data)[*offset], line->length * sizeof(char));
    *offset+=line->length +1;
    return true;
}


static inline void load_game_data(char* file_path){
    struct InstanceData render_ids {};

    FileData file_data = platform_load_entire_file(file_path);
    if(file_data.file_size <= 0){
        Debug::log((char*)"Failed to load game_data");
        return;
    }

    Line text_line = {};
    long long offset = 0;
    LoadType load_type = LoadType::Version;

    while (get_line(&text_line, file_data, &offset)) {
        if(text_line.string[0] == '!' || text_line.string[0] == ' ' || text_line.string[0] == '\n') continue;
        if(text_line.string[0] == '#'){
            load_type = (LoadType)(load_type + 1);
            continue;
        }
        if(load_type ==  LoadType::Models){
            add_message_f(MessageType::LoadModel, text_line.length, text_line.string);
        }
        else if(load_type ==  LoadType::Textures){
            add_message_f(MessageType::LoadTexture, text_line.length, text_line.string);
        }
        else if(load_type == LoadType::Render){
            size_t index = 0;
            render_ids.model_index = parse_to_uint32_length(text_line.string, &index);
            render_ids.texture_index = parse_to_uint32_length(&text_line.string[index++ +1], &index);
            render_ids.capacity = parse_to_uint32_length(&text_line.string[index++ +1], &index);

            add_message_f(MessageType::CreateRenderable, sizeof(InstanceData), (char*)&render_ids);
        }
    }
}
