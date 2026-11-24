#ifndef MEDIDOR_H
#define MEDIDOR_H

// ============================================================================
// CLASE Medidor: COMPOSICION con Negocio
// ============================================================================
class Medidor {
private:
    float energiaTotalKWh;
    float costoTotalCOP;

public:
    Medidor() : energiaTotalKWh(0.0f), costoTotalCOP(0.0f) {}

    void registrarConsumo(float kwh, float costo) {
        energiaTotalKWh += kwh;
        costoTotalCOP += costo;
    }

    void reiniciar() {
        energiaTotalKWh = 0.0f;
        costoTotalCOP = 0.0f;
    }

    float getEnergiaTotal() const { return energiaTotalKWh; }
    float getCostoTotal() const { return costoTotalCOP; }
};

#endif // MEDIDOR_H
