#include "osten_engine.cpp"
#include <cstdint>
#include <unistd.h>

float get_size()
{//https://libstatgrab.org/ Look INto
    using std::ios_base;
    using std::ifstream;
    using std::string;

    double vm_usage = 0.0;
    double resident_set = 0.0;

    // 'file' stat seems to give the most reliable results
    //
    ifstream stat_stream("/proc/self/stat",ios_base::in);

    // dummy vars for leading entries in stat that we don't care about
    //
    string pid, comm, state, ppid, pgrp, session, tty_nr;
    string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    string utime, stime, cutime, cstime, priority, nice;
    string O, itrealvalue, starttime;

    // the two fields we want
    //
    unsigned long virtual_size;
    long resident_set_size;

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                >> utime >> stime >> cutime >> cstime >> priority >> nice
                >> O >> itrealvalue >> starttime >> virtual_size >> resident_set_size; // don't care about the rest

    stat_stream.close();

    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
    virtual_size /= 1024;//Make into kb
    virtual_size /= 1024;//Make into mb
    float result = virtual_size;
    return result;
}

OstenEngine start(uint32_t width, uint32_t height, const char* name){
    PlatformLayer platform_layer{
        &get_size
    };
    return OstenEngine(width, height, name, platform_layer);
}

uint8_t run(OstenEngine& engine){

    engine.main_game_loop();
    return 0;
}
