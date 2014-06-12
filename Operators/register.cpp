#include "register.h"

Register::Register()
    : intData(0),
      dataType(Invalid)
{}

bool Register::operator==(const Register& other) const {
    if (dataType != other.dataType)
        return false;

    if (dataType == Integer)
        return intData == other.intData;

    if (dataType == Double)
        return doubleData == other.doubleData;

    if (dataType == String)
        return *stringData == *(other.stringData);

    return false;
}

bool Register::operator!=(const Register& other) const {
    return ! (*this == other);
}

uint32_t Register::getHash() {
    if (dataType == Integer || dataType == Double)
        return intData & ((1 << 32) - 1);

    // TODO: really bad hash function
    if (dataType == String)
        return 0;
}

uint64_t Register::getInteger() {
    if (dataType != Integer)
        throw 0;

    return intData;
}

void Register::setInteger(uint64_t value) {
    cleanupDynamicData();

    dataType = Integer;
    intData = value;
}

double Register::getDouble() {
    if (dataType != Double)
        throw 0;

    return doubleData;
}

void Register::setDouble(double value) {
    cleanupDynamicData();

    dataType = Double;
    doubleData = value;
}

const string &Register::getString() {
    if (dataType != String)
        throw 0;

    return *stringData;
}

void Register::setString(const string &value) {
    cleanupDynamicData();

    dataType = String;
    stringData = new string(value);
}

void Register::cleanupDynamicData() {
    if (dataType == String)
        delete stringData;
}
