#ifndef CARGA_H
#define CARGA_H

#include <string>

// ============================================================================
// CLASE BASE ABSTRACTA: Carga
// ============================================================================
class Carga {
protected:
    std::string nombre;       // Nombre descriptivo de la carga
    float potenciaKW;    // Potencia en kilovatios (kW)
    bool encendida;      // Estado en la hora simulada (true: encendida, false: apagada)
    int horasUso;        // Acumulador de horas EFECTIVAS operadas en el dia

public:
    Carga(std::string n, float p) : nombre(n), potenciaKW(p), encendida(false), horasUso(0) {}
    virtual ~Carga() {}

    // Metodo virtual puro: obliga a la implementacion polimorfica en clases hijas
    virtual bool debeEncender(int hora, float precio, float embalses) = 0;

    // Modifica el estado de encendido sin acumular horas prematuramente
    void encender() {
        encendida = true;
    }

    void apagar() {
        encendida = false;
    }

    // Acumula una hora de uso unicamenta si la carga efectivamente opero
    void acumularHora() {
        if (encendida) {
            horasUso++;
        }
    }

    // Costo en COP generado en una hora si esta encendida
    float costoHora(float precio) {
        if (encendida) {
            return potenciaKW * precio;
        }
        return 0.0f;
    }

    void reiniciar() {
        encendida = false;
        horasUso = 0;
    }

    // Getters para encapsulamiento
    std::string getNombre() const { return nombre; }
    float getPotencia() const { return potenciaKW; }
    bool estaEncendido() const { return encendida; }
    int getHorasUso() const { return horasUso; }
};

#endif // CARGA_H
