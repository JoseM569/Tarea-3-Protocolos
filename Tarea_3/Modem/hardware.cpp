#include "hardware.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, 16);
bool ledState = false;

void setupHardware() {
    pinMode(LED_PIN, OUTPUT); 
    digitalWrite(LED_PIN, LOW);
    
    Wire.begin(4, 15); // Pines SDA, SCL para Heltec LoRa 32 V2
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        // Si falla la pantalla, loop infinito
        for(;;); 
    }
    display.clearDisplay(); 
    display.display();
}

void mostrarMensajeBienvenidaOLED() {
    display.clearDisplay(); 
    display.setCursor(0,0); 
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.printf("MODEM LORA\nID: %d\nModo: GATEWAY\nListo.", MY_IP_ADDR);
    display.display();
}

void mostrarEnrutamientoOLED(uint16_t src, uint16_t dest, uint8_t proto) {
    display.clearDisplay(); 
    display.setCursor(0,0);
    display.println("ENRUTANDO A LORA...");
    display.printf("De: %d\nPara: %d\nProto: %d", src, dest, proto);
    display.display();
    delay(3000); // Pequeña pausa para poder leer
    mostrarMensajeBienvenidaOLED();
}

void mostrarRecepcionLoRa(int n) {
    display.clearDisplay(); 
    display.setCursor(0,0);
    display.println("RX LORA -> UART");
    display.printf("Bytes: %d", n);
    display.display();
    delay(3000);
    mostrarMensajeBienvenidaOLED();
}

// --- Lógica de Actuadores Locales ---
void ejecutarComando(protocolo& proto) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);

    // CMD 4: Control LED
    if (proto.cmd == 4) { 
        ledState = !ledState; 
        digitalWrite(LED_PIN, ledState);
        
        display.println("CMD LOCAL: LED");
        display.println("----------------");
        display.printf("Estado: %s", ledState ? "ON" : "OFF");
    }
    // CMD 2: Mostrar Texto
    else if (proto.cmd == 2) {
        display.println("CMD LOCAL: TEXTO");
        display.println("----------------");
        
        // Asegurar null-termination para imprimir string seguro
        char msgBuffer[65];
        int len = proto.lng;
        if(len > 64) len = 64;
        memcpy(msgBuffer, proto.data, len);
        msgBuffer[len] = '\0';
        
        display.println(msgBuffer);
    }
    else {
        display.printf("CMD Desconocido: %d", proto.cmd);
    }

    display.display();
}

void manejarParpadeoLED() {
    // No utilizada en Tarea 3 (Control directo)
}