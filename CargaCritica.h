#pragma once

#include "Carga.h"

// ============================================================================
// CLASE DERIVADA: CargaCritica (Herencia de Carga)
// ============================================================================
// REGLA 1: La refrigeracion nunca se apaga por precio, hora ni nivel de embalses.
class CargaCritica : public Carga {
public:
    CargaCritica(std::string n, float p) : Carga(n, p) {}

    bool debeEncender(int hora, float precio, float embalses) override {
        (void)hora;
        (void)precio;
        (void)embalses;
        return true; // Conservacion continua de la cadena de frio
    }
};
