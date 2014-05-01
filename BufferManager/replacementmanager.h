#ifndef REPLACEMENTMANAGER_H
#define REPLACEMENTMANAGER_H

#include <condition_variable>
#include <deque>
#include <mutex>

using namespace std;

class ReplacementManager
{
    public:
        ReplacementManager();
        void promoteFrame(uint64_t frameId);
        void removeFrame(uint64_t frameId);
        uint64_t reclaimFrame();
    private:
        mutex frameListMutex;
        condition_variable frameListCond;
        deque<uint64_t> frameList;
};

#endif // REPLACEMENTMANAGER_H
