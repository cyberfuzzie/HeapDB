#ifndef SLOTTEDPAGE_H
#define SLOTTEDPAGE_H

#include <cstdint>

#include "record.h"
#include "tid.h"

struct Header {
    uint64_t lsn;
    uint16_t slotCount;
    uint16_t firstFreeSlot;
    /**
     * @brief dataStart Number of first occupied byte on the page.
     *  (First here is meant from the beginning of the page,
     *  all data follows after this point.)
     */
    uint32_t dataStart;

    /**
     * @brief freeSpace space in bytes that would
     *  be available after compactification
     */
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

    // Copy-contructor
    Slot(Slot& other): data(other.data){

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
        return (data >> 24) & 0xffffff;
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

    bool isFree() const{
        //offset == 0 and length == 0 ?
        return (data & 0xffffffffffff) == 0;
    }

    void setFree(){
        data = (data & 0x000000000000);
    }


};

class SlottedPage {
    public:
        SlottedPage(void* data, uint32_t size);

        /**
         * @brief initialize a new slotted page
         */
        void initialize();

        /**
         * @brief lookup
         * @param slotId
         * @return the slot for the given slotId
         */
        Slot lookup(uint64_t slotId) const;

        Record readRecord(const Slot& slot) const;

        Slot* getSlot(uint64_t slotId) const;

        bool removeRecord(uint64_t slotId);
        /**
         * @brief spaceAvailableFor
         * @param record
         * @return whether enough space is available for the given record.
         */
        bool spaceAvailableFor(const Record& record) const;

        /**
         * @brief insertRecord Inserts the given record on this page.
         *          Fails silently if there is not enough space! Make sure to check before
         *          calling.
         * @param record
         * @return the slotnumber of the record
         */
        uint32_t insertRecord(const Record& record);

        Header& getHeader() const;
    private:
        union {
            Header* header;
            char* data;
        };
        uint32_t size;
        Slot* firstSlot;
};

#endif // SLOTTEDPAGE_H
