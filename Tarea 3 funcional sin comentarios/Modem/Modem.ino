#include <Arduino.h>
#include "hardware.h"
#include "enrutamiento.h"

// Archivo principal del MODEM ESP32
// Integra Hardware (OLED/LED) y Enrutamiento (LoRa/UART)

void setup() {
    setupHardware();           // Inicia OLED y LED
    mostrarMensajeBienvenidaOLED();
    setupRecepcion();          // Inicia Serial y Radio LoRa
}

void loop() {
    // 1. Revisar si la Raspberry mandó algo por cable
    revisarRecepcionUART();

    // 2. Revisar si llegó algo por la antena LoRa
    revisarRecepcionLoRa();
    
    // (Opcional) Tareas de mantenimiento
}