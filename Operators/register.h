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

        DataType getType();

        int64_t getInteger();
        void setInteger(int64_t value);

        double getDouble();
        void setDouble(double value);

        const string& getString();
        void setString(const string& value);

    private:
        union {
                int64_t intData;
                double doubleData;
                string* stringData;
        };

        DataType dataType;

        void cleanupDynamicData();
};

#endif // REGISTER_H
