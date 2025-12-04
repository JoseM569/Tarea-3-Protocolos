#include "funcionesReceptor.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, 16);
bool ledState = false;

void setupHardware() {
    pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, LOW);
    Wire.begin(4, 15); 
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        for(;;); // Loop infinito si falla OLED
    }
    display.clearDisplay(); display.display();
}

void mostrarMensajeBienvenidaOLED() {
    display.clearDisplay(); display.setCursor(0,0); display.setTextSize(1);
    display.setTextColor(WHITE);
    display.printf("KIT ID: %d\nModo: MODEM\nListo.", MY_IP_ADDR);
    display.display();
}

void mostrarEnrutamientoOLED(uint16_t src, uint16_t dest, uint8_t proto) {
    display.clearDisplay(); display.setCursor(0,0);
    display.println("ENRUTANDO A LORA...");
    display.printf("De: %d\nPara: %d\nProto: %d", src, dest, proto);
    display.display();
    delay(2000); 
    mostrarMensajeBienvenidaOLED();
}

// --- NUEVA FUNCIÓN CORREGIDA ---
void mostrarRecepcionLoRa(int n) {
    display.clearDisplay(); display.setCursor(0,0);
    display.println("RX LORA -> UART");
    display.printf("Bytes: %d", n);
    display.display();
    delay(1000);
    mostrarMensajeBienvenidaOLED();
}

void ejecutarComando(protocolo& proto) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);

    // --- CASO 1: LED (CMD 4) ---
    if (proto.cmd == 4) { 
        ledState = !ledState; 
        digitalWrite(LED_PIN, ledState);
        
        display.println("CMD LOCAL: LED");
        display.println("----------------");
        display.printf("Estado: %s", ledState ? "ENCENDIDO" : "APAGADO");
    }
    // --- CASO 2: MENSAJE DE TEXTO (CMD 2) --- (¡Esto faltaba!)
    else if (proto.cmd == 2) {
        display.println("CMD LOCAL: TEXTO");
        display.println("----------------");
        
        // Asegurar que el texto tenga fin de cadena
        proto.data[proto.lng] = '\0'; 
        display.println((char*)proto.data);
    }
    // --- Otros casos ---
    else {
        display.printf("CMD Desconocido: %d", proto.cmd);
    }

    display.display();
}

void manejarParpadeoLED() {}