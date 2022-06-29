#include "SIM800L.h"

// Set pointer to SoftwareSerial sim module and Reset callback
void SIM800L::begin(SoftwareSerial *sim_module, void (*rst)(void))
{
	if (sim_module != NULL && rst != NULL)
	{
		this->resetCallback = rst;
		this->sim_module = sim_module;
		sim_module->begin(9600);
		sim_module->setTimeout(SIM800L_READ_CHAR_TIMEOUT); 	// timeout per character
		sim_module->print(F("AT+CFUN=1,1\r\n")); 			// Software Reset

		printAndWaitOK(F("AT\r\n"));
		printAndWaitOK(F("AT+CMGF=1\r\n")); 		// Select SMS Message Format (1 = text mode)
		printAndWaitOK(F("AT+CNMI=1,2,0,0,0\r\n")); // SMS Message Indications
		printAndWaitOK(F("AT+COPS?\r\n"));
		// printAndWaitOK(F("AT+CLIP=1\r\n"));
	}
}

void SIM800L::run()
{
	if(sim_module->available()) // if we have bytes
	{
		unsigned long timestamp = millis();

		bufferSize = 0;
		
		// read til no more incoming bytes
		while ((millis() - timestamp) <= SIM800L_READ_CHAR_TIMEOUT)
		{
			if(sim_module->available() && bufferSize < SIM800L_READ_BUF_SIZE)
			{
				buffer[bufferSize++] = sim_module->read();
				timestamp = millis();
			}
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
char* SIM800L::read_module_bytes(size_t nbytes, char terminator = '\n')
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
		unsigned long timestamp = millis();

		while (strstr(read_module_bytes(32), "OK") == NULL) // Read just 8 bytes
		{
			_delay_ms(20); // without this delay it doesnt works (maybe a buffering problem)

			sim_module->print(msg);
			
			if (timestamp + timeout <= millis()) // If timeout is excedeed
				this->resetCallback();
		}

	}
}