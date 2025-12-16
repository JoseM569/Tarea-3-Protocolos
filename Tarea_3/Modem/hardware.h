#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "structProtocolo.h"

// Pin del LED integrado (Suele ser 25 en Heltec o 2 en otros ESP32)
#define LED_PIN 25 

void setupHardware();
void mostrarMensajeBienvenidaOLED();
void ejecutarComando(protocolo& proto); 
void mostrarEnrutamientoOLED(uint16_t src, uint16_t dest, uint8_t proto);
void mostrarRecepcionLoRa(int n);
void manejarParpadeoLED(); // Se mantiene vacía por compatibilidad

#endif