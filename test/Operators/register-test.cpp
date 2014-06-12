
#include "gtest.h"

#include "register.h"

//using namespace std;

TEST(Operators, IntegerRegisterOK) {
    Register r;
    r.setInteger(0);
    ASSERT_EQ(r.getInteger(), 0);
    r.setInteger(42);
    ASSERT_EQ(r.getInteger(), 42);
    r.setInteger(-45768);
    ASSERT_EQ(r.getInteger(), -45768);
}

TEST(Operators, IntegerRegisterWrongType) {
    Register r;
    ASSERT_ANY_THROW(r.getInteger());
}

TEST(Operators, DoubleRegisterOK) {
    Register r;
    r.setDouble(0);
    ASSERT_EQ(r.getDouble(), 0);
    r.setDouble(1.0);
    ASSERT_EQ(r.getDouble(), 1.0);
    r.setDouble(-10e200);
    ASSERT_EQ(r.getDouble(), -10e200);
}

TEST(Operators, DoubleRegisterWrongType) {
    Register r;
    ASSERT_ANY_THROW(r.getDouble());
}

TEST(Operators, StringRegisterOK) {
    Register r;
    r.setString("");
    ASSERT_EQ(r.getString(), "");
    r.setString("Hallo Welt!");
    ASSERT_EQ(r.getString(), "Hallo Welt!");
}

TEST(Operators, StringRegisterWrongType) {
    Register r;
    ASSERT_ANY_THROW(r.getString());
}

TEST(Operators, RegisterCompareEqual) {
    Register r1;
    Register r2;
    ASSERT_NE(r1, r2);
    r1.setInteger(7);
    r2.setInteger(7);
    ASSERT_EQ(r1, r2);
    r2.setInteger(23);
    ASSERT_NE(r1, r2);
    r1.setDouble(23);
    ASSERT_NE(r1, r2);
    r2.setDouble(23);
    ASSERT_EQ(r1, r2);
    r1.setDouble(50);
    ASSERT_NE(r1, r2);
    r2.setString("50");
    ASSERT_NE(r1, r2);
    r1.setString("Hallo");
    ASSERT_NE(r1, r2);
    r2.setString("Hallo");
    ASSERT_EQ(r1, r2);
    r2.setString("Welt");
    ASSERT_NE(r1, r2);
}
