/**
 * @brief Pretty shitty but robust c-string scanning tool to avoid using scanf
 * @author Rafael Dalzotto
 */

#ifndef _SCAN_UTIL_H_
#define _SCAN_UTIL_H_

#define SCAN_UTIL_MAX_SUBSTR_SIZE 128

#ifndef LOCAL_CPP_TEST
#include <Arduino.h>
#else
#include <cstring>
#include <cstdint>
#endif

#define NULLCHAR    (char)0

class ScanUtil
{
public:
    ScanUtil(char* str, unsigned int size = 0)
    {
        if(size == 0)
            this->size = strlen(str);
        else
            this->size = size;
        this->str = str; 
    }
    ScanUtil(const char* str, unsigned int size = 0)
    {
        if(size == 0)
            this->size = strlen(str);
        else
            this->size = size;
        this->str = const_cast<char*>(str);
    }

    /**
     * @brief Seek string and move to the position after
     * 
     * @param cstr  C-style string (null-terminated string)
     * @retval 1 = found
     * @retval 0 = not found or mismatch
     */
    int seek(char* cstr);
    /**
     * @brief Seek string and move to the position after
     * 
     * @param cstr  C-style string (null-terminated string)
     * @retval 1 = found
     * @retval 0 = not found or mismatch
     */
    int seek(const char* cstr);

    /**
     * @brief Skip n bytes
     *
     * @param n bytes to skip
     * @retval 1 = found
     * @retval 0 = not found or mismatch
     */
    int skip(unsigned int n);

    /**
     * @brief skip to first occurrence of char c
     * 
     * @param c 
     * @retval 1 = found
     * @retval 0 = not found or mismatch
     */
    int skipTo(char c);

    /**
     * @brief Save normalized substring until character delimiter is found (or NULLCHAR terminator)
     * 
     * @param substr External buffer, should be initialized with enough size
     * @param delimiter delimiter stop character
     * @param maxSize 
     */
    void substring(char* substr, char delimiter, size_t maxSize = 0);

    /**
     * @brief Get the current number
     * 
     * @param out Pointer to number
     */
    void get_uint8_t(uint8_t* out);
    void get_unsigned_long(unsigned long* out);
    void get_int(int* out);

    /**
     * @brief Get error count
     * 
     * @retval 0  == OK
     * @retval >0 == ERROR
     */
    int error(void);

    /**
     * @brief Normalize ASCII char (lowercase without accents)
     * 
     * @param c ASCII char (8 bit)
     * @return Normalized character
     */
    char normalizeChar(char c);

    ~ScanUtil() {}

private:
    char* str = NULL;       // ptr to c-str to be analyzed (must be null-terminated)
    unsigned int size;      // size of str
    unsigned int pos=0;     // pos to current char of str
    int err=0;              // error counter


    unsigned long scanNum;
    int sign;
    /**
     * @brief Get a number
     * 
     * @retval true - success
     * @retval false - error
     */
    bool getANum(void);
};

inline int ScanUtil::error(void)
{
    return err;
}

inline int ScanUtil::seek(const char *cstr)
{
    char *p = const_cast<char *>(cstr);
    return seek(p);
}

#endif