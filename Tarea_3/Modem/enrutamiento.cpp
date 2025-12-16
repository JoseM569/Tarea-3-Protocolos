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
    Serial.println("Modem Iniciado - ID 8 (Modo memcpy)");
}

uint8_t calcular_checksum(uint8_t *frame) {
    unsigned int acc = 0;
    for (int i = 0; i < 7; i++) acc += frame[i];
    return (uint8_t)(~acc);
}

// --- ENVÍO LORA (USANDO MEMCPY COMO EL PROFESOR) ---
void enviarPaqueteLoRaProfesor(uint8_t proto, uint16_t dest, uint8_t flag, uint16_t offset, protocolo &p) {
    uint8_t frame[100]; 
    uint8_t totalLen = LARGO_HEADER + 4 + p.lng; 
    
    static uint16_t pid_counter = 0;
    uint16_t currentID = pid_counter++;

    // 1. Header (Bits del profesor - Flag y Offset manuales porque comparten bytes)
    frame[0] = (flag & 0x0F) | ((offset & 0x0F) << 4);
    frame[1] = (offset >> 4) & 0xFF;
    frame[2] = totalLen;
    frame[3] = 0; 

    // 2. ID (Copiado directo con memcpy, sin inventar orden)
    memcpy(&frame[4], &currentID, 2);

    frame[6] = proto;
    frame[7] = calcular_checksum(frame);

    // 3. IPs (Copiadas directo con memcpy)
    uint16_t src = MY_IP_ADDR; 
    memcpy(&frame[8], &src, 2);   // IP Origen
    memcpy(&frame[10], &dest, 2); // IP Destino

    // 4. Payload
    frame[12] = p.cmd;
    frame[13] = p.lng;
    memcpy(&frame[14], p.data, p.lng);
    
    // Enviar a la red
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

// --- PROCESAMIENTO UART (Cable) ---
void procesarPaqueteUART(int len) {
    if (len < 12) return; 

    // Extraer IPs usando memcpy para mostrar en pantalla
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

// --- RECEPCIÓN LORA (Aire) ---
// --- RECEPCIÓN LORA (CON DEBUG Y LIMPIEZA) ---
void revisarRecepcionLoRa() {
    if (red.dataDisponible()) {
        int n = red.getData(loraRxBuffer, 255);
        if (n < 12) return; 

        // 1. Desempaquetado del Header
        rx_flag = (loraRxBuffer[0] & 0x0F);
        rx_offset = ((loraRxBuffer[0] >> 4) & 0x0F) | ((loraRxBuffer[1] & 0xFF) << 4);
        
        // Usamos memcpy para leer el ID tal cual viene (seguro del profesor)
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
                // DEBUG: ¡Aquí veremos la verdad!
                Serial.print("[DEBUG] ID Recibido: "); Serial.println(rx_id);

                rx_payload.cmd = loraRxBuffer[12];
                rx_payload.lng = loraRxBuffer[13];
                memcpy(rx_payload.data, &loraRxBuffer[14], rx_payload.lng);
                
                // Responder ACK
                if (rx_proto == 2 || (rx_proto >= 5 && rx_proto != 9)) {
                     delay(50);
                     
                     // Limpiamos la estructura 'ack' para que no tenga basura de la RAM
                     protocolo ack; 
                     memset(&ack, 0, sizeof(ack)); // <--- IMPORTANTE
                     
                     ack.cmd = 1; 
                     ack.lng = 2;
                     
                     // Asignación MANUAL byte a byte (más segura que memcpy para esto)
                     // Copiamos los bytes del ID exactamente como llegaron
                     ack.data[0] = loraRxBuffer[4]; // Byte bajo del ID original
                     ack.data[1] = loraRxBuffer[5]; // Byte alto del ID original
                     
                     enviarPaqueteLoRaProfesor(1, rx_src, 0, 0, ack);
                     
                     Serial.println("[DEBUG] ACK Enviado correctamente.");
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