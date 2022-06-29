#include "scan_util.h"
#include <ctype.h>

// * Pre-analysis OK
// seek string and move to the position after
// cstr is a null-terminated string
// return 1 == found | 0 == not found
int ScanUtil::seek(char* cstr)
{
    int found=0;
    uint8_t i=0;
    while(cstr[i] != (char)0 && !found && pos+1 < size && str[pos] != (char)0){
        if(str[pos] == cstr[i]) // found a match
            i++;
        else
            i=0;
        pos++;

        if(cstr[i] == (char)0) // reached null-terminator of cstr means we have a full match of the string
            found = 1;
    }
    if(found == 0)
        err++;
    return found;
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
    while(pos+1 < size && str[pos] != c && str[pos] != (char)0)
        pos++;
    if(str[pos] == c)
        return 1;
    err++;
    return 0;
}

// * ok
// save substring until character delimiter is found (or NULL terminator)
// substr should be initialized with enough size
void ScanUtil::substring_until(char* substr, char delimiter)
{
    uint8_t i=0;
    while(pos < size && i < SCAN_UTIL_MAX_SUBSTR_SIZE){
        if(str[pos] == (char)0 || str[pos] == delimiter){
            substr[i] = (char)0; // write null terminator
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
