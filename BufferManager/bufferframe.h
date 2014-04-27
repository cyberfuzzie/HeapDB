#ifndef BUFFERFRAME_H
#define BUFFERFRAME_H

#include <memory>

using namespace std;

class BufferFrame
{
    public:
        BufferFrame();
        virtual ~BufferFrame() = 0;
        void* getData();
    protected:
        unique_ptr<char[]> data;
};

#endif // BUFFERFRAME_H
