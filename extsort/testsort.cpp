
#include <iostream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage:" << endl;
        cout << argv[0] << " <testFile>" << endl << endl;
        return 1;
    }

    struct stat fileInfo;
    stat(argv[1], &fileInfo);
    uint64_t numElements = fileInfo.st_size / sizeof(uint64_t);

    int fd = open (argv[1], O_RDONLY);
    uint64_t* buffer = static_cast<uint64_t*>(mmap(NULL, fileInfo.st_size, PROT_READ, MAP_SHARED, fd, 0));

    bool sorted = true;
    uint64_t lastElem = buffer[0];
    for (uint64_t i=1; i < numElements; i++) {
        if (buffer[i] <= lastElem) {
            sorted = false;
            break;
        }
        lastElem = buffer[i];
    }
    munmap(buffer, fileInfo.st_size);
    close (fd);

    if (sorted) {
        cout << "File is sorted" << endl;
    } else {
        cout << "File is NOT sorted" << endl;
    }
    return 0;
}
