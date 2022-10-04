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


#include <iostream>
#include <vector>
using namespace std;

#include "../src/ScanUtil.h"
#include "../src/SMSMessage.h"

void printHeader(char* buf);
void parseIncomingSMS(char* buf);

void smsCallback(SMSMessage& sms);

#define F(a) (a)
#define DEBUG_PRINT(a)  cout << (a) << endl;
#define printAndWaitOK(a) DEBUG_PRINT(a)
#define THROW_TEST_ERROR    return 1;

int main()
{
    cout << "Start main_test.cpp" << endl;

    // Pretty basic tests are done here
    const char *str1 = "\r\n+CMT: \"+5493487123456\",\"\",\"22/06/28,22:02:24-12\"\r\nBoooenas\r\n\0";
    const char *str2 = "AT+CREG?\r\n\r\n+CREG: 032,1\r\nald82s\0";
    const char *str3 = "AT+TEST\r\n\r\nNUM: 3487541299\0";

    vector<const char*> strList = {str1, str2, str3};

    for (auto& s : strList)
    {
        cout << endl;
        size_t bufferSize = strlen(s);
        char* buffer = const_cast<char*>(s);
        uint8_t netStatus = 0;

        char header[10];
        memset(header, 0, 10);
        strncpy(header, buffer, 9);

        printHeader(header);
        // Choose action by header content
		if (strstr(header, "+CMT:") != NULL)	// New SMS received
		{
			parseIncomingSMS(buffer);
		}
		else if(strstr(header, "+CREG") != NULL) // Network Status
		{
			ScanUtil scan(buffer, bufferSize);
			scan.skipTo(',');
			scan.get_uint8_t(&netStatus);
			if(scan.error() > 0)
			{
				DEBUG_PRINT(F("error reading netStatus"))
				netStatus = 0;
                THROW_TEST_ERROR
			}
		}
		else if(strstr(buffer, "\"SM\"") != NULL)  // "SM"
		{
			DEBUG_PRINT(F("Received \"SM\""))
			// This should not happen, but here is a workaround
			printAndWaitOK(F("AT+CMGF=1")); 		// Select SMS Message Format (1 = text mode)
			printAndWaitOK(F("AT+CNMI=1,2,0,0,0")); // SMS Message Indications
		}
        else if(strstr(buffer, "TEST") != NULL)
        {
			ScanUtil scan(buffer, bufferSize);
            scan.seek("NUM: ");
            unsigned long num;
            scan.get_unsigned_long(&num);

            if(scan.error() == 0)
                cout << "num: " << num << endl;
            else
                THROW_TEST_ERROR
        }
    }
    

    return 0;
}

void smsCallback(SMSMessage& sms)
{
    sms.print();
    if( sms.search("encender") )
    {
        cout << "Found command" << endl;
    }
    else
    {
        cout << "command NOT FOUND" << endl;
    }
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

void parseIncomingSMS(char* buffer)
{
	ScanUtil scan(buffer);
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

			if(scan.error() == 0)// no errors
			{
                smsCallback(sms);
                return;
			}
		}
	}

    throw;
}
