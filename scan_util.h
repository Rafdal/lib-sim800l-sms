#ifndef _SCAN_UTIL_H_
#define _SCAN_UTIL_H_

#include <Arduino.h>

#define SCAN_UTIL_MAX_SUBSTR_SIZE 128

#warning NOT TESTED

class ScanUtil
{
private:
    char* str=NULL;     // ptr to c-str to be analyzed (must be null-terminated)
    unsigned int pos;   // pos to current char
    unsigned int size;  
    int err;            // error counter

public:
    ScanUtil(char* str, unsigned int size)
    {
        this->str = str; 
        this->size = size;
        this->pos = 0;
        this->err = 0;
    }
    ScanUtil(char* str)
    {
        this->str = str; 
        this->size = 0; 
        this->pos = 0;
        this->err = 0;
    }

    // seek string and move to the position after
    // cstr is a null-terminated string
    // return 1 == found | 0 == not found
    int seek(char* cstr);

    // skip n bytes
    int skip(unsigned int n);

    // skip to char
    int skip(char c);

    // save substring until character delimiter is found (or NULL terminator)
    // substr should be initialized with enough size and null-terminated
    void substring_until(char* substr, char delimiter);

    // get current number
    void get_uint8_t(uint8_t* out);
    void get_unsigned_long(unsigned long* out);
    void get_int(int* out);

    int error(void);  // returns >0 in case of error

    ~ScanUtil() {}
};

#endif