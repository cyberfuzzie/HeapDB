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
        Register(const Register* other);

        bool operator==(const Register& other) const;
        bool operator!=(const Register& other) const;
        // TODO: other operators

        uint32_t getHash() const;

        DataType getType() const;

        int64_t getInteger() const;
        void setInteger(int64_t value);

        double getDouble() const;
        void setDouble(double value);

        const string& getString() const;
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
