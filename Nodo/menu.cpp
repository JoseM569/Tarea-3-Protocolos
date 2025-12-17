#include "menu.h"
#include "capaRed.h"
#include "structProtocolo.h"
#include <iostream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <wiringPi.h> 

#define PROTO_IMAGEN 9 

std::vector<Neighbor> neighborTable;

const char* menu[] = {
    "====================================",
    "      MENU NODO (Raspberry Pi)      ",
    "====================================",
    "1. Ver Nodos Disponibles",
    "2. Enviar Hello (Broadcast)",
    "3. Enviar Mensaje Unicast (con ACK)",
    "4. Enviar Mensaje Broadcast",
    "5. Comandos Remotos (con ACK)",
    "6. Enviar Imagen (Desafio)",
    "7. Comandos Internos (Tarea 1 y 2)",
    "8. Cambiar ID Destino (Config)",
    "0. Salir",
    NULL
};

uint16_t dest_ip_global = 0; 

void registrarVecinoSilencioso(uint16_t id) {
    if (id == MY_IP_ADDR || id == BROADCAST_IP || id == 0) return;
    bool found = false;
    for (size_t i = 0; i < neighborTable.size(); i++) {
        if (neighborTable[i].id == id) {
            neighborTable[i].lastSeen = time(NULL);
            found = true;
            break;
        }
    }
    if (!found) {
        Neighbor newN; newN.id = id; newN.lastSeen = time(NULL);
        neighborTable.push_back(newN);
        printf("\n?? [NUEVO VECINO DETECTADO]: ID %d guardado en la tabla.\n> ", id);
        fflush(stdout);
    }
}

// --- FUNCIÓN DEBUG: VER QUÉ LLEGA ---
// --- FUNCIÓN CORREGIDA: TIMEOUT DE 15 SEGUNDOS ---
void enviarConConfirmacion(uint8_t proto, uint16_t dest, protocolo p) {
    int intentos = 0;
    bool exito = false;

    while (intentos < 3 && !exito) {
        if (intentos > 0) {
            printf("?? Reintentando (%d/2)...\n", intentos);
            // Si falló, damos un respiro largo para que lleguen los ACKs viejos y mueran
            sleep(3); 
        }

        purgarBuffer(); 

        uint16_t idEnviado = enviarFrameIPv4(proto, dest, 0, 0, p);
        
        printf("Esperando ACK (ID Pkt: %d)...", idEnviado); fflush(stdout);
        
        uint32_t inicio = millis();
        
        // CAMBIO CRÍTICO: 15000 ms (15 Segundos)
        // Tu red tiene picos de latencia de 12s, así que 15s es lo seguro.
        while (millis() - inicio < 15000) { 
            packet_ipv4 rx;
            if (recibirFrameIPv4(rx)) {
                
                registrarVecinoSilencioso(rx.srcIP);
                
                uint16_t idConf = (rx.payload.data[0] << 8) | rx.payload.data[1];
                
                // Mantenemos el Debug para ver el éxito
                printf("\n?? [DEBUG] Recibido: Proto=%d | De=%d | PayloadID=%d", rx.protocol, rx.srcIP, idConf);

                if (rx.protocol == 1) { 
                    if (rx.srcIP == dest && idConf == idEnviado) {
                        printf(" ? ¡MATCH! Confirmado.\n");
                        exito = true;
                        break; 
                    } else {
                        // Si llega un ID viejo, lo ignoramos y SEGUIMOS esperando
                        // No salimos del while, solo avisamos
                        printf(" ?? (Ignorando ACK viejo/desfasado)\n");
                    }
                }
            }
            usleep(1000); 
        }
        
        if (!exito) printf("\n"); 
        intentos++;
    }

    if (!exito) {
        printf("? ERROR: Timeout extremo con destino %d.\n", dest);
    }
}
// --- OPCIONES DEL MENÚ ---

void opcion_1_ver_vecinos() {
    printf("\n--- NODOS DESCUBIERTOS ---\nID\tHace (seg)\n");
    time_t now = time(NULL);
    for (size_t i = 0; i < neighborTable.size(); i++) 
        printf("%d\t%ld s\n", neighborTable[i].id, (long)(now - neighborTable[i].lastSeen));
    printf("--------------------------\n");
}

void opcion_2_hello() {
    protocolo p; p.cmd = 1; p.lng = 4; strcpy((char*)p.data, "hola");
    enviarFrameIPv4(4, 65535, 0, 0, p);
    printf("Mensaje HELLO enviado.\n");
}

void opcion_3_unicast() {
    printf("ID Destino: "); int d; std::cin >> d;
    std::string msg; 
    printf("Mensaje: "); 
    std::cin >> std::ws; 
    std::getline(std::cin, msg);
    
    protocolo p; p.cmd = 1; p.lng = msg.length();
    if(p.lng > 60) p.lng = 60; strncpy((char*)p.data, msg.c_str(), p.lng);
    enviarConConfirmacion(2, (uint16_t)d, p);
}

void opcion_4_broadcast() {
    std::string msg; printf("Broadcast: "); std::cin >> std::ws; std::getline(std::cin, msg);
    protocolo p; p.cmd = 1; p.lng = msg.length(); strncpy((char*)p.data, msg.c_str(), p.lng);
    enviarFrameIPv4(3, 65535, 0, 0, p);
    printf("Enviado Broadcast.\n");
}

// --- AQUÍ ESTABA EL ERROR, YA CORREGIDO ---
void opcion_5_remotos() {
    printf("ID Nodo: "); int d; std::cin >> d;
    printf("1.Test 2.LED 3.OLED\n> "); int op; std::cin >> op;
    
    protocolo p; 
    uint8_t protoID = 0;
    
    if (op == 1) { 
        protoID = 5; p.cmd = 0; p.lng = 0; 
    } 
    else if (op == 2) { 
        protoID = 6; p.cmd = 0; p.lng = 0; 
    } 
    else if (op == 3) {
        protoID = 7; 
        printf("Texto: "); 
        // Primero declaramos la variable
        std::string t; 
        // Luego leemos
        std::cin >> std::ws; 
        std::getline(std::cin, t);
        
        p.cmd = 0; 
        p.lng = t.length(); 
        strncpy((char*)p.data, t.c_str(), p.lng);
    }
    enviarConConfirmacion(protoID, (uint16_t)d, p);
}

void opcion_6_imagen() {
    printf("Destino: "); int d; std::cin >> d;
    uint8_t imagen[1024]; for(int i=0; i<1024; i++) imagen[i] = (uint8_t)(i % 255);
    int max=60, total=1024, offset=0;
    printf("Enviando...\n");
    while(offset < total) {
        protocolo p; p.cmd = 0;
        int chunk = total - offset;
        if(chunk > max) chunk = max; p.lng = chunk;
        for(int k=0; k<chunk; k++) p.data[k] = imagen[offset + k];
        int flag = (offset + chunk >= total) ? 2 : 1;
        enviarFrameIPv4(PROTO_IMAGEN, (uint16_t)d, flag, offset, p);
        offset += chunk; usleep(100000); 
    }
    printf("Imagen enviada.\n");
}

void opcion_7_legacy() {
    printf("1.LED 2.Texto\n> "); int op; std::cin >> op;
    protocolo p;
    if (op == 1) { p.cmd = 4; p.lng = 0; } 
    else {
        printf("Texto: "); 
        std::string t; 
        std::cin >> std::ws; 
        std::getline(std::cin, t);
        p.cmd = 2; p.lng = t.length(); strncpy((char*)p.data, t.c_str(), p.lng);
    }
    enviarFrameIPv4(0, MY_IP_ADDR, 0, 0, p);
}

void opcion_8_cambiar_ip() {
    printf("ID global: "); int id; std::cin >> id; dest_ip_global = id;
}