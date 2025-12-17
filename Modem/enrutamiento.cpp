#include "enrutamiento.h"
#include "hardware.h"
#include <Arduino.h>

Red red; // Instancia librería profesor

uint8_t rxBuffer[256];
uint8_t loraRxBuffer[256];
int rxIndex = 0;
bool slipEscaping = false;

// Variables temporales para desempaquetado
uint8_t rx_flag;
uint16_t rx_offset;
uint16_t rx_id;
uint8_t rx_proto;
uint16_t rx_src;
uint16_t rx_dest;
protocolo rx_payload;

void setupRecepcion() {
    Serial.begin(115200); 
    red.begin(7, 250000, 1, 15); 
    // Serial.println("Modem Iniciado"); 
}

uint8_t calcular_checksum(uint8_t *frame) {
    unsigned int acc = 0;
    for (int i = 0; i < 7; i++) acc += frame[i];
    return (uint8_t)(~acc);
}

// --- ENVÍO LORA ---
void enviarPaqueteLoRaProfesor(uint8_t proto, uint16_t dest, uint8_t flag, uint16_t offset, protocolo &p) {
    uint8_t frame[100]; 
    uint8_t totalLen = LARGO_HEADER + 4 + p.lng; 
    
    static uint16_t pid_counter = 0;
    uint16_t currentID = pid_counter++;

    frame[0] = (flag & 0x0F) | ((offset & 0x0F) << 4);
    frame[1] = (offset >> 4) & 0xFF;
    frame[2] = totalLen;
    frame[3] = 0; 

    // ID del paquete
    memcpy(&frame[4], &currentID, 2);

    frame[6] = proto;
    frame[7] = calcular_checksum(frame);

    // IPs
    uint16_t src = MY_IP_ADDR; 
    memcpy(&frame[8], &src, 2);   
    memcpy(&frame[10], &dest, 2); 

    // Payload
    frame[12] = p.cmd;
    frame[13] = p.lng;
    memcpy(&frame[14], p.data, p.lng);
    
    red.transmite_data(frame, totalLen);
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

    uint16_t srcIP, destIP;
    memcpy(&srcIP, &rxBuffer[8], 2);
    memcpy(&destIP, &rxBuffer[10], 2);
    uint8_t proto = rxBuffer[6];

    mostrarEnrutamientoOLED(srcIP, destIP, proto); 
    
    red.transmite_data(rxBuffer, len);
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

// --- RECEPCIÓN LORA ---
void revisarRecepcionLoRa() {
    if (red.dataDisponible()) {
        int n = red.getData(loraRxBuffer, 255);
        if (n < 12) return; 

        rx_flag = (loraRxBuffer[0] & 0x0F);
        rx_offset = ((loraRxBuffer[0] >> 4) & 0x0F) | ((loraRxBuffer[1] & 0xFF) << 4);
        
        memcpy(&rx_id, &loraRxBuffer[4], 2);
        
        rx_proto = loraRxBuffer[6];
        memcpy(&rx_src, &loraRxBuffer[8], 2);
        memcpy(&rx_dest, &loraRxBuffer[10], 2);

        bool esParaMi = (rx_dest == MY_IP_ADDR);
        bool esBroadcast = (rx_dest == BROADCAST_IP);

        if (esParaMi || esBroadcast) {
            
            enviarHaciaRaspberry(loraRxBuffer, n);
            mostrarRecepcionLoRa(n);

            if (n >= 14 && esParaMi) {
                rx_payload.cmd = loraRxBuffer[12];
                rx_payload.lng = loraRxBuffer[13];
                memcpy(rx_payload.data, &loraRxBuffer[14], rx_payload.lng);
                
                // Responder ACK
                if (rx_proto == 2 || (rx_proto >= 5 && rx_proto != 9)) {
                     delay(50);
                     protocolo ack; 
                     memset(&ack, 0, sizeof(ack));
                     
                     ack.cmd = 1; ack.lng = 2;
                     
                     // --- CORRECCIÓN FINAL (Big Endian) ---
                     // Enviamos primero el Byte Alto (>>8) y luego el Bajo.
                     // Así la Raspberry (que espera Alto primero) leerá: 00 01 -> 1.
                     ack.data[0] = (rx_id >> 8) & 0xFF; 
                     ack.data[1] = rx_id & 0xFF;        
                     
                     enviarPaqueteLoRaProfesor(1, rx_src, 0, 0, ack);
                }

                ejecutarComando(rx_payload); 
                
                if (rx_proto == 5) {
                    delay(500);
                    protocolo p; p.cmd = 2; p.lng = 6; 
                    strcpy((char*)p.data, "ESP_OK");
                    enviarPaqueteLoRaProfesor(0, rx_src, 0, 0, p);
                }
            }
        }
    }
}