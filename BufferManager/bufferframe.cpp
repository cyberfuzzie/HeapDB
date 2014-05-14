
#include "bufferframe.h"

#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

// system calls
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

BufferFrame::BufferFrame()
    : refCount(0),
      writePossible(false),
      data(nullptr),
      frameId(0),
      pageId(0),
      lock(PTHREAD_RWLOCK_INITIALIZER) {}

BufferFrame::~BufferFrame() {}

uint64_t BufferFrame::getFrameId() {
     return this->frameId;
}

void BufferFrame::setFrameId(uint64_t frameId) {
    this->frameId = frameId;
}

void* BufferFrame::getData() {
    return this->data.get();
}

void BufferFrame::rdlock() {
    int result = pthread_rwlock_rdlock(&lock);
    assert(result == 0);
    this->writePossible = false;
    assert(this->writePossible == false);
}

void BufferFrame::wrlock() {
    int result = pthread_rwlock_wrlock(&lock);
    assert(result == 0);
    this->writePossible = true;
    assert(this->writePossible == true);
}

bool BufferFrame::trywrlock() {
    int result = pthread_rwlock_trywrlock(&lock) == 0;
    if (result) {
        this->writePossible = true;
        assert(this->writePossible == true);
    }
    return result;
}

void BufferFrame::unlock() {
    this->writePossible = false;
    assert(this->writePossible == false);
    assert(lock.__data.__nr_readers > 0 || lock.__data.__writer != 0);
    int result = pthread_rwlock_unlock(&lock);
    assert(result == 0);
}

uint64_t BufferFrame::getMappedPageId() {
    return this->pageId;
}

void BufferFrame::mapPage(uint64_t pageId) {
    atomic_thread_fence(memory_order_seq_cst);
    if (!this->writePossible) {
        //TODO: throw error
        cout << "Frame is not held exclusively" << endl;
        return;
    }
    this->pageId = pageId;
    char filename[20];
    calculateFilename(pageId, filename, sizeof(filename));
    int fd = open (filename, O_RDONLY);
    struct stat fileInfo;
    fstat(fd, &fileInfo);
    this->data = unique_ptr<char[]>(new char[PAGESIZE]);
    memset (this->data.get(), 0, PAGESIZE);
    if (fd >= 0) {
        // no error, read
        off_t readExpected = fileInfo.st_size - PAGESIZE * extractActualPageId(pageId);
        if (readExpected > PAGESIZE) {
            readExpected = PAGESIZE;
        }
        ssize_t readCount = pread (fd, this->data.get(), PAGESIZE, PAGESIZE * extractActualPageId(pageId));
        assert(readCount == readExpected);
        close (fd);
    }
}

void BufferFrame::writePage() {
    atomic_thread_fence(memory_order_seq_cst);
    if (!this->writePossible || this->data.get() == nullptr) {
        cout << "Writing is not possible for this page" << endl;
        return;
    }
    char filename[20];
    calculateFilename(pageId, filename, sizeof(filename));
    int fd = open (filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        // no error, read
        posix_fallocate(fd, PAGESIZE * extractActualPageId(pageId), PAGESIZE);
        ssize_t writeCount = pwrite (fd, this->data.get(), PAGESIZE, PAGESIZE * extractActualPageId(pageId));
        assert(writeCount == PAGESIZE);
        close (fd);
    }
}

void BufferFrame::calculateFilename(uint64_t pageId, char* buffer, size_t bufLen) {
    snprintf (buffer, bufLen, "segment%07lu", extractSegmentId(pageId));
}

uint64_t BufferFrame::extractActualPageId(uint64_t pageId) {
    return pageId & 0xffffffffff;
}

uint64_t BufferFrame::extractSegmentId(uint64_t pageId) {
    return pageId >> 40;
}
