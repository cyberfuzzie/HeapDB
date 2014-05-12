#include "slottedpage.h"




SlottedPage::SlottedPage(void *data, uint32_t size)
    : data{data},
      size{size},
      firstSlot{reinterpret_cast<Slot*>(header + 1)} {



}


Slot SlottedPage::lookup(uint64_t slotId) const {

}

Record SlottedPage::readRecord(const Slot& slot) {
    char* recordPos = static_cast<char*>(data) + slot.getOffset();
    return Record(slot.getLength(), recordPos);
}
