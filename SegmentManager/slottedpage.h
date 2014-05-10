#ifndef SLOTTEDPAGE_H
#define SLOTTEDPAGE_H

#include <cstdint>

#include "tid.h"

struct Header {
    uint64_t lsn;
    uint16_t slotCount;
    uint16_t firstFreeSlot;
    uint32_t dataStart;
    //space in bytes that would be available after
    //compactification
    uint16_t freeSpace;
};

class Slot {

    union{
        uint64_t data;
        struct{
            char c[6];
            char s;
            char t;
        };
    };

public:

    bool isRedirect(){
        return t == 0xff;
    }

    void redirectTo(TID otherRecord){
        data = otherRecord;
    }

    void setRedirectFalse(){
        t = 0xff;
    }

    bool isRedirected(){
        return s != 0;
    }

    uint32_t getOffset(){
        return (data >> 32) & 0xffffff;
    }

    void setOffset(uint32_t offset){
        uint64_t o = ((uint64_t)(offset & 0xffffff)) << 24;
        data = (data & (0xffff000000ffffff)) | o;
    }

    uint32_t getLength(){
        return data & 0xfff;
    }

    void setLength(uint32_t length){
        data = (data & 0xffffffffff000000) | (length & 0x00ffffff);
    }


};

class SlottedPage
{
    union{
        Header* header;
        void* data;
    };
    uint32_t size;
    Slot* firstSlot;
public:
    SlottedPage(void* data, uint32_t size);

};

#endif // SLOTTEDPAGE_H
