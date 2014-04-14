
#include "extsort.h"
#include "memsort.h"

#include <iostream>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv) {
    if (argc != 4) {
        cout << "Usage:" << endl;
        cout << argv[0] << " <inputFile> <outputFile> <bufferInMB>" << endl << endl;
        exit (1);
    }

    uint64_t memSize = atoi(argv[3]) * 1024 * 1024;
    struct stat fileInfo;
    stat(argv[1], &fileInfo);
    uint64_t numElements = fileInfo.st_size / sizeof(uint64_t);

    int fdInput = open (argv[1], O_RDONLY);
    int fdOutput = open (argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    externalSort (fdInput, numElements, fdOutput, memSize);
    close (fdInput);
    close (fdOutput);
    return 0;
}

