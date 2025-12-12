#include "menu.h"    
#include "capaRed.h"  
#include "structProtocolo.h"
#include <wiringPi.h>         
#include <wiringSerial.h>     
#include <iostream>           
#include <string>             
#include <sys/select.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <fstream>  // <--- Nuevo: Para escribir archivos
#include <ctime>    // <--- Nuevo: Para la fecha y hora

int fd_serial = -1;

// --- NUEVA FUNCIÓN: GUARDAR LOG ---
// Escribe en "log.txt" cada mensaje recibido con fecha y hora.
void guardarLog(uint16_t origen, const char* mensaje) {
    std::ofstream archivo;
    // std::ios::app significa "Append" (agregar al final sin borrar lo anterior)
    archivo.open("log.txt", std::ios::app);
    
    if (archivo.is_open()) {
        // Obtener fecha y hora actual
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        // Formato: YYYY-MM-DD HH:MM:SS
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
        
        // Escribir en el archivo
        archivo << "[" << buf << "] De Nodo " << origen << ": " << mensaje << "\n";
        
        archivo.close();
        printf(" 💾 (Guardado en log.txt)\n");
    } else {
        printf(" ⚠️ Error al escribir en log.txt\n");
    }
}

int esperarEvento() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0; tv.tv_usec = 100000; 

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    
    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    
    if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) return 1; 
    if (serialDataAvail(fd_serial) > 0) return 2; 
    
    return 0;
}

int main() {
    if (wiringPiSetupGpio() == -1) return 1; 
    
    // REVISA SI NECESITAS CAMBIAR EL PUERTO (ttyUSB0, ttyUSB1, etc)
    if ((fd_serial = serialOpen("/dev/ttyUSB0", 115200)) < 0) {
        printf("ERROR: No se pudo abrir puerto serial.\n"); return 1;
    }
    
    printf("NODO RPi (ID %d) - LISTO\n", MY_IP_ADDR);
    
    for (int i=0; menu[i] != NULL; i++) puts(menu[i]);
    
    printf("\n> "); fflush(stdout);

    for (;;) {
        int evento = esperarEvento();

        // --- CASO 1: LLEGÓ MENSAJE ---
        if (evento == 2) {
            packet_ipv4 rxPacket;
            if (recibirFrameIPv4(rxPacket)) { 
                
                if (rxPacket.protocol == 4) {
                    registrarVecinoSilencioso(rxPacket.srcIP);
                } 
                else if (rxPacket.protocol == 1) {
                    printf("✅ [ACK] Confirmación recibida de ID %d\n> ", 
                           (rxPacket.payload.data[0] << 8) | rxPacket.payload.data[1]);
                    fflush(stdout);
                }
                else {
                    printf("\r\033[K"); 
                    printf("📩 [RX] De: %d | ", rxPacket.srcIP);

                    if (rxPacket.protocol == 2) {
                        // UNICAST: Mostramos y Guardamos LOG
                        printf("Mensaje: %s", rxPacket.payload.data);
                        
                        // LLAMADA A LA FUNCIÓN DE LOG (Punto Extra)
                        guardarLog(rxPacket.srcIP, (char*)rxPacket.payload.data);
                    }
                    else if (rxPacket.protocol == 3) {
                        printf("Mensaje Broadcast: %s\n", rxPacket.payload.data);
                    }
                    else if (rxPacket.protocol == 5) {
                        printf("Comando: TEST DE PANTALLA (Ejecutando...)\n");
                        protocolo p; p.cmd = 2; p.lng = 5; strcpy((char*)p.data, "TEST");
                        enviarFrameIPv4(0, MY_IP_ADDR, 0, 0, p); 
                    }
                    else if (rxPacket.protocol == 6) {
                        printf("Comando: CAMBIAR LED (Ejecutando...)\n");
                        protocolo p; p.cmd = 4; p.lng = 0; 
                        enviarFrameIPv4(0, MY_IP_ADDR, 0, 0, p);
                    }
                    else if (rxPacket.protocol == 7) {
                        printf("Comando: MENSAJE OLED (Ejecutando...)\n");
                        protocolo p; p.cmd = 2; p.lng = rxPacket.payload.lng;
                        memcpy(p.data, rxPacket.payload.data, p.lng);
                        enviarFrameIPv4(0, MY_IP_ADDR, 0, 0, p);
                    }
                    
                    // --- ENVIAR ACK DE RESPUESTA ---
                    if (rxPacket.protocol == 2 || rxPacket.protocol >= 5) {
                         protocolo ack;
                         ack.cmd = 1; 
                         ack.lng = 2; 
                         ack.data[0] = (rxPacket.id >> 8) & 0xFF;
                         ack.data[1] = rxPacket.id & 0xFF;
                         enviarFrameIPv4(1, rxPacket.srcIP, 0, 0, ack);
                    }
                    
                    printf("> "); fflush(stdout);
                }
            }
        }

        // --- CASO 2: TECLADO ---
        if (evento == 1) {
            std::string line;
            if (std::getline(std::cin, line) && !line.empty()) {
                long opt = 0;
                opt = atol(line.c_str());
                
                if (opt == 0) break;

                switch ((int)opt) {
                    case 1: opcion_1_ver_vecinos(); break;
                    case 2: opcion_2_hello(); break;
                    case 3: opcion_3_unicast(); break;
                    case 4: opcion_4_broadcast(); break;
                    case 5: opcion_5_remotos(); break;
                    case 6: opcion_6_imagen(); break;
                    case 7: opcion_7_legacy(); break;
                    case 8: opcion_8_cambiar_ip(); break;
                }
                printf("\n> "); fflush(stdout);
            }
        }
    }
    serialClose(fd_serial);
    return 0; 
}