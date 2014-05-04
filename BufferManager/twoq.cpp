#ifndef TWOQ_C
#define TWOQ_C

#include "twoq.h"


#include <mutex>

TwoQ::TwoQ(uint64_t bufferSize)
    : Kin{bufferSize / 4},
      Kout{bufferSize / 2},
      Am{},
      A1in{},
      A1out{}
{
}

void TwoQ::promote(BufferFrame* frame)
{
    unique_lock<mutex> lck{m};
    if (Am.contains(frame)){
        Am.moveTop(frame);
    }else if (A1out.contains(frame->getMappedPageId())){
        Am.putTop(frame);
        A1out.remove(frame->getMappedPageId());
    }else if (A1in.contains(frame)){
        //do nothing
    }else{
        A1in.putTop(frame);
    }
}

BufferFrame* TwoQ::getFrameForReclaim()
{
    unique_lock<mutex> lck{m};

    //TODO: check if all frames are in use, e.g. by setting counters
    while(true){
        if (A1in.getSize() > Kin){
            BufferFrame* reclaim = A1in.getLast();
            if (reclaim->refCount > 0){
                //if the candidate for reclaim is still in use
                //give him an upgrade to the hotlist.
                A1in.remove(reclaim);
                Am.putTop(reclaim);
                continue;
            }

            //Remove the frame that will be reclaimed from
            //A1 and remember the pageId
            A1in.remove(reclaim);
            A1out.putTop(reclaim->getMappedPageId());

            //housekeeping on remembered page Ids
            if (A1out.getSize() > Kout){
                A1out.removeLast();
            }

            return reclaim;
        }else{
            BufferFrame* reclaim = Am.getLast();
            if (reclaim->refCount > 0){
                Am.moveTop(reclaim);
                continue;
            }

            Am.remove(reclaim);

            return reclaim;
        }
    }
}

#endif
