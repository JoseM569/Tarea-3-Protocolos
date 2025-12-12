#include "menu.h"
#include "capaRed.h"
#include "structProtocolo.h"
#include <iostream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

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
    bool found = false;
    for (size_t i = 0; i < neighborTable.size(); i++) {
        if (neighborTable[i].id == id) {
            neighborTable[i].lastSeen = time(NULL);
            found = true;
            break;
        }
    }
    if (!found) {
        Neighbor newN;
        newN.id = id;
        newN.lastSeen = time(NULL);
        neighborTable.push_back(newN);
    }
}

// --- LÓGICA DE REINTENTOS (ACK) ---
void enviarConConfirmacion(uint8_t proto, uint16_t dest, protocolo p) {
    int intentos = 0;
    bool exito = false;

    // Intentar hasta 3 veces (Original + 2 Reintentos)
    while (intentos < 3 && !exito) {
        if (intentos > 0) printf("⚠️ Sin respuesta. Reintentando (%d/2)...\n", intentos);
        
        // 1. Enviar y guardar ID
        uint16_t idEnviado = enviarFrameIPv4(proto, dest, 0, 0, p);
        
        // 2. Esperar ACK (2 segundos)
        printf("Esperando ACK..."); fflush(stdout);
        time_t inicio = time(NULL);
        
        while (time(NULL) - inicio < 2) { 
            packet_ipv4 rx;
            if (recibirFrameIPv4(rx)) {
                // Si llega Hello, guardar
                if (rx.protocol == 4) {
                    registrarVecinoSilencioso(rx.srcIP);
                }
                // Si llega ACK (Proto 1)
                else if (rx.protocol == 1) {
                    uint16_t idConfirmado = (rx.payload.data[0] << 8) | rx.payload.data[1];
                    // Verificar que sea de quien esperamos y el ID correcto
                    if (rx.srcIP == dest && idConfirmado == idEnviado) {
                        printf(" ✅ ¡Confirmado!\n");
                        exito = true;
                        break; 
                    }
                }
            }
            usleep(10000); 
        }
        
        if (!exito) printf("\n"); 
        intentos++;
    }

    if (!exito) {
        printf("❌ ERROR: El destino %d no responde después de 3 intentos.\n", dest);
    }
}

void opcion_1_ver_vecinos() {
    printf("\n--- NODOS DESCUBIERTOS ---\n");
    printf("ID\tHace (seg)\n");
    time_t now = time(NULL);
    for (size_t i = 0; i < neighborTable.size(); i++) {
        printf("%d\t%ld s\n", neighborTable[i].id, (long)(now - neighborTable[i].lastSeen));
    }
    printf("--------------------------\n");
}

void opcion_2_hello() {
    protocolo p;
    p.cmd = 1;
    p.lng = 4;
    strcpy((char*)p.data, "hola");
    enviarFrameIPv4(4, 65535, 0, 0, p);
    printf("Mensaje HELLO enviado a Broadcast.\n");
}

void opcion_3_unicast() {
    printf("Ingrese ID Destino: ");
    int d; std::cin >> d;
    std::string msg;
    printf("Mensaje: ");
    std::cin.ignore(); 
    std::getline(std::cin, msg);
    
    protocolo p;
    p.cmd = 1; 
    p.lng = msg.length();
    if(p.lng > 60) p.lng = 60;
    strncpy((char*)p.data, msg.c_str(), p.lng);
    
    // USAMOS LA NUEVA FUNCIÓN CON ACK
    enviarConConfirmacion(2, (uint16_t)d, p);
}

void opcion_4_broadcast() {
    std::string msg;
    printf("Mensaje Broadcast: ");
    std::cin.ignore(); 
    std::getline(std::cin, msg);
    protocolo p;
    p.cmd = 1;
    p.lng = msg.length();
    if(p.lng > 60) p.lng = 60;
    strncpy((char*)p.data, msg.c_str(), p.lng);
    enviarFrameIPv4(3, 65535, 0, 0, p);
    printf("Enviado Broadcast.\n");
}

void opcion_5_remotos() {
    printf("Ingrese ID Nodo Remoto: ");
    int d; std::cin >> d;
    
    printf("1. Test Pantalla\n2. Toggle LED\n3. Mensaje OLED\n> ");
    int op; std::cin >> op;
    
    protocolo p;
    uint8_t protoID = 0;
    
    if (op == 1) {
        protoID = 5; // Prueba
        p.cmd = 0; p.lng = 0; 
    } else if (op == 2) {
        protoID = 6; // Estado LED
        p.cmd = 0; p.lng = 0; 
    } else if (op == 3) {
        protoID = 7; // Mensaje OLED
        printf("Texto: ");
        std::string t; std::cin.ignore(); std::getline(std::cin, t);
        p.cmd = 0; p.lng = t.length();
        strncpy((char*)p.data, t.c_str(), p.lng);
    }
    
    // USAMOS LA NUEVA FUNCIÓN CON ACK
    enviarConConfirmacion(protoID, (uint16_t)d, p);
}

void opcion_6_imagen() {
    printf("Ingrese ID Destino para imagen: ");
    int d; std::cin >> d;
    
    uint8_t imagen[1024];
    for(int i=0; i<1024; i++) imagen[i] = (uint8_t)(i % 255);
    
    int maxDataLen = 60; 
    int totalLen = 1024;
    int offset = 0;
    
    printf("Enviando imagen fragmentada...\n");
    
    while(offset < totalLen) {
        protocolo p;
        p.cmd = 0;
        int chunk = totalLen - offset;
        if(chunk > maxDataLen) chunk = maxDataLen;
        
        p.lng = chunk;
        for(int k=0; k<chunk; k++) p.data[k] = imagen[offset + k];
        
        int flag = 1; 
        if (offset + chunk >= totalLen) {
            flag = 2; 
        }
        
        enviarFrameIPv4(PROTO_IMAGEN, (uint16_t)d, flag, offset, p);
        
        offset += chunk;
        usleep(100000); 
    }
    printf("Imagen enviada.\n");
}

void opcion_7_legacy() {
    printf("1. LED Local\n2. Texto Local\n> ");
    int op; std::cin >> op;
    protocolo p;
    
    if (op == 1) {
        p.cmd = 4; p.lng = 0; 
    } else {
        printf("Texto: ");
        std::string t; std::cin.ignore(); std::getline(std::cin, t);
        p.cmd = 2; p.lng = t.length(); 
        strncpy((char*)p.data, t.c_str(), p.lng);
    }
    enviarFrameIPv4(0, MY_IP_ADDR, 0, 0, p);
    printf("Comando local enviado.\n");
}

void opcion_8_cambiar_ip() {
    printf("Ingrese ID global: ");
    int id; std::cin >> id;
    dest_ip_global = id;
}