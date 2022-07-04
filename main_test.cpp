/**
 * @file main_test.cpp
 * @author Rafael Dalzotto (rdalzotto@itba.edu.ar)
 * @brief This file is used for testing and development purposes and should be ignored
 * @version 0.1
 * @date 2022-07-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef Arduino_h // This shouldn't be compiled under the Arduino enviroment

#include <iostream>
using namespace std;

#include "ScanUtil.h"
#include "SMSMessage.h"

void printHeader(char* buf);

int main()
{
    cout << "Start main.cpp" << endl;

    // const char *str = "\r\n+CMT: \"+5493487123456\",\"\",\"22/06/28,22:02:24-12\"\r\nBoooenas\r\n\0";
    // const char *str = "AT+CREG?\r\n\r\n+CREG: 032,1\r\nald82s\0";
    const char *str = "AT+TEST\r\n\r\nNUM: 3487541299\0";

    char header[10];
    memset(header, 0, 10);
    strncpy(header, str, 9);

    printHeader(header);

    ScanUtil scan(str, 24);
    if (strstr(header, "+CMT:") != NULL)
    {
        SMSMessage sms;
        scan.seek("+CMT: \"");
        scan.substring(sms.phone, '\"', SMS_PHONE_NUMBER_SIZE);

        if (scan.seek("\"\",\""))
        {
            scan.get_uint8_t(&sms.date.year);
            scan.get_uint8_t(&sms.date.month);
            scan.get_uint8_t(&sms.date.day);
            scan.get_uint8_t(&sms.date.hour);
            scan.get_uint8_t(&sms.date.min);
            scan.get_uint8_t(&sms.date.sec);

            if (scan.seek("\"\r\n"))
            {
                scan.substring(sms.message, '\r', SMS_MESSAGE_MAX_LEN);
                sms.size = strlen(sms.message);

                if(scan.error() == 0)
                {
                    sms.print(); // no errors
                }
            }
        }
    }
    else if(strstr(header, "AT+CREG") != NULL)
    {
        uint8_t netStatus = 0;
        scan.skipTo(',');
        scan.get_uint8_t(&netStatus);
        if(scan.error() == 0)
        {
            cout << "Net status: " << (int)netStatus << endl;
        }
    }
    else if(strstr(str, "\"SM\"") != NULL)
    {

    }
    else if(strstr(str, "TEST") != NULL)
    {
        scan.seek("NUM: ");
        unsigned long num;
        scan.get_unsigned_long(&num);

        if(scan.error() == 0)
            cout << "num: " << num << endl;
    }

    if(scan.error() != 0)
    {
        cout << "Scan ERROR\n";
    }

    return 0;
}

void printHeader(char* buf)
{
	cout << "header: \"";
	for(unsigned int i=0; i < strlen(buf); i++)
	{
		switch (buf[i])
		{
		case '\n':
			cout << "\\n";
			break;
		case '\r':
			cout << "\\r";
			break;
		case '\t':
			cout << "\\t";
			break;		
		default:
			cout << buf[i];
			break;
		}
	}
	cout << "\"\nbytes: {";
	for(unsigned int i=0; i < strlen(buf); i++)
	{
		cout << (int)(buf[i]);
		cout << ",";
	}
	cout << "}\n";
}

#endif