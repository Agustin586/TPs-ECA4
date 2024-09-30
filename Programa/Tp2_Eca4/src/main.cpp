#include <Arduino.h>

#include <Sensado.h>
#include <HMI_Local.h>

#define LED_PIN 2

void setup()
{
  Serial.begin(115200); // COnfiguramos el puerto serial

  HmiLocal_init();
  Sensado_init();
}

void loop()
{
}