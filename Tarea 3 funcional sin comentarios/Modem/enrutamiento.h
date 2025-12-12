#ifndef ENRUTAMIENTO_H
#define ENRUTAMIENTO_H

#include <Arduino.h>
#include "structProtocolo.h"
#include "red.h"
#include "LoRa.h"

// Definir ID si no está en structProtocolo
#ifndef MY_IP_ADDR
#define MY_IP_ADDR 8
#endif

void setupRecepcion();
void revisarRecepcionUART();
void revisarRecepcionLoRa();
void enviarHaciaRaspberry(uint8_t* data, int len);

#endif