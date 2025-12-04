#ifndef STRUCT_PROTOCOLO_H
#define STRUCT_PROTOCOLO_H

#include <stdint.h>
#include <string.h>

// --- Configuración de Red (ID DEL KIT) ---
#define MY_IP_ADDR    8      
#define BROADCAST_IP  0xFFFF 

// --- Tipos de Protocolo ---
#define PROTO_LEGACY  0  
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
typedef struct __attribute__((packed)) {
    uint16_t fragOffset;      
    uint8_t  totalLen;        
    uint8_t  padding;         
    uint16_t id;              
    uint8_t  protocol;        
    uint8_t  headerChecksum;  
    uint16_t srcIP;           
    uint16_t destIP;          
    
    protocolo payload;        
    
    uint8_t frame[200]; 
} packet_ipv4;

#endif