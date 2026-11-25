#pragma once

#include "Carga.h"
#include "configuracion.h"

// ============================================================================
// CLASE DERIVADA: CargaAplazable (Herencia de Carga)
// ============================================================================
// REGLAS 2, 3, 4 y 5: Aplazamiento compuesto ante pico, precio caro o embalses bajos.
// COORDINACION DE PRODUCCION: Al cumplir sus horas minimas, cede el turno para evitar sobrecargas.
class CargaAplazable : public Carga {
private:
    int horasMinimas;

public:
    CargaAplazable(std::string n, float p, int hMin) : Carga(n, p), horasMinimas(hMin) {}

    int getHorasMinimas() const { return horasMinimas; }

    bool debeEncender(int hora, float precio, float embalses) override {
        bool esPico = (hora >= PICO_INICIO && hora <= PICO_FIN);
        bool esPrecioCaro = (precio >= PRECIO_CARO);
        bool esEmbalseCritico = (embalses < UMBRAL_EMBALSES_CRITICO);

        if (esPico || esPrecioCaro || esEmbalseCritico) {
            return false;
        }

        // Si ya completo su cuota requerida en el dia, cede potencia
        if (horasMinimas > 0 && horasUso >= horasMinimas) {
            return false;
        }

        return true;
    }
};
