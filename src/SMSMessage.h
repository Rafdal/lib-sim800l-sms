/**
 * @file SMSMessage.h
 * @author Rafael Dalzotto (rdalzotto@itba.edu.ar)
 * @brief SMS Message data structure
 * @date 2022-07-04
 * 
 * @copyright Copyright (c) 2022
 */

#ifndef SMSMESSAGE_H
#define SMSMESSAGE_H

#define SMS_MESSAGE_MAX_LEN     64
#define SMS_PHONE_NUMBER_SIZE   15  // with '+' and NULL terminator
#define SMS_WORD_SIZE_MAX       16

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

    /**
     * @brief print date to c-string in compact format
     * @param buf 
     */
    void formatTo(char* buf)
    {
        const char* format = "%02u-%u-%02u %02u:%02u:%02u";
        sprintf(buf, format, day, month, year, hour, min, sec); 
    }
};

class SMSMessage
{
public:

    int size = 0;
    char message[SMS_MESSAGE_MAX_LEN];
    char phone[SMS_PHONE_NUMBER_SIZE];
    DateTime date;

    /**
     * @brief Print SMS content
     * 
     */
    void print();

    /**
     * @brief Number of words that appear in the message
     * 
     * @return int = word count
     */
    int countWords();

    /**
     * @brief Get the word that appears in the N possition 
     * 
     * @param n word count to look for
     * @param word external buffer initialized with enough size 
     * @retval true = found OK
     * @retval false = ERROR
     */
    bool getNWord(int n, char* word);

    /**
     * @brief Compare word that appears in the N possition 
     * 
     * @param n word count to look for
     * @param wordToCompare c-string to match
     * @retval true = match OK
     * @retval false = word does not match
     */
    bool compareWordAt(int n, char* wordToCompare);
    /**
     * @brief Compare word that appears in the N possition 
     * 
     * @param n word count to look for
     * @param wordToCompare c-string to match
     * @retval true = match OK
     * @retval false = word does not match
     */
    bool compareWordAt(int n, const char* wordToCompare);

    /**
     * @brief Search str inside message. This is a wrapper of strstr func
     * 
     * @param str string to search
     * @return char* pointer to the first occurrence of str inside message or NULL if not found
     */
    char* search(const char* str);

    /**
     * @brief Construct a new SMSMessage object
     * 
     */
    SMSMessage();
    ~SMSMessage() {}

private:

};

inline bool SMSMessage::compareWordAt(int n, const char* wordToCompare)
{
    char *p = const_cast<char *>(wordToCompare);
    return compareWordAt(n, p);
}

inline char* SMSMessage::search(const char* str)
{
    return strstr(message, str);
}

#endif