#ifndef CAPA_RED_H
#define CAPA_RED_H

#include "structProtocolo.h"
#include <stdint.h>
#include <stddef.h>

// Funciones de utilidad
unsigned short fcs(uint8_t * array, int tam);
uint8_t header_checksum(void* vdata, size_t length);

// Envío
uint16_t enviarFrameIPv4(uint8_t protocol, uint16_t dest, int flag, int offset, protocolo proto);

// Recepción
// Retorna true si se completó un paquete válido y lo guarda en 'packet'
bool recibirFrameIPv4(packet_ipv4 &packet);

#endif