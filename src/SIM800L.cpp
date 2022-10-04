/**
 * @file SIM800L.cpp
 * @author Rafael Dalzotto (rdalzotto@itba.edu.ar)
 * @brief Main library source file
 * @date 2022-07-04
 *
 * @copyright Copyright (c) 2022
 */

#include "SIM800L.h"

#define SMS_TERMINATOR (char)26 // required according to the datasheet

void SIM800L::begin(SoftwareSerial *sim_module, VoidCallback rst)
{
	if (sim_module != NULL)
	{
		this->resetCallback = rst;
		this->sim_module = sim_module;
		sim_module->begin(SIM800L_BAUDRATE);
		sim_module->setTimeout(SIM800L_READ_CHAR_TIMEOUT); // timeout per character

		printAndWaitOK(F("AT"));				// Sync
		printAndWaitOK(F("AT+CMGF=1"));			// Select SMS Message Format (1 = text mode)
		printAndWaitOK(F("AT+CNMI=1,2,0,0,0")); // SMS Message Indications
		printAndWaitOK(F("AT+CLIP=1"));			// Show incoming call telephone number
		
		_delay_ms(200);
		run(); // read buffer

		checkConnection();
	}
}

void SIM800L::readToBuffer()
{
	unsigned long timestamp = millis();
	bufferSize = 0;

	// read until no more incoming bytes
	while ((millis() - timestamp) <= SIM800L_READ_CHAR_TIMEOUT)
	{
		if (sim_module->available() && bufferSize < SIM800L_READ_BUF_SIZE)
		{
			buffer[bufferSize++] = sim_module->read();
			timestamp = millis();
		}
		else if (bufferSize >= SIM800L_READ_BUF_SIZE)
			break;
	}
}

void SIM800L::parseIncomingSMS()
{
	ScanUtil scan(buffer, bufferSize);
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

			if (scan.error() == 0) // no errors
			{
				if (smsCallback != NULL)
				{
					messageCallback(sms);
				}
				else
				{
					DEBUG_PRINT(F("SMS Callback not set!"))
				}
			}
			else
			{
				DEBUG_PRINT(F("SMS scan error!"))
			}
			
		}
	}
}

void SIM800L::messageCallback(SMSMessage& sms)
{
	smsCallback(sms);
}


void SIM800L::sendMessage(SMSMessage &sms)
{
	DEBUG_PRINT(F("@SEND: Sending message:"));
	sms.print();

	if (sim_module != NULL)
	{
		sim_module->print(F("AT+CMGF=1\r\n")); // Select SMS Message Format (1 = text mode)
		_delay_ms(100);
		sim_module->print(F("AT+CMGS=\""));
		sim_module->print(sms.phone);
		sim_module->print(F("\"\r"));

		for (int i = 0; i < sms.size && (sms.message[i] != NULLCHAR) && i < SMS_MESSAGE_MAX_LEN; i++)
			sim_module->print(sms.message[i]);

		_delay_ms(30);
		sim_module->print(SMS_TERMINATOR);
		sim_module->print(F("\r\n"));
	}
}


void SIM800L::run()
{
	if (sim_module == NULL)
		return;

	// this is non-blocking
	if (sim_module->available()) // if we have some bytes
	{
		readToBuffer();

		uint8_t headSize = 12;
		char header[headSize];
		memset(header, 0, headSize);
		strncpy(header, buffer, headSize - 1); // Dont erase last null character!

		// Choose action by header content
		if (strstr(header, "+CMT:") != NULL) // * New SMS received
		{
			parseIncomingSMS();
		}
		else if (strstr(header, "+CREG") != NULL) // * Network Status
		{
			ScanUtil scan(buffer, bufferSize);
			scan.skipTo(',');
			scan.get_uint8_t(&netStatus);
			if (scan.error() > 0)
			{
				DEBUG_PRINT(F("error reading netStatus"))
				netStatus = 0;
			}

			if (netStatus != lastNetStatus)
			{
				if (netChangedCallback != NULL)
					netChangedCallback( connected() );
				lastNetStatus = netStatus;

				if( !connected() )
				{
					disconnectedCount++;
					if(disconnectedCount == SIM800L_AUTO_RESET_AFTER_CHECK_FAILS)
					{
						reset();
					}
				}
				else
					disconnectedCount = 0;
			}
		}
		else if (strstr(buffer, "\"SM\"") != NULL) // * "SM"
		{
			DEBUG_PRINT(F("Received \"SM\""))
			// This should not happen, but here is a workaround
			printAndWaitOK(F("AT+CMGF=1"));			// Select SMS Message Format (1 = text mode)
			printAndWaitOK(F("AT+CNMI=1,2,0,0,0")); // SMS Message Indications
		}
		else
		{
			printBuffer(); // Unknown response
		}
	}

	if(millis() - lastMsConnectionCheckInterval >= SIM800L_CONNECTION_CHECK_INTERVAL)
	{
		checkConnection();
		lastMsConnectionCheckInterval = millis();
	}
}

void SIM800L::printBuffer()
{
#ifdef PRINT_BUFFER_SIM800L
	Serial.print("\n\n@ SIM800L:\nbuffer: \"");
	for (unsigned int i = 0; i < bufferSize; i++)
	{
		switch (buffer[i])
		{
		case '\n':
			Serial.print("\\n");
			break;
		case '\r':
			Serial.print("\\r");
			break;
		case '\t':
			Serial.print("\\t");
			break;
		default:
			Serial.print(buffer[i]);
			break;
		}
	}
	Serial.print("\"\nbytes: {");
	for (unsigned int i = 0; i < bufferSize; i++)
	{
		Serial.print((int)(buffer[i]));
		Serial.print(",");
	}
	Serial.println("}");
#endif
}

void SIM800L::reset()
{
	if (sim_module != NULL)
	{
		sim_module->print(F("AT+CFUN=1,1\r\n")); // Software Reset
		sim_module->print(F("AT+CFUN=1,1\r\n")); // Software Reset
		sim_module->print(F("AT+CFUN=1,1\r\n")); // Software Reset
		_delay_ms(1000);
		readToBuffer(); // flush serial

		if (resetCallback != NULL)
			this->resetCallback();

		asm("jmp 0x0000");
	}
}

// Print flash-stored string (with F macro) to the module and wait for OK response
// Max size is SIM800L_COMM_BUF_SIZE (with null terminator)
void SIM800L::printAndWaitOK(const __FlashStringHelper *msg)
{
	if (this->sim_module != NULL)
	{
		// retrieved from WString.h and modified
		PGM_P p = reinterpret_cast<PGM_P>(msg);
		size_t n = 0;
		char msg_buf[SIM800L_COMM_BUF_SIZE];
		while (n < SIM800L_COMM_BUF_SIZE - 1)
		{
			unsigned char c = pgm_read_byte(p++);

			if ((msg_buf[n] = c))
				n++;
			else
				break; // break in case of a null terminator
		}

		// set a null terminator at the end of buf in case the buffer is full
		msg_buf[SIM800L_COMM_BUF_SIZE - 1] = (char)0;

		unsigned long timestamp = millis();

		bool found = false;
		while (!found)
		{
			DEBUG_PRINT(msg_buf)

			sim_module->print(msg_buf);
			sim_module->print(F("\r\n"));

			_delay_ms(100); // without this delay it doesnt works (maybe a buffering problem)

			if (sim_module->available()) // if we have some bytes
			{
				readToBuffer();
				printBuffer();

				ScanUtil scan(buffer, bufferSize);

				if(scan.seek("OK"))
					found = true;
			}

			if (timestamp + timeout <= millis()) // If long timeout is excedeed
			{
				reset();
				return; // just in case, you know
			} 
		}
	}
}