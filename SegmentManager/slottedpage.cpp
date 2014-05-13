#include "slottedpage.h"

#include "spsegment.h"

#include <cassert>


SlottedPage::SlottedPage(void *data, uint32_t size)
    : data{reinterpret_cast<char*>(data)},
      size{size},
      firstSlot{reinterpret_cast<Slot*>(header + 1)} {
}

void SlottedPage::initialize() {
    header->lsn = 0;
    header->slotCount = 0;
    header->firstFreeSlot = 0;
    header->dataStart = size;
    header->freeSpace = size - sizeof(Header);
}

Slot SlottedPage::lookup(uint64_t slotId) const {
    return *(firstSlot + slotId);
}

void SlottedPage::redirect(uint64_t slotId, TID tid)
{
    Slot* slot = getSlot(slotId);
    if (!slot->isRedirect()){
        header->freeSpace += slot->getLength();
    }
    slot->redirectTo(tid);
}

inline Slot* SlottedPage::getSlot(uint64_t slotId) const {
    assert(slotId < header->slotCount);
    return firstSlot + slotId;
}

Record SlottedPage::readRecord(const Slot& slot) const {
    char* recordPos = static_cast<char*>(data) + slot.getOffset();
    return Record(slot.getLength(), recordPos);
}

bool SlottedPage::removeRecord(uint64_t slotId) {
    Slot* slot = getSlot(slotId);
    header->freeSpace -= slot->getLength();
    if(slotId < header->firstFreeSlot){
        header->firstFreeSlot = slotId;
    }
    slot->setFree();
    return true;
}

bool SlottedPage::spaceAvailableForInsert(const Record& record) const {
    unsigned rlength = record.getLen();
    uint64_t freeAfterDataStart = header->dataStart - sizeof(Header) - (header->slotCount * sizeof(Slot));
    return rlength + sizeof(Slot) <= freeAfterDataStart;
}

bool SlottedPage::spaceAvailableForUpdate(const uint64_t slotId, const Record& record) const{
    Slot* slot = getSlot(slotId);
    if (record.getLen() <= slot->getLength()){
        return true;
    }else{
        unsigned rlength = record.getLen();
        uint64_t freeAfterDataStart = header->dataStart - sizeof(Header) - (header->slotCount * sizeof(Slot));
        return rlength <= freeAfterDataStart;
    }

}

uint32_t SlottedPage::insertRecord(const Record& record) {
    //create new slot
    uint32_t nextFreeSlot = header->firstFreeSlot;
    Slot* slot = getSlot(nextFreeSlot);
    while (++nextFreeSlot < header->slotCount) {
        //REVIEW: changed slot to firstSlot, because nextFreeSlot is an absolute index. Correct?
        if (firstSlot[nextFreeSlot].isFree()) {
            break;
        }
    }
    //increment slot count only if we actually occupy a new slot
    if (header->firstFreeSlot == header->slotCount){
        header->slotCount++;
    }
    header->firstFreeSlot = nextFreeSlot;

    putRecordAtDataStart(slot, header, record);

    return header->slotCount - 1;
}

inline void SlottedPage::putRecordAtDataStart(Slot* slot, Header* header, const Record& record){
    slot->setLength(record.getLen());

    header->dataStart -= record.getLen();
    header->freeSpace -= record.getLen();
    slot->setOffset(header->dataStart);

    //put data
    dataToPage(slot, record);
}

bool SlottedPage::updateRecord(const uint64_t slotId, const Record& record){
    Slot* slot = getSlot(slotId);

    if (slotId > header->slotCount){
        return false;
    }

    if (record.getLen() <= slot->getLength()){
        //Put record to old position in case it fits
        dataToPage(slot, record);
        header->freeSpace = header->freeSpace + record.getLen() - slot->getLength();
        slot->setLength(record.getLen());
        return true;
    }else{
        //Put record into free space on page and redirect on same page
        header->freeSpace = header->freeSpace + record.getLen();
        putRecordAtDataStart(slot, header, record);
        return true;
    }
}

void SlottedPage::dataToPage(const Slot* slot,const Record& record){
    memcpy(data + slot->getOffset(), record.getData(), record.getLen());
}

Header &SlottedPage::getHeader() const {
    return *header;
}


