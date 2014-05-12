#include "slottedpage.h"




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

inline Slot* SlottedPage::getSlot(uint64_t slotId) const {
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

bool SlottedPage::spaceAvailableFor(const Record& record) const {
    unsigned rlength = record.getLen();
    uint64_t freeAfterDataStart = header->dataStart - sizeof(Header) - (header->slotCount * sizeof(Slot));
    return rlength + sizeof(Slot) <= freeAfterDataStart;
}

uint32_t SlottedPage::insertRecord(const Record& record) {
    //create new slot
    uint32_t nextFreeSlot = header->firstFreeSlot;
    Slot* slot = getSlot(nextFreeSlot);
    while (++nextFreeSlot < header->slotCount) {
        if (slot[nextFreeSlot].isFree()) {
            break;
        }
    }
    header->firstFreeSlot = nextFreeSlot;
    slot->setLength(record.getLen());

    header->dataStart -= record.getLen();
    header->freeSpace -= record.getLen();
    slot->setOffset(header->dataStart);
    header->slotCount++;

    //put data
    memcpy(data + slot->getOffset(), record.getData(), record.getLen());
    return header->slotCount - 1;
}

Header &SlottedPage::getHeader() const {
    return *header;
}


