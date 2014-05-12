#include "slottedpage.h"




SlottedPage::SlottedPage(void *data, uint32_t size)
    : data{reinterpret_cast<char*>(data)},
      size{size},
      firstSlot{reinterpret_cast<Slot*>(header + 1)} {
}

void SlottedPage::initialize() {

}

Slot SlottedPage::lookup(uint64_t slotId) const {
    return *(firstSlot + slotId);
}

Record SlottedPage::readRecord(const Slot& slot) const {
    char* recordPos = static_cast<char*>(data) + slot.getOffset();
    return Record(slot.getLength(), recordPos);
}

bool SlottedPage::spaceAvailableFor(const Record& record) const {
    unsigned rlength = record.getLen();
    uint64_t freeAfterDataStart = header->dataStart - sizeof(Header) - (header->slotCount * sizeof(Slot));
    return rlength + sizeof(Slot) <= freeAfterDataStart;
}

uint32_t SlottedPage::insertRecord(const Record& record) {
    //create new slot
    Slot* slot = firstSlot + header->slotCount;
    slot->setLength(record.getLen());
    slot->setOffset(header->dataStart - record.getLen());
    header->slotCount++;

    //put data
    memcpy(data + slot->getOffset(), record.getData(), record.getLen());
    return header->slotCount - 1;
}

Header &SlottedPage::getHeader() const {
    return *header;
}


