#ifndef CAPA_RED_H
#define CAPA_RED_H

#include "structProtocolo.h"
#include <stdint.h>
#include <stddef.h>

#define LARGO_HEADER 12

// Funciones de utilidad
unsigned short fcs(uint8_t * array, int tam);

// Esta función implementa la lógica de suma del profesor
uint8_t calcular_checksum_header(uint8_t* data);

// Envío
uint16_t enviarFrameIPv4(uint8_t protocol, uint16_t dest, int flag, int offset, protocolo proto);

// Recepción
bool recibirFrameIPv4(packet_ipv4 &packet);

// Limpieza
void purgarBuffer();

#endif