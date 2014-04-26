
#include "hashtable.h"

#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    HashTable<uint64_t,uint64_t> hashTable(4);
    hashTable.put(4,5);
    hashTable.put(1,8);
    hashTable.put(3,10);
    hashTable.put(17,18);
    hashTable.put(33,12);
    hashTable.put(3,7);
    hashTable.putIfNotExists(17,20);
    hashTable.putIfNotExists(20,1);
    hashTable.remove(1);
    hashTable.putIfNotExists(1,2);
    hashTable.putIfNotExists(1,11);
    cout << "Key  1, Value " << hashTable.get(1) << endl;
    cout << "Key  4, Value " << hashTable.get(4) << endl;
    cout << "Key  3, Value " << hashTable.get(3) << endl;
    cout << "Key 17, Value " << hashTable.get(17) << endl;
    cout << "Key 20, Value " << hashTable.get(20) << endl;
    cout << "Key 33, Value " << hashTable.get(33) << endl;
}
