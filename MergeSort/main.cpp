#include <list>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

int main()
{
    return 0;
}

/**
 * @brief externalSort
 * @param fdInput file handle to read from
 * @param size number of 64bit unsigned integer values stored in file;
 * @param fdOutput file handle to write to
 * @param memSize number of bytes in main memory to use for merge sort
 */
void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize){

}

std::list<int> partitionToRuns(int fdInput, uint64_t size, uint64_t memSize){

    //remaining ints = size
    //determine buffersize
    buffersize = memSize / (sizeof(uint64_t));
    //create buffer
    uint64_t buffer[buffersize];
    //read from file to buffer
    while (readBytes = read(fdInput, buffer, buffersize * sizeof(uint64_t))){
        //sort buffer
        sort(buffer.begin(), buffer.end());
        //open tmp file
        //write to run to tmp file

        //add run file to result list

    }

    //free buffer? TODO
    //return list of file handles
}

