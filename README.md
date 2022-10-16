# lib-sim800l-sms (WIP)
Simple and robust C++ library to handle SIM800L chip for sending and receiving SMS

[![flawfinder](https://github.com/Rafdal/lib-sim800l-sms/actions/workflows/flawfinder.yml/badge.svg?branch=main)](https://github.com/Rafdal/lib-sim800l-sms/actions/workflows/flawfinder.yml)
[![CodeQL](https://github.com/Rafdal/lib-sim800l-sms/actions/workflows/codeql.yml/badge.svg)](https://github.com/Rafdal/lib-sim800l-sms/actions/workflows/codeql.yml)

## Supported Features
- Receiving SMS (polling approach with callbacks, non-blocking)
- Sending SMS
- Check GSM network connection
- Software reset for the SIM module

## Considerations
- SMS text content is automatically converted to lowercase and normalized

## Known issues
- Some SMS text scanning methods are actually pretty inefficient

## Example code
```c++
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
        Serial.print(F("@ SIM: Connection status: "));
        Serial.println(connected);
	});
}

void loop()
{
    sim.run();
}

void messageCallback(SMSMessage& sms)
{
	sms.print();

	if( sms.search("info") )
	{
		const char str[] = "SOME INFO";
		strcpy(sms.message, str);
		sms.size = strlen(str);

		sim.sendMessage(sms);
	}
}
```

## Work In Progress...

## Contact and Feedback
Gmail: rdalzotto@itba.edu.ar