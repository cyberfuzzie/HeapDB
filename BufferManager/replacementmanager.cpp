#include "replacementmanager.h"

#include <mutex>

using namespace std;

ReplacementManager::ReplacementManager() {
}

void ReplacementManager::promoteFrame(uint64_t frameId) {
    lock_guard<mutex> lock(frameListMutex);
    deque<uint64_t>::iterator it = frameList.begin();
    while (it != frameList.end() && *(it++) != frameId);
    if (it == frameList.end()) {
        frameList.push_back(frameId);
        frameListCond.notify_one();
    }
}

void ReplacementManager::removeFrame(uint64_t frameId) {
    lock_guard<mutex> lock(frameListMutex);
    deque<uint64_t>::iterator it = frameList.begin();
    while (it != frameList.end() && *(it++) != frameId);
    if (it != frameList.end()) {
        frameList.erase(it);
    }
}

uint64_t ReplacementManager::reclaimFrame() {
    unique_lock<mutex> lock(frameListMutex);
    while (frameList.size() == 0) {
        frameListCond.wait(lock);
    }
    uint64_t nextFrame = frameList.front();
    frameList.pop_front();
    return nextFrame;
}
