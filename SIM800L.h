#ifndef _SIM800L_H_
#define _SIM800L_H_

#include <Arduino.h>


// If using software serial
#ifndef SoftwareSerial_h
#include <SoftwareSerial.h>
#endif

#define SIM800L_COM_BUF_SIZE 32
#define SIM800L_READ_BUF_SIZE 256
#define SIM800L_READ_CHAR_TIMEOUT 40    // single char read timeout (ms)

class SIM800L
{
private:
    unsigned long timeout = 12000L;  // Default timeout (ms)

    SoftwareSerial *sim_module = NULL;

    void (*reset)(void) = NULL; // Reset callback

    char buffer[SIM800L_READ_BUF_SIZE];

    void printAndWaitOK(const __FlashStringHelper * msg); // Print null-terminated string and wait for OK response

    char* read_module_bytes(size_t nbytes, char terminator); // read n bytes from module or until terminator has been found
public:
    SIM800L();
    void begin(SoftwareSerial* simModule, void (*rst)(void));

    

    ~SIM800L();
};

SIM800L::SIM800L()
{
}

SIM800L::~SIM800L()
{
}



#endif