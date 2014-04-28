#include "bufferframe.h"

#include <iostream>

using namespace std;

BufferFrame::BufferFrame()
    : data(nullptr) {}

BufferFrame::~BufferFrame() {}

void* BufferFrame::getData() {
    return this->data.get();
}
