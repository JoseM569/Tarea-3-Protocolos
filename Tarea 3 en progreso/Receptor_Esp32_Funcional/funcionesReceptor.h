#ifndef FUNCIONES_RECEPTOR_H
#define FUNCIONES_RECEPTOR_H

#include "structProtocolo.h"

// Pin del LED
#define LED_PIN 25

void setupHardware();
void mostrarMensajeBienvenidaOLED();
void ejecutarComando(protocolo& proto); 
void mostrarEnrutamientoOLED(uint16_t src, uint16_t dest, uint8_t proto);
void mostrarRecepcionLoRa(int n); // <--- NUEVA FUNCIÓN
void manejarParpadeoLED();

#endif