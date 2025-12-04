#include "funcionesMenu.h"    
#include "funcionesProtocolo.h"  
#include <wiringPi.h>         
#include <wiringSerial.h>     
#include <iostream>           
#include <string>             
#include <sys/select.h> // Necesario para select()

int fd_serial = -1;

// Función que revisa si hay datos sin bloquear
// Retorna: 1 si hay Teclado, 2 si hay Serial, 0 si no hay nada
int esperarEvento() {
    struct timeval tv;
    fd_set fds;

    tv.tv_sec = 0;
    tv.tv_usec = 100000; // 100ms timeout

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); // Escuchar Teclado (ID 0)

    // select() espera algo en STDIN. 
    // Para el serial wiringPi, usamos su propia función serialDataAvail
    // así que aquí solo usamos select para el teclado de forma no bloqueante o con timeout
    
    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    
    if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
        return 1; // Hay tecla pulsada
    }
    
    if (serialDataAvail(fd_serial) > 0) {
        return 2; // Hay datos en el cable USB
    }
    
    return 0;
}

int main() {
    if (wiringPiSetupGpio() == -1) return 1; 
    
    // IMPORTANTE: Asegúrate de que este puerto sea el correcto (ttyUSB0 o ttyACM0)
    if ((fd_serial = serialOpen("/dev/ttyUSB0", 115200)) < 0) {
        printf("ERROR: No se pudo abrir puerto serial.\n"); return 1;
    }
    
    printf("NODO RPi (ID %d) - LISTO\n", MY_IP_ADDR);
    
    // Imprimir menú una vez al inicio
    for (size_t i=0; i<9; ++i) puts(menu[i]);
    printf("\nEsperando comando o mensaje... (Escribe opción)\n> ");
    fflush(stdout);

    for (;;) {
        int evento = esperarEvento();

        // CASO 1: LLEGÓ MENSAJE POR USB (Radio o Modem)
        if (evento == 2) {
            packet_ipv4 rxPacket;
            if (recibirFrameIPv4(rxPacket)) {
                // Limpiar línea actual para que se vea bonito
                printf("\r\033[K"); 
                
                // Mostrar Mensaje Recibido
                printf("📩 [RX] De: %d | Proto: %d | Data: %s\n", 
                       rxPacket.srcIP, rxPacket.protocol, rxPacket.payload.data);
                
                // Volver a poner el prompt
                printf("> "); 
                fflush(stdout);
            }
        }

        // CASO 2: EL USUARIO ESCRIBIÓ ALGO
        if (evento == 1) {
            std::string line;
            if (std::getline(std::cin, line) && !line.empty()) {
                long opt = 0;
                try { opt = std::stol(line); } catch (...) { continue; }
                
                if (opt == 0) break;

                // Ejecutar Opción
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
                
                // Volver a mostrar menú resumido
                printf("\n> ");
                fflush(stdout);
            }
        }
    }
    serialClose(fd_serial);
    return 0; 
}