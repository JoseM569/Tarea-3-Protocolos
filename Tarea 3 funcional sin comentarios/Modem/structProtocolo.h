#ifndef STRUCT_PROTOCOLO_H
#define STRUCT_PROTOCOLO_H

#include <stdint.h>

// --- CONFIGURACIÓN DE IDENTIDAD ---
#define MY_IP_ADDR 8
#define BROADCAST_IP 65535

// Constantes SLIP
#define SLIP_END     0xC0
#define SLIP_ESC     0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

// Estructura de Capa 4 (Protocolo Propio / Legacy)
struct protocolo {
    uint8_t cmd;        // Comando (0=Nada, 1=ACK, 2=Txt, 4=LED...)
    uint8_t lng;        // Longitud de datos
    uint8_t data[64];   // Payload real
    uint16_t fcs;       // Checksum del payload
} __attribute__((packed));

// Estructura de Capa 3 (IPv4 Simplificado - 12 bytes Header)
struct packet_ipv4 {
    uint16_t fragOffset; // 3 bits flags + 13 bits offset
    uint8_t totalLen;
    uint8_t padding;     // Relleno
    uint16_t id;
    uint8_t protocol;
    uint8_t headerChecksum;
    uint16_t srcIP;
    uint16_t destIP;
    
    // El payload es la estructura de capa 4
    protocolo payload;
} __attribute__((packed));

#endif