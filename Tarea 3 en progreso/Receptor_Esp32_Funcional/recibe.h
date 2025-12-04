#ifndef RECIBE_H
#define RECIBE_H

#include "structProtocolo.h"
#include "red.h" // <-- Librería del profesor

// Objeto global de red (LoRa)
extern Red red;

// Inicializa Serial y LoRa
void setupRecepcion();

// Revisa UART (RPi -> ESP32)
void revisarRecepcionUART();

// Revisa LoRa (Aire -> ESP32)
void revisarRecepcionLoRa();

// Funciones auxiliares
unsigned short fcs(uint8_t * array, int tam);
uint8_t header_checksum(void* vdata, size_t length);

#endif