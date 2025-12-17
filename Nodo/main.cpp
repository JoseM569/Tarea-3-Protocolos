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
#include <unistd.h> 
#include <fstream>  
#include <ctime>    

// GPIO 17 (Pin Físico 11) para el LED
#define LED_PIN 17 

int fd_serial = -1;

void printTiempo() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "[%H:%M:%S] ", &tstruct);
    printf("%s", buf);
}

void guardarLog(uint16_t origen, const char* mensaje) {
    std::ofstream archivo;
    archivo.open("log.txt", std::ios::app);
    if (archivo.is_open()) {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
        archivo << "[" << buf << "] De Nodo " << origen << ": " << mensaje << "\n";
        archivo.close();
    }
}

int esperarEvento() {
    struct timeval tv; fd_set fds; tv.tv_sec = 0; tv.tv_usec = 100000; 
    FD_ZERO(&fds); FD_SET(STDIN_FILENO, &fds);
    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) return 1; 
    if (serialDataAvail(fd_serial) > 0) return 2; 
    return 0;
}

int main() {
    // 1. SETUP
    if (wiringPiSetupGpio() == -1) return 1; 
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); 

    // OJO: USB0 o USB1 según corresponda
    if ((fd_serial = serialOpen("/dev/ttyUSB1", 115200)) < 0) {
        printf("ERROR: Serial.\n"); return 1;
    }
    
    printf("NODO RPi (ID %d) - LISTO\n", MY_IP_ADDR);
    for (int i=0; menu[i] != NULL; i++) puts(menu[i]);
    printf("\n> "); fflush(stdout);

    for (;;) {
        int evento = esperarEvento();

        // --- RECEPCIÓN (RX) ---
        if (evento == 2) {
            packet_ipv4 rxPacket;
            if (recibirFrameIPv4(rxPacket)) { 
                
                registrarVecinoSilencioso(rxPacket.srcIP);

                // --- PASO 1: DECIDIR SI NECESITA ACK ---
                // Solo confirmamos Mensajes (2) y Comandos (5, 6, 7).
                // ¡IMPORTANTE! Excluimos la Imagen (9) para no saturar.
                bool necesitaACK = false;
                if (rxPacket.protocol == 2 || 
                   (rxPacket.protocol >= 5 && rxPacket.protocol != 9)) {
                    necesitaACK = true;
                }

                // --- PASO 2: ENVIAR ACK (SI CORRESPONDE) ---
                if (necesitaACK) {
                     usleep(50000); // 50ms espera técnica
                     
                     protocolo ack;
                     ack.cmd = 1; ack.lng = 2; 
                     ack.data[0] = (rxPacket.id >> 8) & 0xFF;
                     ack.data[1] = rxPacket.id & 0xFF;
                     enviarFrameIPv4(1, rxPacket.srcIP, 0, 0, ack);
                     
                     // Debug visual mínimo
                     // printf(" (ACK Enviado) ");
                }

                // --- PASO 3: EJECUTAR ACCIÓN (OBLIGATORIO) ---
                // Usamos Switch para ordenar y asegurar que el código entre.
                
                printf("\r\033[K"); // Limpiar línea visual
                
                switch (rxPacket.protocol) {
                    
                    // CASO: Confirmación recibida (ACK)
                    case 1:
                        printTiempo();
                        printf("? [ACK] ID %d confirmado.\n", 
                           (rxPacket.payload.data[0] << 8) | rxPacket.payload.data[1]);
                        break;

                    // CASO: Mensaje de Texto
                    case 2:
                        printTiempo();
                        printf("?? [RX] Mensaje: %s\n", rxPacket.payload.data);
                        guardarLog(rxPacket.srcIP, (char*)rxPacket.payload.data);
                        break;

                    // CASO: Broadcast
                    case 3:
                        printTiempo();
                        printf("?? [BROADCAST]: %s\n", rxPacket.payload.data);
                        break;

                    // CASO: Hello
                    case 4:
                        // Silencioso (ya registrado arriba)
                        break;

                    // CASO: Comando TEST
                    case 5:
                        printTiempo();
                        printf("?? CMD: TEST recibido.\n");
                        
                        // Esperamos 1 SEGUNDO completo para asegurar que el ACK
                        // haya salido del aire antes de mandar la respuesta.
                        sleep(1); 
                        
                        { // Bloque para variables locales
                            printf("   -> Enviando respuesta de datos...\n");
                            protocolo p; p.cmd = 2; p.lng = 13; 
                            strcpy((char*)p.data, "TEST_RESPONSE");
                            enviarFrameIPv4(0, MY_IP_ADDR, 0, 0, p);
                        }
                        break;

                    // CASO: Comando LED
                    case 6:
                        printTiempo();
                        printf("?? CMD: LED TOGGLE... ");
                        {
                            int estado = digitalRead(LED_PIN);
                            digitalWrite(LED_PIN, !estado);
                            printf("Hecho. (Ahora: %s)\n", (!estado) ? "ON" : "OFF");
                        }
                        break;

                    // CASO: Comando OLED
                    case 7:
                        printTiempo();
                        printf("??? CMD: OLED -> '%s'\n", rxPacket.payload.data);
                        // Aquí pones tu código real de OLED
                        break;

                    // CASO: IMAGEN (Sin ACK)
                    case 9:
                        // No imprimimos hora para no ensuciar tanto
                        // Solo un punto para mostrar actividad
                        printf("."); 
                        fflush(stdout);
                        break;

                    default:
                        printf("?? Protocolo desconocido: %d\n", rxPacket.protocol);
                }
                
                // Restaurar prompt si no es imagen masiva
                if (rxPacket.protocol != 9) {
                    printf("> "); fflush(stdout);
                }
            }
        }

        // --- TECLADO ---
        if (evento == 1) {
            std::string line;
            if (std::getline(std::cin, line) && !line.empty()) {
                long opt = atol(line.c_str());
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