#include <Arduino.h>

#include <SoftwareSerial.h>
#include <SIM800L.h>

SoftwareSerial	simSerial(5,6); // (RX , TX)
SIM800L sim;


void messageCallback(SMSMessage& sms);

void setup()
{
	Serial.begin(115200);
	Serial.println("SIM800L TEST");

    sim.begin(&simSerial);

	Serial.print("Time to start: ");
	Serial.print((float)( millis() / 1000.0f ), 1);
	Serial.println(" s");


    sim.onMessage(messageCallback);

	sim.onConnectionStateChanged([](bool connected){
		if(connected)
			Serial.print(F("@ SIM:\tConnected "));
		else
			Serial.print(F("@ SIM:\tDisconnected "));
		Serial.print((float)(millis() / 1000.0f), 1);
		Serial.println(" s");
	});
}

void loop()
{
    sim.run();
}

void messageCallback(SMSMessage& sms)
{
	sms.print();
	
	char wordBuffer[SMS_WORD_SIZE_MAX];
	int wordCount = sms.countWords();

	Serial.print(F("- word list: "));
	Serial.println(wordCount);

	for (int i = 0; i < wordCount; i++)
	{
		if(sms.getNWord(i, wordBuffer))
		{
			Serial.print("\t");
			Serial.print(i);
			Serial.print(F(" - "));
			Serial.println(wordBuffer);
		}
	}

	if( sms.search("info") )
	{
		const char str[] = "INFORMACION\nFOR YOU";
		strcpy(sms.message, str);
		sms.size = strlen(str);

		sim.sendMessage(sms);
	}

	if( sms.compareWordAt(0, "set") )
	{
		if( sms.compareWordAt(1, "timer") )
		{
			Serial.println("SET TIMER!!!");
		}
		else if( sms.compareWordAt(1, "interval") )
		{
			Serial.println("SET INTERVAL!!!");
		}
		else
		{
			Serial.println("SET!!!");
		}
	}
}