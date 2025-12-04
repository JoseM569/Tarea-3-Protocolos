#include "recibe.h"
#include "funcionesReceptor.h"

// Nota: Asegúrate de tener LoRa.cpp, LoRa.h, red.cpp y red.h en la carpeta
protocolo rx_proto;

void setup() {
    setupHardware();
    mostrarMensajeBienvenidaOLED();
    setupRecepcion(); // Inicia Serial y LoRa
}

void loop() {
    // 1. Atender Raspberry (TX)
    revisarRecepcionUART();
    
    // 2. Atender Radio LoRa (RX)
    revisarRecepcionLoRa();
    
    // 3. Tareas locales
    manejarParpadeoLED();
}