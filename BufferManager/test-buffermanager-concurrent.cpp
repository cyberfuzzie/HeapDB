
#include "buffermanager.h"

#include <mutex>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>

using namespace std;

BufferManager* bm;
unsigned pagesOnDisk;
unsigned pagesInRAM;
unsigned threadCount;
unsigned* threadSeed;
volatile bool stop=false;
mutex m;

unsigned randomPage(unsigned threadNum) {
   // pseudo-gaussian, causes skewed access pattern
   unsigned page=0;
   for (unsigned  i=0; i<20; i++)
      page+=rand_r(&threadSeed[threadNum])%pagesOnDisk;
   return page/20;
}

static void* scan(void *arg) {
   // scan all pages and check if the counters are not decreasing
   unsigned counters[pagesOnDisk];
   for (unsigned i=0; i<pagesOnDisk; i++)
      counters[i]=0;

   while (!stop) {
      unsigned start = random()%(pagesOnDisk-10);
      for (unsigned page=start; page<start+10; page++) {
//         cout << "Fixing page " << page << " for scanning" << endl;
         BufferFrame& bf = bm->fixPage(page, false);
//         cout << "Fixed page " << page << " for scanning" << endl;
         unsigned newcount = reinterpret_cast<unsigned*>(bf.getData())[0];
//         {
//             unique_lock<mutex> lock(m);
//             cout << "Testing page " << page << " in frame " << &bf << " data at " << bf.getData() << " with old value " << counters[page] << " and new value " << newcount << endl;
//         }
         assert(counters[page]<=newcount);
         counters[page]=newcount;
//         cout << "Unfixing page " << page << " from scanning" << endl;
         bm->unfixPage(bf, false);
//         cout << "Unfixed page " << page << " from scanning" << endl;
      }
   }

   return NULL;
}

static void* readWrite(void *arg) {
   // read or write random pages
   uintptr_t threadNum = reinterpret_cast<uintptr_t>(arg);

   uintptr_t count = 0;
   for (unsigned i=0; i<100000/threadCount; i++) {
      bool isWrite = rand_r(&threadSeed[threadNum])%128<10;
      uint64_t page = randomPage(threadNum);
//      cout << "Fixing page " << page << " for writing " << isWrite << endl;
      BufferFrame& bf = bm->fixPage(page, isWrite);
//      cout << "Fixed page " << page << " for writing " << isWrite << endl;

      if (isWrite) {
         count++;
         reinterpret_cast<unsigned*>(bf.getData())[0]++;
//         {
//             unique_lock<mutex> lock(m);
//             cout << "Writing page " << page << " in frame " << &bf << " data at " << bf.getData() << " with value " << reinterpret_cast<unsigned*>(bf.getData())[0] << endl;
//         }
      }
//      cout << "Unfixing page " << page << " from writing " << isWrite << endl;
      bm->unfixPage(bf, isWrite);
//      cout << "Unfixed page " << page << " from writing " << isWrite << endl;
   }

   return reinterpret_cast<void*>(count);
}

int main(int argc, char** argv) {
   if (argc==4) {
      pagesOnDisk = atoi(argv[1]);
      pagesInRAM = atoi(argv[2]);
      threadCount = atoi(argv[3]);
   } else {
      cerr << "usage: " << argv[0] << " <pagesOnDisk> <pagesInRAM> <threads>" << endl;
      exit(1);
   }

   threadSeed = new unsigned[threadCount];
   for (unsigned i=0; i<threadCount; i++)
      threadSeed[i] = i*97134;

   bm = new BufferManager(pagesInRAM);

   pthread_t threads[threadCount];
   pthread_attr_t pattr;
   pthread_attr_init(&pattr);

   cout << "initializing pages " << endl;

   // set all counters to 0
   for (unsigned i=0; i<pagesOnDisk; i++) {
      BufferFrame& bf = bm->fixPage(i, true);
      reinterpret_cast<unsigned*>(bf.getData())[0]=0;
      bm->unfixPage(bf, true);
   }

   cout << "starting test " << endl;

   // start scan thread
   pthread_t scanThread;
   pthread_create(&scanThread, &pattr, scan, NULL);

   // start read/write threads
   for (unsigned i=0; i<threadCount; i++)
      pthread_create(&threads[i], &pattr, readWrite, reinterpret_cast<void*>(i));

   // wait for read/write threads
   unsigned totalCount = 0;
   for (unsigned i=0; i<threadCount; i++) {
      void *ret;
      pthread_join(threads[i], &ret);
      totalCount+=reinterpret_cast<uintptr_t>(ret);
   }

   // wait for scan thread
   stop=true;
   pthread_join(scanThread, NULL);

   // restart buffer manager
   delete bm;
   bm = new BufferManager(pagesInRAM);
   
   cout << "checking result " << endl;

   // check counter
   unsigned totalCountOnDisk = 0;
   for (unsigned i=0; i<pagesOnDisk; i++) {
      BufferFrame& bf = bm->fixPage(i,false);
      totalCountOnDisk+=reinterpret_cast<unsigned*>(bf.getData())[0];
      bm->unfixPage(bf, false);
   }

   // cleanup
   delete[] threadSeed;

   // result output
   if (totalCount==totalCountOnDisk) {
      cout << "test successful" << endl;
      delete bm;
      return 0;
   } else {
      cerr << "error: expected " << totalCount << " but got " << totalCountOnDisk << endl;
      delete bm;
      return 1;
   }
}
