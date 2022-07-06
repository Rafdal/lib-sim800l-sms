#include <Arduino.h>

#include <SoftwareSerial.h>
#include <SIM800L.h>

SoftwareSerial	simSerial(5,6); // 4,11  --- (RX , TX) ----- 5,6
SIM800L sim;

void setup()
{
	Serial.begin(115200);
	Serial.println("SIM800L TEST");

	unsigned long stamp = millis();
    sim.begin(&simSerial);

	Serial.print("Time to start: ");
	Serial.println((int)( (millis() - stamp)/1000 ));


    sim.onMessage([](SMSMessage& msg){
        
        Serial.println("Se recibio un nuevo mensaje");
        msg.print();
    });
}

void loop()
{
    sim.run();
}