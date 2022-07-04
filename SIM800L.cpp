/**
 * @file SIM800L.cpp
 * @author Rafael Dalzotto (rdalzotto@itba.edu.ar)
 * @brief Main library source file
 * @date 2022-07-04
 * 
 * @copyright Copyright (c) 2022
 */

#include "SIM800L.h"

#define SMS_TERMINATOR	(char)26  // required according to the datasheet

void SIM800L::begin(SoftwareSerial *sim_module, VoidCallbackVoid rst)
{
	if (sim_module != NULL)
	{
		this->resetCallback = rst;
		this->sim_module = sim_module;
		sim_module->begin(SIM800L_BAUDRATE);
		sim_module->setTimeout(SIM800L_READ_CHAR_TIMEOUT); 	// timeout per character

		printAndWaitOK(F("AT"));				// Sync
		printAndWaitOK(F("AT+CMGF=1")); 		// Select SMS Message Format (1 = text mode)
		printAndWaitOK(F("AT+CNMI=1,2,0,0,0")); // SMS Message Indications
		printAndWaitOK(F("AT+CLIP=1"));			// Show incoming call telephone number
	}
}

void SIM800L::readToBuffer()
{
	unsigned long timestamp = millis();
	bufferSize = 0;
	
	// read until no more incoming bytes
	while ((millis() - timestamp) <= SIM800L_READ_CHAR_TIMEOUT)
	{
		if(sim_module->available() && bufferSize < SIM800L_READ_BUF_SIZE)
		{
			buffer[bufferSize++] = sim_module->read();
			timestamp = millis();
		}
		else if(bufferSize >= SIM800L_READ_BUF_SIZE)
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

			if(scan.error() == 0)// no errors
			{
				#ifdef PRINT_SMS_SIM800L
				sms.print();
				#endif

				if(smsCallback != NULL)
				{
					smsCallback(sms);
				}
				else
				{
					DEBUG_PRINT(F("SMS Callback not set!"))
				}
			}
		}
	}
}

void SIM800L::sendMessage(SMSMessage& sms)
{
	if(sim_module != NULL)
	{
		printAndWaitOK(F("AT+CMGF=1")); 		// Select SMS Message Format (1 = text mode)
		_delay_ms(100);
		sim_module->print(F("AT+CMGS=\""));
		sim_module->print(sms.phone);
		sim_module->print(F("\"\r"));

		for(int i=0; i < sms.size && !(sms.message[i]) && i < SMS_MESSAGE_MAX_LEN; i++)
			sim_module->print(sms.message[i]);

		_delay_ms(30);
		sim_module->print(SMS_TERMINATOR);
		sim_module->print(F("\r\n"));
	}
}


void SIM800L::run()
{
	if(sim_module == NULL)
		return;
		
	// this is non-blocking
	if(sim_module->available()) // if we have some bytes
	{
		readToBuffer();
		printBuffer();

		uint8_t headSize = 12;
		char header[headSize];
		memset(header, 0, headSize);
		strncpy(header, buffer, headSize - 1); // Dont erase last null character!

		// Choose action by header content
		if (strstr(header, "+CMT:") != NULL)	// New SMS received
		{
			parseIncomingSMS();
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
			}
		}
		else if(strstr(str, "\"SM\"") != NULL)  // "SM"
		{
			DEBUG_PRINT(F("Received \"SM\""))
			// This should not happen, but here is a workaround
			printAndWaitOK(F("AT+CMGF=1")); 		// Select SMS Message Format (1 = text mode)
			printAndWaitOK(F("AT+CNMI=1,2,0,0,0")); // SMS Message Indications
		}

	}
}

void SIM800L::printBuffer()
{
	#ifdef PRINT_BUFFER_SIM800L
	Serial.print("\n\n@ SIM800L:\nbuffer: \"");
	for(unsigned int i=0; i < bufferSize; i++)
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
	for(unsigned int i=0; i < bufferSize; i++)
	{
		Serial.print((int)(buffer[i]));
		Serial.print(",");
	}
	Serial.println("}");
	#endif
}

// Read n bytes from sim module or until terminator has been found and store them in buffer
// returns pointer to buffer
char* SIM800L::read_module_bytes(size_t nbytes, char terminator)
{
	size_t nread = sim_module->readBytesUntil(terminator, buffer, nbytes);

	// Set null-terminator
	if(nread >= SIM800L_READ_BUF_SIZE)
		buffer[SIM800L_READ_BUF_SIZE-1] = (char)0;
	else
		buffer[nread] = (char)0;

	return buffer;
}

void SIM800L::reset()
{
	if(sim_module != NULL)
	{
		sim_module->print(F("AT+CFUN=1,1\r\n")); 	// Software Reset
		_delay_ms(100);
		if(resetCallback != NULL)
			this->resetCallback();

		begin(sim_module, resetCallback);
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

			if ( (msg_buf[n] = c) )
				n++;
			else
				break; // break in case of a null terminator
		}

		// set a null terminator at the end of buf in case the buffer is full
		msg_buf[SIM800L_COMM_BUF_SIZE - 1] = (char)0;

		DEBUG_PRINT(msg_buf)

		sim_module->print(msg_buf);
		sim_module->print(F("\r\n"));
		_delay_ms(50);

		unsigned long timestamp = millis();

		// TODO: Check maybe its better to just call readToBuffer ??? 
		while (strstr(read_module_bytes(10), "OK") == NULL) // Read just 10 bytes
		{
			sim_module->print(msg_buf);
			sim_module->print(F("\r\n"));

			_delay_ms(50); // without this delay it doesnt works (maybe a buffering problem)
			// TODO: Test this again without delay, but with readToBuffer

			if (timestamp + timeout <= millis()) // If long timeout is excedeed
				reset();
		}
	}
}