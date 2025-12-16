#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <time.h>

// Definición de la variable externa del menú (Array de strings)
extern const char* menu[];

struct Neighbor {
    uint16_t id;
    time_t lastSeen;
};

// Función vital para el registro de vecinos
void registrarVecinoSilencioso(uint16_t id);

// --- FUNCIONES DEL MENÚ PLANO (1-8) ---
// Estas son las que busca tu main.cpp
void opcion_1_ver_vecinos();
void opcion_2_hello();
void opcion_3_unicast();
void opcion_4_broadcast();
void opcion_5_remotos();
void opcion_6_imagen();
void opcion_7_legacy();
void opcion_8_cambiar_ip();

#endif