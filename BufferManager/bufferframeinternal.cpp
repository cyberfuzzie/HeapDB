#include "bufferframeinternal.h"

#define PAGESIZE 4096

// standard library
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <mutex>
#include <unordered_map>

using namespace std;

// system calls
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

unordered_map<pthread_t,pthread_rwlock_t*> lockMap;
mutex lockMapMutex;
mutex outMutex;

BufferFrameInternal::BufferFrameInternal()
    : BufferFrame(),
      pageId(0),
      writePossible(false),
      lock(PTHREAD_RWLOCK_INITIALIZER) {
    this->wrlock();
}

void BufferFrameInternal::rdlock() {
    pthread_t threadId = pthread_self();
//    {
//        unique_lock<mutex> outLock(outMutex);
//        cout << "Thread " << threadId << " read-locking BufferFrame " << this << endl;
//    }
    {
        unique_lock<mutex> mutexLock(lockMapMutex);
        unordered_map<pthread_t,pthread_rwlock_t*>::iterator savedLock = lockMap.find(threadId);
        assert(savedLock == lockMap.end());
    }
    int result = pthread_rwlock_rdlock(&lock);
    assert(result == 0);
    {
        unique_lock<mutex> mutexLock(lockMapMutex);
        pthread_rwlock_t* savedLock = &lock;
        assert(savedLock != nullptr);
        lockMap[threadId] = savedLock;
    }
//    cout << "read-lock for " << this << endl;
    this->writePossible = false;
}

/*bool BufferFrameInternal::trywrlock() {
    bool wrlocked = (pthread_rwlock_trywrlock(&lock) == 0);
    if (wrlocked) {
        cout << "write-lock for " << this << endl;
        this->writePossible = true;
    }
    return wrlocked;
}*/

void BufferFrameInternal::wrlock() {
    pthread_t threadId = pthread_self();
//    {
//        unique_lock<mutex> outLock(outMutex);
//        cout << "Thread " << threadId << " write-locking BufferFrame " << this << endl;
//    }
    {
        unique_lock<mutex> mutexLock(lockMapMutex);
        unordered_map<pthread_t,pthread_rwlock_t*>::iterator savedLock = lockMap.find(threadId);
        assert(savedLock == lockMap.end());
    }
    int result = pthread_rwlock_wrlock(&lock);
    assert(result == 0);
    {
        unique_lock<mutex> mutexLock(lockMapMutex);
        pthread_rwlock_t* savedLock = &lock;
        assert(savedLock != nullptr);
        lockMap[threadId] = savedLock;
    }
//    cout << "write-lock for " << this << endl;
    this->writePossible = true;
}

void BufferFrameInternal::unlock() {
    pthread_t threadId = pthread_self();
//    {
//        unique_lock<mutex> outLock(outMutex);
//        cout << "Thread " << threadId << " un-locking BufferFrame " << this << endl;
//    }
    {
        unique_lock<mutex> mutexLock(lockMapMutex);
        unordered_map<pthread_t,pthread_rwlock_t*>::iterator mapIt = lockMap.find(threadId);
        assert(mapIt != lockMap.end());
        pthread_rwlock_t* savedLock = mapIt->second;
        assert(savedLock == &lock);
    }
    int result = pthread_rwlock_unlock(&lock);
    assert(result == 0);
    {
        unique_lock<mutex> mutexLock(lockMapMutex);
        lockMap.erase(threadId);
    }
//    cout << "un-lock for " << this << endl;
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
//    {
//        unique_lock<mutex> outLock(outMutex);
//        pthread_t threadId = pthread_self();
//        cout << "Thread " << threadId << " loading page " << this->pageId << " from disk to addr " << static_cast<void*>(this->data.get()) << endl;
//    }
    memset (this->data.get(), 0, PAGESIZE);
    if (fd >= 0) {
        // no error, read
        ssize_t readCount = pread (fd, this->data.get(), PAGESIZE, PAGESIZE * (pageId & 0xff));
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
