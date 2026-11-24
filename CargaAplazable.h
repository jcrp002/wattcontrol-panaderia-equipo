#ifndef CARGAAPLAZABLE_H
#define CARGAAPLAZABLE_H

#include "Carga.h"
#include "Constantes.h"

// ============================================================================
// CLASE DERIVADA: CargaAplazable (Herencia de Carga)
// ============================================================================
// REGLAS 2, 3, 4 y 5: Se aplaza si es hora pico, precio caro o embalses < 40%.
// COORDINACION INTELIGENTE: Si ya cumplio sus horas minimas requeridas del dia,
// cede el turno para permitir que otros equipos operen sin violar la potencia contratada.
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

        // Aplazamiento ante condiciones desfavorables
        if (esPico || esPrecioCaro || esEmbalseCritico) {
            return false;
        }

        // Coordinacion de demanda: una vez completadas sus horas diarias de labor,
        // se apaga para liberar potencia en la panaderia
        if (horasMinimas > 0 && horasUso >= horasMinimas) {
            return false;
        }

        return true;
    }
};

#endif // CARGAAPLAZABLE_H
