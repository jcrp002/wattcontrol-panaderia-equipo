#pragma once

#include <string>

// ============================================================================
// CLASE BASE ABSTRACTA: Carga
// ============================================================================
class Carga {
protected:
    std::string nombre;
    float potenciaKW;
    bool encendida;
    int horasUso;        // Horas efectivas operadas

public:
    Carga(std::string n, float p) : nombre(n), potenciaKW(p), encendida(false), horasUso(0) {}
    virtual ~Carga() {}

    // Metodo virtual puro (= 0): cada tipo de carga implementa su regla
    virtual bool debeEncender(int hora, float precio, float embalses) = 0;

    void encender() {
        encendida = true;
    }

    void apagar() {
        encendida = false;
    }

    // Solo acumula si efectivamente permanecio encendida tras el control de potencia
    void acumularHora() {
        if (encendida) {
            horasUso++;
        }
    }

    float costoHora(float precio) const {
        return encendida ? (potenciaKW * precio) : 0.0f;
    }

    void reiniciar() {
        encendida = false;
        horasUso = 0;
    }

    std::string getNombre() const { return nombre; }
    float getPotencia() const { return potenciaKW; }
    bool estaEncendido() const { return encendida; }
    int getHorasUso() const { return horasUso; }
};
