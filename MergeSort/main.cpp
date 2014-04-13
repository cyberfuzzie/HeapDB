#include <list>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <queue>

using namespace std;

class RunBuffer{
public:
    RunBuffer(int fdInput, uint64_t memSize) :fdInput{fdInput},
        bufferSize{memSize / sizeof(uint64_t)},
        buffer{new uint64_t[bufferSize]},
        bufferCurrentPos{buffer}
    {
        int readBytes = read(fdInput, buffer, bufferSize * sizeof(uint64_t));

        intsInBuffer = readBytes / sizeof(uint64_t);

        bufferStopPos = buffer + intsInBuffer;
    }

    /*~RunBuffer(){
        delete[] buffer;
    }*/

    inline bool operator < (const RunBuffer& other) const {
        return other.value() < *bufferCurrentPos;
    }

    RunBuffer& operator ++(){
        ++bufferCurrentPos;
        //if buffer is completely read, try to read more from file
        if (bufferCurrentPos == bufferStopPos){
            if (intsInBuffer = read(fdInput, buffer, bufferSize * sizeof(uint64_t)) / sizeof(uint64_t)){
                bufferCurrentPos = buffer;
                bufferStopPos = buffer + intsInBuffer;
            }
        }
    }

    bool hasNext() const{
        return bufferCurrentPos != bufferStopPos;
    }

    uint64_t value() const{
        return *bufferCurrentPos;
    }
private:
    int fdInput;
    uint64_t bufferSize;
    uint64_t intsInBuffer;
    uint64_t* buffer;
    uint64_t* bufferCurrentPos;
    uint64_t* bufferStopPos;
};


int createTmpFile(){
    char buffer[L_tmpnam];
    tmpnam (buffer);
    cout << buffer;
    return open(buffer, O_CREAT | O_RDWR);
}

/**
 * @brief merge merges the integers in all inputs into one file.
 * @param fdInputs list of files to use as input. integers in file must be in sorted order.
 * @param memSize maximum amount of bytes to use.
 * @return the handle to the result file
 */
int merge(list<int>* fdInputs, int fdOutfile, uint64_t memSize){
    priority_queue<RunBuffer> inputOrder;


    //determine buffersize
    uint64_t memPerRun = max(sizeof(uint64_t), memSize / (fdInputs->size() + 1));

    //allocate output buffer
    uint64_t* outBuffer = new uint64_t[memPerRun/sizeof(uint64_t)];
    uint64_t* outBufferEnd = outBuffer + (memPerRun/sizeof(uint64_t));
    uint64_t* outBufferStart = outBuffer;

    //prepare run buffers
    for (list<int>::iterator input = fdInputs->begin(); input != fdInputs->end(); ++input){
        inputOrder.push(RunBuffer(*input, memPerRun));
    }

    uint64_t elementsMerged = 0;

    //draw from run buffers in order determined by queue
    while (!inputOrder.empty()){
        RunBuffer current = inputOrder.top();
        inputOrder.pop();
        if (current.hasNext()){
            *outBuffer = current.value();

            //advance run
            ++current;
            //put the advanced run back into queue
            inputOrder.push(current);

            //manage output buffer
            ++outBuffer;
            ++elementsMerged;
            if (outBuffer == outBufferEnd){
                write(fdOutfile, outBufferStart, elementsMerged * sizeof(uint64_t));
                outBuffer = outBufferStart;
                elementsMerged = 0;
            }
        }
    }

    //flush buffer
    write(fdOutfile, outBufferStart, elementsMerged * sizeof(uint64_t));

    delete[] outBuffer;
}

/**
 * @brief partitionToRuns reads 64-bit integers from fdInput and writes
 *              it into sorted runs.
 * @param fdInput handle for input file
 * @param size number of integers to read from file
 * @param memSize maximum amount of bytes to use
 * @return list of handles for temporary files, containing the runs
 */
list<int>* partitionToRuns(int fdInput, uint64_t size, uint64_t memSize){

    std::list<int>* runs = new std::list<int>();
    //determine buffersize
    uint64_t buffersize = memSize / (sizeof(uint64_t));
    //create buffer
    uint64_t* buffer = new uint64_t[buffersize];
    //read from file to buffer
    while (uint64_t readBytes = read(fdInput, buffer, buffersize * sizeof(uint64_t))){
        //sort buffer
        //TODO: is buffer + readBytes correct?
        sort(buffer, buffer + (readBytes/sizeof(uint64_t)));
        //open tmp file
        int tmpFile = createTmpFile();
        //write to run to tmp file
        pwrite(tmpFile,buffer, readBytes, 0);
        //add run file to result list
        runs->push_back(tmpFile);
    }

    //free buffer
    delete[] buffer;
    //return list of file handles
    return runs;
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

void test(){
    //test data
    uint64_t data[] = {5,3,4,8,1};
    //create inputfile and fill it with data
    int fdSource = createTmpFile();
    pwrite(fdSource, data, 5 * sizeof(uint64_t), 0);

    int memSize = sizeof(uint64_t) * 2;

    //make runs from source file
    list<int>* runs = partitionToRuns(fdSource, 5, memSize);

    cout << "\n";

    for (list<int>::iterator runsi = runs->begin(); runsi != runs->end(); ++runsi){
        uint64_t buffer[memSize];
        int bytesRead = pread(*runsi, buffer, memSize, 0);
        for (int i = 0; i < bytesRead / sizeof(uint64_t); ++i){
            cout << ' ' << buffer[i];
        }
        cout << "\n";
    }

    close(fdSource);

    //merge phase
    int fdOut = createTmpFile();

    merge(runs, fdOut, memSize);

    uint64_t buffer[5];

    cout << "\n";

    int bytesRead = pread(fdOut, buffer, 5 * sizeof(uint64_t), 0);
    for (int i = 0; i < bytesRead / sizeof(uint64_t); ++i){
        cout << ' ' << buffer[i];
    }

    close(fdOut);
    //TODO close run file handles

    cout << "\n";

}

int main()
{

    test();
    return 0;
}



