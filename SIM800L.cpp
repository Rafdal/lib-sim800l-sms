#include "SIM800L.h"

// Set pointer to SoftwareSerial sim module and Reset callback
void SIM800L::begin(SoftwareSerial *sim_module, void (*rst)(void))
{
	if (sim_module != NULL && rst != NULL)
	{
		this->reset = rst;
		this->sim_module = sim_module;
		sim_module->begin(9600);
		sim_module->setTimeout(SIM800L_READ_CHAR_TIMEOUT);
		sim_module->print(F("AT+CFUN=1,1\r\n"));

		printAndWaitOK(F("AT\r\n"));
		printAndWaitOK(F("AT+CMGF=1\r\n"));
		printAndWaitOK(F("AT+CNMI=2,2,0,0,0\r\n"));
		printAndWaitOK(F("AT+COPS?\r\n"));
		printAndWaitOK(F("AT+CLIP=1\r\n"));

		
	}
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

// Print flash-stored string (with F macro) and wait for OK response
// Max size is SIM800L_COM_BUF_SIZE (with null terminator)
void SIM800L::printAndWaitOK(const __FlashStringHelper *msg)
{
	if (this->reset != NULL)
	{
		// retrieved from WString.h and modified
		PGM_P p = reinterpret_cast<PGM_P>(msg);
		size_t n = 0;
		char msg_buf[SIM800L_COM_BUF_SIZE];
		while (n < SIM800L_COM_BUF_SIZE - 1)
		{
			unsigned char c = pgm_read_byte(p++);

			if (msg_buf[n] = c)
				n++;
			else
				break; // break in case of a null terminator
		}
		msg_buf[SIM800L_COM_BUF_SIZE - 1] = (char)0; // set a null terminator at the end of buf in case the buffer is full

		sim_module->print(msg_buf);
		unsigned long timestamp = millis();

		while (strstr(read_module_bytes(8), "OK") == NULL) // Read just 8 bytes
		{
			sim_module->print(msg);

			if (timestamp + timeout <= millis()) // If timeout is excedeed
				this->reset();
		}
	}
}