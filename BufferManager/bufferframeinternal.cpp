#include "bufferframeinternal.h"

#define PAGESIZE 4096

// standard library
#include <cstdio>
#include <cstring>
#include <iostream>

using namespace std;

// system calls
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

BufferFrameInternal::BufferFrameInternal()
    : BufferFrame(),
      pageId(0),
      writePossible(false),
      lock(PTHREAD_RWLOCK_INITIALIZER) {
    this->wrlock();
}

void BufferFrameInternal::rdlock() {
    pthread_rwlock_rdlock(&lock);
    this->writePossible = false;
}

bool BufferFrameInternal::trywrlock() {
    bool wrlocked = (pthread_rwlock_trywrlock(&lock) == 0);
    if (wrlocked) {
        this->writePossible = true;
    }
    return wrlocked;
}

void BufferFrameInternal::wrlock() {
    pthread_rwlock_wrlock(&lock);
    this->writePossible = true;
}

void BufferFrameInternal::unlock() {
    pthread_rwlock_unlock(&lock);
}

uint64_t BufferFrameInternal::getMappedPageId() {
    return this->pageId;
}

void BufferFrameInternal::mapPage(uint64_t pageId) {
    if (!this->writePossible) {
        cout << "Frame is not held exclusively" << endl;
        return;
    }
    this->pageId = pageId;
    char filename[20];
    snprintf (filename, 20, "page%lu", (pageId >> 8));
    int fd = open (filename, O_RDONLY);
    this->data = unique_ptr<char[]>(new char[PAGESIZE]);
    memset (this->data.get(), 0, PAGESIZE);
    if (fd >= 0) {
        // no error, read
        pread (fd, this->data.get(), PAGESIZE, PAGESIZE * (pageId & 0xff));
        close (fd);
    }
}

void BufferFrameInternal::writePage() {
    if (!this->writePossible || this->data.get() == nullptr) {
        cout << "Writing is not possible for this page" << endl;
        return;
    }
//    cout << "Writing page " << this->pageId << " to disk" << endl;
    char filename[20];
    snprintf (filename, 20, "page%lu", (pageId >> 8));
    int fd = open (filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd >= 0) {
        // no error, read
        posix_fallocate(fd, PAGESIZE * (pageId & 0xff), PAGESIZE);
        ssize_t writeCount = pwrite (fd, this->data.get(), PAGESIZE, PAGESIZE * (pageId & 0xff));
        close (fd);
    }
}