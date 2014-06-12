#ifndef REGISTER_H
#define REGISTER_H

#include <string>
#include <cstdint>

using namespace std;

class Register
{
    public:
        enum DataType {
            Invalid,
            Integer,
            Double,
            String
        };

        Register();

        bool operator==(const Register& other) const;
        bool operator!=(const Register& other) const;
        // TODO: other operators

        uint32_t getHash();

        uint64_t getInteger();
        void setInteger(uint64_t value);

        double getDouble();
        void setDouble(double value);

        const string& getString();
        void setString(const string& value);

    private:
        union {
                uint64_t intData;
                double doubleData;
                string* stringData;
        };

        DataType dataType;

        void cleanupDynamicData();
};

#endif // REGISTER_H
