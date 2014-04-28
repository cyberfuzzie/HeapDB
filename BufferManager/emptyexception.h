#ifndef EMPTYEXCEPTION_H
#define EMPTYEXCEPTION_H

#include <exception>

class EmptyException : public std::exception
{
public:
    EmptyException();
};

#endif // EMPTYEXCEPTION_H
