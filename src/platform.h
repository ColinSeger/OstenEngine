

float platform_memory_mb();

void* platform_alloc_memory(unsigned long long amount);

void platform_free_memory(void* pointer, unsigned long long size);

void* load_file(const char* filepath);

double get_time_since_start();
