#ifndef STRUCT_PROTOCOLO_H
#define STRUCT_PROTOCOLO_H

#include <stdint.h>

// --- CONFIGURACIÓN DE IDENTIDAD ---
// CAMBIA ESTO SEGÚN EL NODO (8 o 16)
#define MY_IP_ADDR 8  
#define BROADCAST_IP 65535

// Constantes SLIP
#define SLIP_END     0xC0
#define SLIP_ESC     0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

// Estructura de Capa 4 (Payload)
struct protocolo {
    uint8_t cmd;        
    uint8_t lng;        
    uint8_t data[64];   
    uint16_t fcs;       
} __attribute__((packed));

// Estructura contenedora para IPv4 (Desempaquetada)
// Ya no usamos bitfields complejos, guardamos los datos limpios aquí
struct packet_ipv4 {
    uint8_t flags;       // 4 bits (según código profe)
    uint16_t offset;     // 12 bits (según código profe)
    
    uint8_t totalLen;    
    uint8_t padding;     // Relleno (byte 3)
    
    uint16_t id;         
    uint8_t protocol;    
    uint8_t headerChecksum; 
    
    uint16_t srcIP;      
    uint16_t destIP;     
    
    protocolo payload;
};

#endif