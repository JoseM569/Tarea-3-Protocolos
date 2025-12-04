#ifndef FUNCIONES_PROTOCOLO_H
#define FUNCIONES_PROTOCOLO_H

#include "structProtocolo.h"

unsigned short fcs(uint8_t * array, int tam);
uint8_t header_checksum(void* vdata, size_t length);

// Enviar (Ya la tenías)
void enviarFrameIPv4(uint8_t protocol, uint16_t dest, int flag, int offset, protocolo proto);

// --- NUEVA: Recibir ---
// Retorna true si se completó un paquete válido y lo guarda en 'packet'
bool recibirFrameIPv4(packet_ipv4 &packet);

#endif