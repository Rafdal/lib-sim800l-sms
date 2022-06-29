#include "scan_util.h"
#include <ctype.h>

// * Pre-analysis OK
// seek string and move to the position after
// cstr is a null-terminated string
// return 1 == found | 0 == not found or mismatch
int ScanUtil::seek(char* cstr)
{
    uint8_t i = 0;
    while( (cstr[i] != NULL) && (pos < size) && (str[pos] != NULL) ){

        if(str[pos] == cstr[i]) // found a match
            i++;
        else
            i=0;
        pos++;

        if(cstr[i] == NULL) // reached null-terminator of cstr means we have a full match of the string
            return 1;
    }
    err++;
    return 0;
}

// ! OJO: No tiene en cuenta el terminador nulo
// skip n bytes
// return 1 == OK | 0 == error
int ScanUtil::skip(unsigned int n)
{
    if(pos+n < size){
        pos+=n;
        return 1;
    }
    err++;
    return 0;
}

// * ok
// skip to first occurrence of char c
// return 1 == found | 0 == not found
int ScanUtil::skip(char c)
{
    while(pos < size && str[pos] != c && str[pos] != NULL)
        pos++;
    if(str[pos] == c)
        return 1;
    err++;
    return 0;
}

// * ok
// save substring until character delimiter is found (or NULL terminator)
// substr should be initialized with enough size
void ScanUtil::substring_until(char* substr, char delimiter, size_t maxSize)
{
    uint8_t i=0;
    while((pos < size) && (i < SCAN_UTIL_MAX_SUBSTR_SIZE)){
        if((str[pos] == NULL) || (str[pos] == delimiter) || (maxSize != 0 && i >= maxSize)){
            substr[i] = NULL; // write null terminator
            return;
        }

        substr[i++] = str[pos++];
    }
    err++; // error
}

// TODO
void ScanUtil::_getANum(void){

    char num_buf[16];
    switch (str[pos])
    {
    case '-':
        {
            this->sign = 1;
            pos++;
        }
        break;
    case '+':
        {
            this->sign = 2;
            pos++;
        }
        break;
    default:
        this->sign = 0;
        break;
    }
    
    while(str[pos]){

    }
}

// TODO
// get current number
void ScanUtil::get_uint8_t(uint8_t* out)
{

}

// TODO
void ScanUtil::get_unsigned_long(unsigned long* out)
{
}

// TODO
void ScanUtil::get_int(int* out)
{
}

int ScanUtil::error(void)
{
    return err;
}  // returns >0 in case of error
