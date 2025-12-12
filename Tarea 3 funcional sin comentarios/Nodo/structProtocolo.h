#ifndef STRUCT_PROTOCOLO_H
#define STRUCT_PROTOCOLO_H

#include <stdint.h>

// --- CONFIGURACIÓN DE IDENTIDAD ---
// Asegúrate de usar la misma lógica de direcciones
#define MY_IP_ADDR 8
#define BROADCAST_IP 65535

// Constantes SLIP
#define SLIP_END     0xC0
#define SLIP_ESC     0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

// Estructura de Capa 4 (Protocolo Propio / Legacy)
struct protocolo {
    uint8_t cmd;        // Comando
    uint8_t lng;        // Longitud
    uint8_t data[64];   // Payload
    uint16_t fcs;       // Checksum del payload
} __attribute__((packed));

// Estructura de Capa 3 (IPv4 Simplificado - 12 bytes Header)
// Coincide EXACTAMENTE con la Tabla 1 de tu imagen
struct packet_ipv4 {
    uint16_t fragOffset; // Bits 0-15: Flags + Offset
    uint8_t totalLen;    // Bits 16-23: Longitud Total
    uint8_t padding;     // Bits 24-31: Relleno (NECESARIO)
    
    uint16_t id;         // Identificador
    uint8_t protocol;    // Protocolo
    uint8_t headerChecksum; // Suma de verificación
    
    uint16_t srcIP;      // IP Origen
    uint16_t destIP;     // IP Destino
    
    // El payload (Capa 4) va pegado a continuación
    protocolo payload;
} __attribute__((packed));

#endif