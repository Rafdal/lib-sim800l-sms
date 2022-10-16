/**
 * @file SMSMessage.cpp
 * @author Rafael Dalzotto (rdalzotto@itba.edu.ar)
 * @brief SMS Message data structure
 * @date 2022-07-04
 * 
 * @copyright Copyright (c) 2022
 */

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

// TODO: Optimize this
int SMSMessage::countWords()
{
    int count = 0;
    char lastChar = '\0';

    for (int i = 0; (i < size) && (message[i]); i++)
    {
        char c = message[i];
        if(!isalnum(lastChar) && isalnum(c))
            count++;
        lastChar = c;
    }
    return count;
}

// TODO: Optimize this SHIT
bool SMSMessage::getNWord(int idx, char* word)
{
    int count = -1;
    char lastChar = '\0';
    int wordSize = 0;

    for (int i = 0; (i < size) && (message[i]); i++)
    {
        char c = message[i];
        if(!isalnum(lastChar) && isalnum(c)) // detect word first alphanumeric char
            count++;

        if((count == idx) && isalnum(c) && (wordSize < (SMS_WORD_SIZE_MAX - 1))) // last char = 0
        {
            word[wordSize++] = c;
        }
        else if(wordSize > 0)
        {
            break;
        }

        lastChar = c;
    }

    if(wordSize > 0)
    {
        word[wordSize] = '\0'; // set NULL terminator
        return true;
    }
    return false;
}

// TODO: Optimize this SHIT
bool SMSMessage::compareWordAt(int n, char* wordToCompare)
{
    char word[SMS_WORD_SIZE_MAX];
    getNWord(n, word);
    if(strcmp(word, wordToCompare) == 0)
        return true;
    return false;
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