#include "tablescanoperator.h"
#include "register.h"

TableScanOperator::TableScanOperator(schema::Relation& relation, SPSegment& spseg)
    : relation(relation),
      spsegment(spseg) {}

TableScanOperator::~TableScanOperator() {

}

void TableScanOperator::open() {
    initAttributeOffsets();
    recordIterator = spsegment.getRecordIterator();
    endRecordIterator = recordIterator->end();
}

bool TableScanOperator::next() {
    ++(*recordIterator);
    if ((*recordIterator) == (*endRecordIterator)) {
        // load data
        for (int i=0; i<relation.attributes_size(); i++) {
            loadRegister(i, registers[i]);
        }
        return true;
    } else {
        return false;
    }
}

vector<Register*> TableScanOperator::getOutput() const {
    return registers;
}

void TableScanOperator::close() {
    //destruct iterator
    recordIterator.reset();
    delete[] attributeOffsets;
    for (unsigned int i=0; i<registers.size(); i++) {
        delete registers[i];
    }
}

void TableScanOperator::initAttributeOffsets() {
    attributeOffsets = new off_t[relation.attributes_size()];
    int count = 0;
    for (int i=0; i<relation.attributes_size(); i++) {
        const schema::Attribute& attr = relation.attributes(i);
        if (attr.type() == attr.Numeric
                || attr.type() == attr.Decimal
                || attr.type() == attr.Varchar) {
            attributeOffsets[count] = count;
            count++;
        } else {
            // unsupported type
            throw 0;
        }
    }
}

void TableScanOperator::initRegisters() {
    for (int i=0; i<relation.attributes_size(); i++) {
        registers.push_back(new Register());
    }
}

void TableScanOperator::loadRegister(int attrPos, Register* reg) {
    const schema::Attribute& attr = relation.attributes(attrPos);
    off_t offset = attributeOffsets[attrPos];
    const char* data = (**recordIterator).getData();
    if (attr.type() == attr.Numeric) {
        reg->setInteger(*reinterpret_cast<const uint64_t*>(data + offset));
    } else if (attr.type() == attr.Decimal) {
        reg->setDouble(*reinterpret_cast<const double*>(data + offset));
    } else if (attr.type() == attr.Varchar) {
        uint64_t strData = *reinterpret_cast<const uint64_t*>(data + offset);
        uint32_t strLen = strData & 0xffffffff;
        uint32_t strOff = strData >> 32;
        reg->setString(string(data + strOff, strLen));
    } else {
        // unsupported type
        throw 0;
    }
}
