
#include "memsort.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

void memSort (int fd, uint64_t size) {
    uint64_t* buffer = static_cast<uint64_t*>(mmap(NULL, size * sizeof(uint64_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    std::sort(buffer, buffer+size);
    munmap(buffer, size * sizeof(uint64_t));
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Usage:" << endl;
        cout << argv[0] << " <inputFile> <outputFile>" << endl << endl;
        exit (1);
    }

    struct stat fileInfo;
    stat(argv[1], &fileInfo);
    uint64_t numElements = fileInfo.st_size / sizeof(uint64_t);

    int fdInput = open (argv[1], O_RDONLY);
    int fdOutput = open (argv[2], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    sendfile(fdOutput, fdInput, 0, fileInfo.st_size);
    close (fdInput);

    memSort (fdOutput, numElements);
    close (fdOutput);

    return 0;
}
