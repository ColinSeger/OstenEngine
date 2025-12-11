#include "osten_engine.cpp"
#include <cstdint>


OstenEngine start(uint32_t width, uint32_t height, const char* name){
    return OstenEngine(width, height, name);
}

uint8_t run(OstenEngine& engine){
    engine.main_game_loop();
    return 0;
}
