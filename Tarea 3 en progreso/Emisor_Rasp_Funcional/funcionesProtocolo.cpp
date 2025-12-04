#include "funcionesProtocolo.h"
#include <wiringSerial.h>
#include <unistd.h> 
#include <stdio.h> // printf

extern int fd_serial; 

// Buffers de recepción
uint8_t rx_buffer[512];
int rx_index = 0;
bool slip_escaped = false;

// --- Helpers de Envío (Sin cambios) ---
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

uint8_t header_checksum(void* vdata, size_t length) {
    uint8_t* data = (uint8_t*)vdata;
    unsigned int acc = 0;
    for (size_t i = 0; i < length; i++) acc += data[i];
    return (uint8_t)(~acc);
}

void enviarFrameIPv4(uint8_t protocol, uint16_t dest, int flag, int offset, protocolo proto) {
    if (fd_serial < 0) return;
    proto.fcs = fcs(proto.data, proto.lng); 

    packet_ipv4 packet;
    packet.fragOffset = ((flag & 0x07) << 13) | (offset & 0x1FFF);
    packet.totalLen = 12 + 4 + proto.lng;
    packet.padding = 0;
    static uint16_t pid = 0; packet.id = pid++;
    packet.protocol = protocol;
    packet.srcIP = MY_IP_ADDR;
    packet.destIP = dest;
    packet.headerChecksum = 0;
    packet.payload = proto;

    uint8_t raw_header[12];
    raw_header[0] = (packet.fragOffset >> 8) & 0xFF;
    raw_header[1] = packet.fragOffset & 0xFF;
    raw_header[2] = packet.totalLen;
    raw_header[3] = packet.padding;
    raw_header[4] = (packet.id >> 8) & 0xFF;
    raw_header[5] = packet.id & 0xFF;
    raw_header[6] = packet.protocol;
    raw_header[7] = 0; 
    raw_header[8] = (packet.srcIP >> 8) & 0xFF;
    raw_header[9] = packet.srcIP & 0xFF;
    raw_header[10] = (packet.destIP >> 8) & 0xFF;
    raw_header[11] = packet.destIP & 0xFF;
    raw_header[7] = header_checksum(raw_header, 12);

    enviarByteUART(fd_serial, SLIP_END);
    for(int i=0; i<12; i++) enviarByteSLIP(fd_serial, raw_header[i]);
    enviarByteSLIP(fd_serial, packet.payload.cmd);
    enviarByteSLIP(fd_serial, packet.payload.lng);
    for(int i=0; i<packet.payload.lng; i++) enviarByteSLIP(fd_serial, packet.payload.data[i]);
    enviarByteSLIP(fd_serial, (packet.payload.fcs >> 8) & 0xFF);
    enviarByteSLIP(fd_serial, packet.payload.fcs & 0xFF);
    enviarByteUART(fd_serial, SLIP_END);
}

// --- NUEVA: Lógica de Recepción ---
bool recibirFrameIPv4(packet_ipv4 &packet) {
    if (fd_serial < 0) return false;

    // Leemos todo lo que haya en el buffer del serial
    while (serialDataAvail(fd_serial) > 0) {
        uint8_t b = serialGetchar(fd_serial);

        if (b == SLIP_END) {
            if (rx_index > 12) { // Si tenemos un paquete completo (Header + algo)
                // 1. Decodificar Header (Big Endian a Host)
                packet.fragOffset = (rx_buffer[0] << 8) | rx_buffer[1];
                packet.totalLen   = rx_buffer[2];
                // ... saltamos padding e ID ...
                packet.protocol   = rx_buffer[6];
                packet.srcIP      = (rx_buffer[8] << 8) | rx_buffer[9];
                packet.destIP     = (rx_buffer[10] << 8) | rx_buffer[11];

                // 2. Extraer Payload Legacy
                // El payload empieza en el byte 12
                if (rx_index >= 16) { // 12 Header + 4 min legacy
                    packet.payload.cmd = rx_buffer[12];
                    packet.payload.lng = rx_buffer[13];
                    // Copiar datos (máx 63)
                    int dataLen = packet.payload.lng;
                    if(dataLen > 63) dataLen = 63;
                    
                    for(int i=0; i<dataLen; i++) {
                        packet.payload.data[i] = rx_buffer[14+i];
                    }
                    packet.payload.data[dataLen] = '\0'; // Null terminate string
                }
                
                rx_index = 0;
                slip_escaped = false;
                return true; // ¡Paquete listo!
            }
            rx_index = 0; // Reset si era paquete vacío
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
    return false; // No hay paquete completo aún
}