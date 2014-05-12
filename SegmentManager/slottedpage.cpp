#include "slottedpage.h"




SlottedPage::SlottedPage(void *data, uint32_t size)
    : data{reinterpret_cast<char*>(data)},
      size{size},
      firstSlot{reinterpret_cast<Slot*>(header + 1)} {
}


Slot SlottedPage::lookup(uint64_t slotId) const {
    return *(firstSlot + slotId);
}

inline Slot* SlottedPage::getSlot(uint64_t slotId){
    return firstSlot + slotId;
}

Record SlottedPage::readRecord(const Slot& slot) {
    char* recordPos = static_cast<char*>(data) + slot.getOffset();
    return Record(slot.getLength(), recordPos);
}

bool SlottedPage::removeRecord(uint64_t slotId)
{
    Slot* slot = getSlot(slotId);
    header->freeSpace -= slot->getLength();
    if(slotId < header->firstFreeSlot){
        header->firstFreeSlot = slotId;
    }

    slot->setFree();
}

bool SlottedPage::spaceAvailableFor(const Record& record)
{
    unsigned rlength = record.getLen();
    uint64_t freeAfterDataStart = header->dataStart - sizeof(Header) - (header->slotCount * sizeof(Slot));
    return rlength + sizeof(Slot) <= freeAfterDataStart;
}

uint32_t SlottedPage::insertRecord(const Record& record)
{
    //create new slot
    Slot* slot = getSlot(header->slotCount);
    slot->setLength(record.getLen());
    slot->setOffset(header->dataStart - record.getLen());
    header->slotCount++;

    //put data
    //TODO: Sascha, please review and comment: Is this a good method to put move the data?
    memcpy(data + slot->getOffset(), record.getData(), record.getLen());
    return header->slotCount - 1;
}

Header &SlottedPage::getHeader()
{
    return *header;
}


