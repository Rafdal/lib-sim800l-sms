#ifndef SMSMESSAGE_H
#define SMSMESSAGE_H

#define SMS_MESSAGE_MAX_LEN     64
#define SMS_PHONE_NUMBER_SIZE   15  // with '+' and NULL terminator

#ifndef LOCAL_CPP_TEST
#include <Arduino.h>
#else
#include <cstring>
#include <cstdint>
#include <cstdio>
#endif

struct DateTime
{
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    DateTime()
    {
        hour = 0;
        min = 0;
        sec = 0;
        year = 0;
        month = 0;
        day = 0;
    }
    void date(uint8_t y, uint8_t m, uint8_t d)
    {
        year = y;
        month = m;
        day = d;
    }
    void time(uint8_t h, uint8_t m, uint8_t s)
    {
        hour = h;
        min = m;
        sec = s;
    }
    void formatTo(char* buf)
    {
        const char* format = "%02u-%u-%02u %02u:%02u:%02u";
        sprintf(buf, format, day, month, year, hour, min, sec); 
    }
};

class SMSMessage
{
private:
public:

    int size = 0;
    char message[SMS_MESSAGE_MAX_LEN];
    char phone[SMS_PHONE_NUMBER_SIZE];
    DateTime date;

    void print();

    SMSMessage();
    ~SMSMessage() {}
};

#endif