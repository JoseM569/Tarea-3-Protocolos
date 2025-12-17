#ifndef STRUCT_PROTOCOLO_H
#define STRUCT_PROTOCOLO_H

#include <stdint.h>

// --- CONFIGURACIÓN DE IDENTIDAD ---
#ifndef MY_IP_ADDR
#define MY_IP_ADDR 8  // <--- IMPORTANTE: ID DE TU ESP32
#endif

#define BROADCAST_IP 65535
#define LARGO_HEADER 12

// Constantes SLIP
#define SLIP_END     0xC0
#define SLIP_ESC     0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

// Estructura de Capa 4 (Payload)
struct __attribute__((packed)) protocolo {
    uint8_t cmd;        
    uint8_t lng;        
    uint8_t data[64];   
    uint16_t fcs;       
};

#endif