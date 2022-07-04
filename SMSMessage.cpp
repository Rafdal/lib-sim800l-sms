#include "SMSMessage.h"

// Testing definitions, just ignore this
#ifndef LOCAL_CPP_TEST
#define PRINT(s)    Serial.print(s);
#else
#include <cstring>
#include <cstdint>
#include <iostream>
#define PRINT(s)    std::cout << (s);
#endif

SMSMessage::SMSMessage()
{
    memset(phone, 0, SMS_PHONE_NUMBER_SIZE);
    memset(message, 0, SMS_MESSAGE_MAX_LEN);
}

void SMSMessage::print()
{
    PRINT("@ SMS \n")

    PRINT("- from: ")
    PRINT(phone)

    PRINT("\n- date: ")
    char buf[20];
    date.formatTo(buf);
    PRINT(buf);

    PRINT("\n- msg[")
    PRINT(size)
    PRINT("] = \"")
    PRINT(message)
    PRINT("\"\n")
}