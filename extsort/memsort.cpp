
#include "memsort.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>

#include <sys/mman.h>

void memSort (int fd, uint64_t size) {
    uint64_t* buffer = static_cast<uint64_t*>(mmap(NULL, size * sizeof(uint64_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    std::sort(buffer, buffer+size);
    munmap(buffer, size * sizeof(uint64_t));
}
