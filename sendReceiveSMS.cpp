#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SIM800L.h>

SoftwareSerial simSerial(5,6);
SIM800L sim;

void setup()
{
    Serial.begin(9600);
    sim.begin(&simSerial);

    sim.onMessage([](SMSMessage& msg){
        
        Serial.println("Se recibio un nuevo mensaje");
        msg.print();
    });
}

void loop()
{
    sim.run();
}