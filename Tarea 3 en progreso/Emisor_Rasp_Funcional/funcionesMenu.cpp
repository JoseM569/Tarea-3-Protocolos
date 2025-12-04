#include "funcionesMenu.h"
#include "funcionesProtocolo.h"
#include <iostream>
#include <vector>
#include <ctime>
#include <string.h>
#include <unistd.h>

unsigned short DEST_IP = BROADCAST_IP;
protocolo tx;
uint8_t IMAGEN_BITMAP[1024]; // Dummy buffer

struct Neighbor { uint16_t id; time_t lastSeen; };
std::vector<Neighbor> neighborTable;

const char* menu[10] = {
    "===== MENÚ NODO (ID 8) =====",
    "1) Ver Nodos", "2) Enviar HELLO", "3) Mensaje Unicast", 
    "4) Mensaje Broadcast", "5) Comandos Remotos", 
    "6) Imagen Fragmentada", "7) Comandos Locales (Legacy)", 
    "8) CAMBIAR ID DESTINO", "0) Salir"
};

void initImagen() { for(int i=0; i<1024; i++) IMAGEN_BITMAP[i] = (i%2==0)?0xFF:0x00; }

void opcion_1_ver_vecinos() {
    printf("--- Tabla de Vecinos ---\n");
    for (const auto& n : neighborTable) printf("%d\t%ld seg\n", n.id, time(NULL)-n.lastSeen);
}
void opcion_2_hello() {
    char msg[]="hola"; memset(&tx,0,sizeof(tx)); memcpy(tx.data,msg,4); tx.lng=4;
    enviarFrameIPv4(PROTO_HELLO, BROADCAST_IP, 0, 0, tx);
    printf("HELLO enviado.\n");
}
void opcion_3_unicast() {
    printf("Msg para %d: ", DEST_IP); std::string s; std::getline(std::cin, s);
    memset(&tx,0,sizeof(tx)); strncpy((char*)tx.data,s.c_str(),63); tx.lng=s.length();
    enviarFrameIPv4(PROTO_UNICAST, DEST_IP, 0, 0, tx);
}
void opcion_4_broadcast() {
    printf("Broadcast: "); std::string s; std::getline(std::cin, s);
    memset(&tx,0,sizeof(tx)); strncpy((char*)tx.data,s.c_str(),63); tx.lng=s.length();
    enviarFrameIPv4(PROTO_BCAST, BROADCAST_IP, 0, 0, tx);
}
void opcion_5_remotos() {
    printf("1) LED 2) OLED: "); int s; std::cin>>s; std::cin.ignore(); memset(&tx,0,sizeof(tx));
    if(s==1) enviarFrameIPv4(PROTO_LED, DEST_IP, 0, 0, tx);
    else { printf("Txt: "); std::string t; std::getline(std::cin, t);
           strncpy((char*)tx.data,t.c_str(),63); tx.lng=t.length();
           enviarFrameIPv4(PROTO_OLED, DEST_IP, 0, 0, tx); }
}
void opcion_6_imagen() {
    initImagen(); int sent=0; printf("Enviando img a %d...\n", DEST_IP);
    while(sent<1024) {
        int len=(1024-sent>60)?60:(1024-sent);
        int flag=(sent+len<1024)?1:2;
        memset(&tx,0,sizeof(tx)); memcpy(tx.data,&IMAGEN_BITMAP[sent],len); tx.lng=len;
        enviarFrameIPv4(PROTO_LEGACY, DEST_IP, flag, sent, tx);
        sent+=len; usleep(100000);
    }
}
void opcion_7_legacy() {
    printf("--- Control Local (ID 8) ---\n1) LED 2) OLED: "); int s; std::cin>>s; std::cin.ignore();
    memset(&tx,0,sizeof(tx));
    if(s==1) { tx.cmd=4; enviarFrameIPv4(PROTO_LEGACY, MY_IP_ADDR, 0, 0, tx); }
    else { printf("Txt: "); std::string t; std::getline(std::cin, t);
           tx.cmd=2; strncpy((char*)tx.data,t.c_str(),63); tx.lng=t.length();
           enviarFrameIPv4(PROTO_LEGACY, MY_IP_ADDR, 0, 0, tx); }
}
void opcion_8_cambiar_ip() {
    printf("Nuevo ID: "); int id; std::cin>>id; std::cin.ignore(); DEST_IP=(unsigned short)id;
}