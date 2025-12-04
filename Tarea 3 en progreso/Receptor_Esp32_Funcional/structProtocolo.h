/**
 * @file structProtocolo.h
 * @brief Definiciones compartidas para el protocolo de comunicación UART con IPv4 Simplificado (16-bit).
 * @details CONFIGURACIÓN PARA KIT ID 8.
 */

#ifndef STRUCT_PROTOCOLO_H
#define STRUCT_PROTOCOLO_H

#include <stdint.h>
#include <string.h>

// --- Configuración de Red (ID DEL KIT) ---
// Ambos dispositivos (RPi y ESP32) son parte del mismo Kit 8
#define MY_IP_ADDR    8      
#define BROADCAST_IP  0xFFFF 

// --- Tipos de Protocolo (Capa 3) ---
#define PROTO_LEGACY  0  // Comandos Tarea 1 y 2 encapsulados
#define PROTO_ACK     1  
#define PROTO_UNICAST 2  
#define PROTO_BCAST   3  
#define PROTO_HELLO   4  
#define PROTO_TEST    5  
#define PROTO_LED     6  
#define PROTO_OLED    7  

// --- Constantes SLIP ---
#define SLIP_END      0xC0 
#define SLIP_ESC      0xDB 
#define SLIP_ESC_END  0xDC 
#define SLIP_ESC_ESC  0xDD 

// --- Estructura Legacy (Capa 4) ---
typedef struct __attribute__((packed)) {
    uint8_t cmd;
    uint8_t lng;
    uint8_t data[63];
    uint16_t fcs;
} protocolo;

// --- Estructura IPv4 Simplificado (Capa 3) ---
// Total Header: 12 Bytes
typedef struct __attribute__((packed)) {
    uint16_t fragOffset;      // 2 bytes
    uint8_t  totalLen;        // 1 byte
    uint8_t  padding;         // 1 byte
    uint16_t id;              // 2 bytes
    uint8_t  protocol;        // 1 byte
    uint8_t  headerChecksum;  // 1 byte
    uint16_t srcIP;           // 2 bytes
    uint16_t destIP;          // 2 bytes
    
    protocolo payload;        // Capa 4
    
    // Buffer auxiliar
    uint8_t frame[200]; 
} packet_ipv4;

#endif // STRUCT_PROTOCOLO_H