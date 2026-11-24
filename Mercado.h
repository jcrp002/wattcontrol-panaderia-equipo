#ifndef MERCADO_H
#define MERCADO_H

#include <cstdlib>
#include "Constantes.h"

// ============================================================================
// CLASE Mercado: ASOCIACION con Negocio
// ============================================================================
class Mercado {
public:
    // Retorna precio aleatorio dentro de los rangos reales de XM utilizando constantes
    float precioHora(int hora) {
        int minimo, maximo;
        if (hora <= FRANJA_MADRUGADA_FIN) {
            minimo = PRECIO_MIN_MADRUGADA;
            maximo = PRECIO_MAX_MADRUGADA;
        } else if (hora <= FRANJA_MANANA_FIN) {
            minimo = PRECIO_MIN_MANANA;
            maximo = PRECIO_MAX_MANANA;
        } else if (hora <= FRANJA_TARDE_FIN) {
            minimo = PRECIO_MIN_TARDE;
            maximo = PRECIO_MAX_TARDE;
        } else if (hora <= FRANJA_PICO_FIN) {
            minimo = PRECIO_MIN_PICO;
            maximo = PRECIO_MAX_PICO;
        } else {
            minimo = PRECIO_MIN_NOCHE;
            maximo = PRECIO_MAX_NOCHE;
        }
        return minimo + rand() % (maximo - minimo + 1);
    }

    // Simula nivel de embalses nacional
    float nivelEmbalses(int hora) {
        float variacion = (hora % EMBALSE_PERIODO_VAR) - EMBALSE_AMPLITUD_VAR;
        return EMBALSE_NIVEL_BASE + variacion;
    }
};

#endif // MERCADO_H
