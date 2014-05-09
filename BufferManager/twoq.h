#ifndef TWOQ_H
#define TWOQ_H


#include "concurrentlist_simple.h"
#include "emptyexception.h"
#include "bufferframe.h"

#include <cstdint>
#include <mutex>

class TwoQ
{
public:
    TwoQ(uint64_t bufferSize);

    /**
     * @brief promote inform TwoQ that this frame has just been used.
     * @param bufferFrame
     */
    void promote(BufferFrame* frame);

    /**
     * @brief findElementToUnfixFor
     * @return the BufferFrame that should be overriden next. Subsequent calls do not
     *          get the same BufferFrame, unless the BufferFrame was promoted again.
     */
    BufferFrame* getFrameForReclaim();

private:
    uint64_t Kin;
    uint64_t Kout;

    ConcurrentListSimple<BufferFrame*> Am;
    ConcurrentListSimple<BufferFrame*> A1in;
    ConcurrentListSimple<uint64_t> A1out;

    mutex m;


};

#endif // TWOQ_H
