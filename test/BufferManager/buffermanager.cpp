
#include "bufferframe.h"
#include "buffermanager.h"

#include <iostream>

#include "gtest.h"

using namespace std;

TEST(BufferManager, simpleReadWrite) {

    //TODO: transform this into a real test
    BufferManager mgr(16);
    BufferFrame& frm1 = mgr.fixPage(1, false);
    cout << "Got BufferFrame at " << &frm1 << endl;
    cout << "BufferFrame has pointer " << frm1.getData() << endl;
    BufferFrame& frm2 = mgr.fixPage(1, false);
    cout << "Got BufferFrame at " << &frm2 << endl;
    cout << "BufferFrame has pointer " << frm2.getData() << endl;
    mgr.unfixPage(frm1, false);
    mgr.unfixPage(frm2, false);
    BufferFrame& frm3 = mgr.fixPage(1, true);
    cout << "Got BufferFrame at " << &frm3 << endl;
    cout << "BufferFrame has pointer " << frm3.getData() << endl;
    mgr.unfixPage(frm3, false);
    BufferFrame& frm4 = mgr.fixPage(1, true);
    cout << "Got BufferFrame at " << &frm4 << endl;
    cout << "BufferFrame has pointer " << frm4.getData() << endl;
    mgr.unfixPage(frm4, true);
}
