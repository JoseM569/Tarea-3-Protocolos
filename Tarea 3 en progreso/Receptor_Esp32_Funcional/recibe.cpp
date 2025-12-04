#include "recibe.h"
#include "funcionesReceptor.h"
#include <Arduino.h>

// Instancia de la clase del profesor
Red red;

uint8_t rxBuffer[256];
uint8_t loraRxBuffer[256];
int rxIndex = 0;
bool slipEscaping = false;
packet_ipv4 tempPacket;

void setupRecepcion() {
    Serial.begin(115200); 
    red.begin(7, 250000, 1, 15);
    Serial.println("Modem Iniciado");
}

uint8_t header_checksum(void* vdata, size_t length) {
    uint8_t* data = (uint8_t*)vdata;
    unsigned int acc = 0;
    for (size_t i = 0; i < length; i++) acc += data[i];
    return (uint8_t)(~acc);
}

unsigned short fcs(uint8_t * array, int tam){
    unsigned short res= 0;
    for (int i = 0; i < tam; i++) {
        for (int j = 0; j < 8; j++) res += (array[i] >> j) & 0x01;  
    }
    return res;
}

void enviarHaciaRaspberry(uint8_t* data, int len) {
    if (len <= 0) return;
    Serial.write(SLIP_END);
    for(int i=0; i<len; i++) {
        if(data[i] == SLIP_END) { Serial.write(SLIP_ESC); Serial.write(SLIP_ESC_END); }
        else if(data[i] == SLIP_ESC) { Serial.write(SLIP_ESC); Serial.write(SLIP_ESC_ESC); }
        else { Serial.write(data[i]); }
    }
    Serial.write(SLIP_END);
}

void procesarPaqueteUART(int len) {
    if (len < 12) return; 

    tempPacket.destIP   = (rxBuffer[10] << 8) | rxBuffer[11];
    tempPacket.srcIP    = (rxBuffer[8] << 8) | rxBuffer[9];
    tempPacket.protocol = rxBuffer[6];

    bool esParaMi = (tempPacket.destIP == MY_IP_ADDR);
    bool esLegacy = (tempPacket.protocol == PROTO_LEGACY);

    if (esParaMi && esLegacy) {
        tempPacket.payload.cmd = rxBuffer[12];
        tempPacket.payload.lng = rxBuffer[13];
        for(int i=0; i<tempPacket.payload.lng; i++) 
            tempPacket.payload.data[i] = rxBuffer[14+i];
        ejecutarComando(tempPacket.payload);
    } 
    else if (!esLegacy) { 
        mostrarEnrutamientoOLED(tempPacket.srcIP, tempPacket.destIP, tempPacket.protocol);
        red.transmite_data(rxBuffer, len);
    }
}

void revisarRecepcionUART() {
    while (Serial.available()) {
        uint8_t b = Serial.read();
        if (b == SLIP_END) {
            if (rxIndex > 0) { procesarPaqueteUART(rxIndex); rxIndex = 0; }
        } else if (b == SLIP_ESC) slipEscaping = true;
        else {
            if (slipEscaping) { b=(b==SLIP_ESC_END)?SLIP_END:SLIP_ESC; slipEscaping=false; }
            if (rxIndex < 250) rxBuffer[rxIndex++] = b;
        }
    }
}

void revisarRecepcionLoRa() {
    if (red.dataDisponible()) {
        int n = red.getData(loraRxBuffer, 255);
        if (n < 12) return; 

        uint16_t destIP = (loraRxBuffer[10] << 8) | loraRxBuffer[11];
        
        if (destIP == MY_IP_ADDR || destIP == BROADCAST_IP) {
            enviarHaciaRaspberry(loraRxBuffer, n);
            
            // --- AQUÍ ESTABA EL ERROR, AHORA LLAMAMOS A LA FUNCIÓN CORRECTA ---
            mostrarRecepcionLoRa(n);
        }
    }
}