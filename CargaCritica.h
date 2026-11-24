#ifndef CARGACRITICA_H
#define CARGACRITICA_H

#include "Carga.h"

// ============================================================================
// CLASE DERIVADA: CargaCritica (Herencia de Carga)
// ============================================================================
// REGLA 1: La refrigeracion nunca se apaga por precio, hora ni embalses.
class CargaCritica : public Carga {
public:
    CargaCritica(std::string n, float p) : Carga(n, p) {}

    bool debeEncender(int hora, float precio, float embalses) override {
        (void)hora;
        (void)precio;
        (void)embalses;
        return true; // Siempre debe operar para preservar la cadena de frio
    }
};

#endif // CARGACRITICA_H
