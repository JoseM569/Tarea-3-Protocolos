#include "capaRed.h"
#include <wiringSerial.h>
#include <unistd.h> 
#include <stdio.h> 
#include <string.h> // Para memcpy

extern int fd_serial; 

uint8_t rx_buffer[512];
int rx_index = 0;
bool slip_escaped = false;

void enviarByteUART(int fd, uint8_t b) { serialPutchar(fd, b); }

void enviarByteSLIP(int fd, uint8_t b) {
    if (b == SLIP_END) { serialPutchar(fd, SLIP_ESC); serialPutchar(fd, SLIP_ESC_END); } 
    else if (b == SLIP_ESC) { serialPutchar(fd, SLIP_ESC); serialPutchar(fd, SLIP_ESC_ESC); } 
    else { serialPutchar(fd, b); }
}

unsigned short fcs(uint8_t * array, int tam){
    unsigned short res= 0;
    for (int i = 0; i < tam; i++) for (int j = 0; j < 8; j++) res += (array[i] >> j) & 0x01;
    return res;
}

// --- LÓGICA DE CHECKSUM DEL PROFESOR ---
// "checksum_header = checksum(frame, 7); // solo hasta Protocolo"
// Suma los bytes del 0 al 6.
uint8_t calcular_checksum_header(uint8_t* data) {
    unsigned int acc = 0;
    for (size_t i = 0; i < 7; i++) {
        acc += data[i];
    }
    return (uint8_t)(~acc); // Complemento a 1 (lo usual en estos checksums simples)
}

uint16_t enviarFrameIPv4(uint8_t protocol, uint16_t dest, int flag, int offset, protocolo proto) {
    if (fd_serial < 0) return 0;
    proto.fcs = fcs(proto.data, proto.lng); 

    // Preparamos los datos del paquete
    static uint16_t pid = 0; 
    uint16_t currentID = pid++;
    uint8_t totalLen = LARGO_HEADER + 4 + proto.lng; 

    // --- EMPAQUETADO SEGÚN CÓDIGO DEL PROFESOR ---
    uint8_t frame[12]; // Buffer temporal para el header
    memset(frame, 0, 12);

    // Byte 0: Flag (4 bits bajos) | Offset parte baja (4 bits altos)
    frame[0] = (flag & 0x0F) | ((offset & 0x0F) << 4);
    
    // Byte 1: Offset parte alta (8 bits restantes)
    frame[1] = (offset >> 4) & 0xFF;
    
    // Byte 2: Longitud Total
    frame[2] = totalLen & 0xFF;
    
    // Byte 3: Relleno (0)
    frame[3] = 0;

    // Byte 4-5: ID (memcpy para seguridad de endianness)
    memcpy(&frame[4], &currentID, 2);

    // Byte 6: Protocolo
    frame[6] = protocol & 0xFF;

    // Byte 7: Checksum (Se calcula con los bytes 0-6)
    frame[7] = calcular_checksum_header(frame);

    // Byte 8-9: IP Origen
    uint16_t src = MY_IP_ADDR;
    memcpy(&frame[8], &src, 2);

    // Byte 10-11: IP Destino
    memcpy(&frame[10], &dest, 2);

    // --- ENVÍO FÍSICO (SLIP) ---
    enviarByteUART(fd_serial, SLIP_END);
    
    // Enviar Header (12 bytes)
    for(int i=0; i<12; i++) enviarByteSLIP(fd_serial, frame[i]);
    
    // Enviar Payload
    enviarByteSLIP(fd_serial, proto.cmd);
    enviarByteSLIP(fd_serial, proto.lng);
    for(int i=0; i<proto.lng; i++) enviarByteSLIP(fd_serial, proto.data[i]);
    enviarByteSLIP(fd_serial, (proto.fcs >> 8) & 0xFF);
    enviarByteSLIP(fd_serial, proto.fcs & 0xFF);
    
    enviarByteUART(fd_serial, SLIP_END);

    return currentID;
}

bool recibirFrameIPv4(packet_ipv4 &packet) {
    if (fd_serial < 0) return false;

    while (serialDataAvail(fd_serial) > 0) {
        uint8_t b = serialGetchar(fd_serial);

        if (b == SLIP_END) {
            if (rx_index > 12) { 
                // --- DESEMPAQUETADO SEGÚN CÓDIGO DEL PROFESOR ---
                uint8_t* data = rx_buffer;

                // Recuperar Flag y Offset (Inverso al empaquetado)
                packet.flags = (data[0] & 0x0F);
                packet.offset = ((data[0] >> 4) & 0x0F) | ((data[1] & 0xFF) << 4);
                
                packet.totalLen = data[2] & 0xFF;
                packet.padding = 0; // data[3] es relleno

                memcpy(&packet.id, &data[4], 2);
                packet.protocol = data[6] & 0xFF;
                packet.headerChecksum = data[7] & 0xFF;
                
                memcpy(&packet.srcIP, &data[8], 2);
                memcpy(&packet.destIP, &data[10], 2);

                // Recuperar Payload
                if (rx_index >= 16) { 
                    packet.payload.cmd = rx_buffer[12];
                    packet.payload.lng = rx_buffer[13];
                    int dataLen = packet.payload.lng;
                    if(dataLen > 63) dataLen = 63;
                    
                    for(int i=0; i<dataLen; i++) {
                        packet.payload.data[i] = rx_buffer[14+i];
                    }
                }
                
                rx_index = 0;
                slip_escaped = false;
                return true; 
            }
            rx_index = 0; 
        } 
        else if (b == SLIP_ESC) {
            slip_escaped = true;
        } 
        else {
            if (slip_escaped) {
                if (b == SLIP_ESC_END) b = SLIP_END;
                else if (b == SLIP_ESC_ESC) b = SLIP_ESC;
                slip_escaped = false;
            }
            if (rx_index < 500) rx_buffer[rx_index++] = b;
        }
    }
    return false;
}

void purgarBuffer() {
    if (fd_serial >= 0) {
        serialFlush(fd_serial);
        rx_index = 0;
        slip_escaped = false;
    }
}