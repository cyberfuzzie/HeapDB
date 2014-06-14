#include "register.h"

Register::Register()
    : intData(0),
      dataType(Invalid) {}

Register::Register(const Register *other)
    : intData(0),
      dataType(other->dataType)
{
    if (dataType == Integer) {
        intData = other->intData;
    } else if (dataType == Double) {
        doubleData = other->doubleData;
    } else if (dataType == String) {
        stringData = other->stringData;
    }
}

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

uint32_t Register::getHash() const {
    if (dataType == Integer || dataType == Double)
        return intData & ((1l << 32) - 1);

    // TODO: really bad hash function
    if (dataType == String)
        return 0;

    return 0;
}

Register::DataType Register::getType() const {
    return dataType;
}

int64_t Register::getInteger() const {
    if (dataType != Integer)
        throw 0;

    return intData;
}

void Register::setInteger(int64_t value) {
    cleanupDynamicData();

    dataType = Integer;
    intData = value;
}

double Register::getDouble() const {
    if (dataType != Double)
        throw 0;

    return doubleData;
}

void Register::setDouble(double value) {
    cleanupDynamicData();

    dataType = Double;
    doubleData = value;
}

const string &Register::getString() const {
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
