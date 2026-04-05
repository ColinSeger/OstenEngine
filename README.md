# OstenEngine

Is a work in progress game "engine" made in C++ and Vulkan, tried to use minimal dependencies for this project while still keeping 
the development within a resonable time.

To build this project you need to be able to run the vkcube command, info on that can be found at https://vulkan.lunarg.com/ 


## Build

### Windows

#### Prerequisites

You will need Cmake to be able to build the build files You can find out more on that here https://cmake.org/.

Vulkan is required for this project to run, you can check if you have Vulkan dev tools installed by running the console command 
"vkcube" and if a cube saying lunarg shows up you have Vulkan tools already, otherwhise go to https://vulkan.lunarg.com/ and install the Vulkan tools.

The compiler I used for testing on Windows was their MSVC compiler but I belive it should be able to compile using other compilers that can be used with Cmake.

On windows it should just be to run cmake --build command in the Engine folder if that does not work you might not have a compiler installed,
it could also be that you need to install GLFW on you system.

### Linux

-Requirements
- GCC or Clang compiler
- Vulkan

Slightly more advanced than windows but still quite simple, all you should need is vulkan developer packages installed on your system and a
C++ compiler. First you will need to compile GLFW into a library and the same for ImGui, for ImGui there is a helper script in the external folder (imgui_build.sh).

When both of them are libraries you should just need to run (linux_builder.sh) and it should work.

Though if you are not using g++ you would need to change to your desired builder in (linux_builder.sh)

### Additional Step

The engine at the moment does not support figuring out file paths automatically and instead you will need to either run it in the engine folder or move required items to the same folder as the executable

* Means that files in this folder are needed
Folder paths that need to exist in running directory:
src/renderer/shaders/* 
assets/debug_assets/*

### Engine Execution Summary

![Brief Layout](Documentation/BriefSummary.webp)

The data loaded by the engine is inside a text file in "game/game_data.txt"

That text file could contain something like this.
´
    !1
    #Model_Paths
    assets/debug_assets/Cube.obj
    assets/debug_assets/viking.bin
    #Texture_Paths
    assets/debug_assets/debug_texture.png
    assets/debug_assets/funny_texture.jpg
    #Render_Instances //Corresponds to model_index/texture_index
    0/1/4000
    1/2/4000
´

The parser looks at ! to find parsing version 1 is the first and only one that exists so far.

Then the parser will look at # to load data the text after the # does not mean anything it is just to make it easier to understand for humans.
The first # is model paths the asset loader will look in the 2nd # tells the asset loader to load those textures then the final # means that the render pipeline should prepare 
render instances using the first number as a model index the second as a texture index and third is the capacity of this renderable model.

Quick Start Game Code:
´

    //Change this line in your platformlayer to your C or Cpp file containing what you want to run
    #include "game/total_cheese.hpp"

    struct GameData{
      //Put whatever data you want to store in the game
      bool* paused_state;
      void (*game_code)(OstenEngine*, GameData*) = nullptr;
    };
    
    static void run_game(OstenEngine* engine, GameData* data){
      //Game Code
    }
    
    static void menu_state(OstenEngine* engine, GameData* data){
      //Some menu code
      
      if(example_button){
        data->game_code = run_game;
      }
    }
    
    //Function called by platform layer
    static void load_game_resources(OstenEngine* engine, GameData* data){
    
      struct InstanceData render_ids = {};
    
      render_ids.model_index = loaded_models.size()-1;
      render_ids.texture_index = 1;
      render_ids.capacity = 2;
      
      //Tells engine to create a model with texture 1 and a capacity of 2
      add_message_f(MessageType::CreateRenderable, sizeof(InstanceData), (char*)&render_ids);
    
      data->game_code = menu_state;
    }

´


Right now the OstenEngine API is very and I mean very bare-bones but some actions included are creating entities creating model instances with a material and loading of textures/models

The image formats supported by load texture is png and jpeg at the moment

Example Usage:
´

    char* message = "cube.bin";
    uint8_t message_size = sizeof(message);
    add_message_f(MessageType::LoadModel, message_size, message);

´
