#include "slottedpage.h"




SlottedPage::SlottedPage(void *data, uint32_t size)
    : data{data},
      size{size},
      firstSlot{reinterpret_cast<Slot*>(header + 1)}
{



}
