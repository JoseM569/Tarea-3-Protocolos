#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <time.h>

extern const char* menu[];

struct Neighbor {
    uint16_t id;
    time_t lastSeen;
};

void registrarVecinoSilencioso(uint16_t id); // <--- NUEVA

void opcion_1_ver_vecinos();
void opcion_2_hello();
void opcion_3_unicast();
void opcion_4_broadcast();
void opcion_5_remotos();
void opcion_6_imagen();
void opcion_7_legacy();
void opcion_8_cambiar_ip();

#endif