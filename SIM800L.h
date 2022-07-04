/**
 * @file SIM800L.h
 * @author Rafael Dalzotto (rdalzotto@itba.edu.ar)
 * @brief Main library header file
 * @date 2022-07-04
 * 
 * @copyright Copyright (c) 2022
 */

#ifndef _SIM800L_H_
#define _SIM800L_H_

#include <Arduino.h>

#ifndef SoftwareSerial_h
#include <SoftwareSerial.h>
#endif

#include "ScanUtil.h"       // c-string scan utilities
#include "SMSMessage.h"     // SMS data structure

#define SIM800L_COMM_BUF_SIZE 32        // command buffer size
#define SIM800L_READ_BUF_SIZE 256       // read buffer size
#define SIM800L_READ_CHAR_TIMEOUT 50    // single char read timeout (ms)
#define SIM800L_BAUDRATE 9600           // Software Serial baudrate with module

#define DEBUG_SIM800L           // uncomment / comment to set debug on / off
#define PRINT_BUFFER_SIM800L    // same thing here
#define PRINT_SMS_SIM800L

#ifdef DEBUG_SIM800L 
#define DEBUG_PRINT(a) Serial.println(a);
#else
#define DEBUG_PRINT(a)
#endif

typedef void (*VoidCallbackVoid)(void);
typedef void (*SMSCallback)(SMSMessage&);

class SIM800L
{
public:
    SIM800L(){}
    ~SIM800L(){}

    /**
     * @brief Init SIM800L module and setup
     * 
     * @param simModule pointer to SoftwareSerial instance of SIM module
     * @param rstFunc (optional) function callback to be executed after SIM800L software reset
     */
    void begin(SoftwareSerial* simModule, VoidCallbackVoid rstFunc = NULL);

    /**
     * @brief Set SMS Callback
     * 
     * @param callback pointer to void function that receives SMSMessage object
     */
    void onMessage(SMSCallback callback);

    /**
     * @brief This does what it says
     * @param msg SMSMessage Object
     */
    void sendMessage(SMSMessage& msg);

    /**
     * @brief Poll for new data and SMS (Non-blocking), should be called in a real time loop
     */
    void run();

    /**
     * @brief Reset module and restart (also calls rstFunc if set)
     */
    void reset();

    /**
     * @brief Return last connection status received
     * 
     * @retval true = connected
     * @retval false = disconnected
     */
    bool connected();

    /**
     * @brief Ask the module for the current network status (non-blocking)
     */
    void checkConnection();

    /**
     * @brief Print buffer content and bytes
     */
    void printBuffer();

private:
    unsigned long timeout = 12000L;  // Default timeout (12000ms)

    SoftwareSerial *sim_module = NULL;

    VoidCallbackVoid resetCallback = NULL;
    SMSCallback smsCallback = NULL;

    char buffer[SIM800L_READ_BUF_SIZE];
    unsigned int bufferSize = 0;

    uint8_t netStatus = 0;

    // Print Flash-stored string and wait for OK response. This is printed with CRLF
    void printAndWaitOK(const __FlashStringHelper * msg);

    // read n bytes from module or until terminator has been found
    char* read_module_bytes(size_t nbytes, char terminator = '\0');

    // read incoming bytes until timeout and store them in buffer
    void readToBuffer();

    void parseIncomingSMS();
};

inline void SIM800L::onMessage(SMSCallback c)
{
    smsCallback = c;
}

inline bool SIM800L::connected()
{
    return (netStatus == 1);
}

inline void SIM800L::checkConnection()
{
    if(sim_module != NULL)
        sim_module->print(F("AT+CREG?\r\n"));
}

#endif