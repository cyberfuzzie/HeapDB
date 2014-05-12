#ifndef SLOTTEDPAGE_H
#define SLOTTEDPAGE_H

#include <cstdint>

#include "record.h"
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
            unsigned char c[6];
            unsigned char s;
            unsigned char t;
        };
    };

public:
    // Move-constructor
    Slot(Slot&& other) : data(other.data) {
        other.data = 0;
    }
    // Move-assignment
    Slot& operator=(Slot&& other) {
        data = other.data;
        other.data = 0;
        return *this;
    }

    bool isRedirect() const {
        return t == 0xff;
    }

    void redirectTo(TID otherRecord) {
        data = otherRecord;
    }

    TID getRedirect() const {
        return data;
    }

    void setRedirectFalse() {
        t = 0xff;
    }

    bool isRedirected() const {
        return s != 0;
    }

    uint32_t getOffset() const {
        return (data >> 32) & 0xffffff;
    }

    void setOffset(uint32_t offset) {
        uint64_t o = ((uint64_t)(offset & 0xffffff)) << 24;
        data = (data & (0xffff000000ffffff)) | o;
    }

    uint32_t getLength() const {
        return data & 0xffffff;
    }

    void setLength(uint32_t length) {
        data = (data & 0xffffffffff000000) | (length & 0x00ffffff);
    }


};

class SlottedPage {
    public:
        SlottedPage(void* data, uint32_t size);
        Slot lookup(uint64_t slotId) const;
        Record readRecord(const Slot& slot);
    private:
        union {
            Header* header;
            void* data;
        };
        uint32_t size;
        Slot* firstSlot;
};

#endif // SLOTTEDPAGE_H
