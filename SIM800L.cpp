#include "SIM800L.h"

// Set pointer to SoftwareSerial sim module and Reset callback
void SIM800L::begin(SoftwareSerial *sim_module, void (*rst)(void))
{
	if (sim_module != NULL && rst != NULL)
	{
		this->resetCallback = rst;
		this->sim_module = sim_module;
		sim_module->begin(SIM800L_BAUDRATE);
		sim_module->setTimeout(SIM800L_READ_CHAR_TIMEOUT); 	// timeout per character
		sim_module->print(F("AT+CFUN=1,1\r\n")); 			// Software Reset
		_delay_ms(50);

		printAndWaitOK(F("AT"));
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
		{
			break;
		}
	}
}

void SIM800L::parseIncomingSMS()
{
	ScanUtil scan(str);
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
				// TODO: SMS Callback
			}
		}
	}
}

void SIM800L::run()
{
	// this is non-blocking
	if(sim_module->available()) // if we have some bytes
	{
		readToBuffer();

		char header[10];
		memset(header, 0, 10);
		strncpy(header, buffer, 9);

		if (strstr(header, "+CMT:") != NULL)
		{
			parseIncomingSMS();
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
			// This should not happen, but here is a workaround
			printAndWaitOK(F("AT+CMGF=1")); 		// Select SMS Message Format (1 = text mode)
			printAndWaitOK(F("AT+CNMI=1,2,0,0,0")); // SMS Message Indications
		}

		printBuffer();
	}
}

void SIM800L::printBuffer()
{
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
}

// Read n bytes from sim module or until terminator has been found and store them in buffer
// returns pointer to buffer
char* SIM800L::read_module_bytes(size_t nbytes, char terminator = '\0')
{
	size_t nread = sim_module->readBytesUntil(terminator, buffer, nbytes);

	// Set null-terminator
	if(nread >= SIM800L_READ_BUF_SIZE)
		buffer[SIM800L_READ_BUF_SIZE-1] = (char)0;
	else
		buffer[nread] = (char)0;

	return buffer;
}

// Print flash-stored string (with F macro) to the module and wait for OK response
// Max size is SIM800L_COMM_BUF_SIZE (with null terminator)
void SIM800L::printAndWaitOK(const __FlashStringHelper *msg)
{
	if (this->resetCallback != NULL)
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
			
			if (timestamp + timeout <= millis()) // If long timeout is excedeed
				this->resetCallback();
		}
	}
}