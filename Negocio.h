#pragma once

#include "configuracion.h"
#include "utilidades.h"
#include "Carga.h"
#include "CargaAplazable.h"
#include "Medidor.h"
#include "Mercado.h"

// ============================================================================
// CLASE Negocio: AGREGACION, COMPOSICION, ASOCIACION Y MATRIZ DE COSTOS
// ============================================================================
class Negocio {
private:
    string nombre;
    string responsable;

    Medidor medidor;                   // COMPOSICION
    Carga* cargas[MAX_CARGAS];         // AGREGACION
    int numCargas;
    Mercado* mercado;                  // ASOCIACION

    // MATRICES DE COSTOS SEMANALES [7 dias][24 horas]
    float costosCon[DIAS_SEMANA][HORAS_DIA];
    float costosSin[DIAS_SEMANA][HORAS_DIA];

    // Contadores de dias con produccion completa
    int diasCumplidosHorno;
    int diasCumplidosAmasadora;
    bool semanaSimulada;

public:
    Negocio(string n, string resp, Mercado* m)
        : nombre(n), responsable(resp), numCargas(0), mercado(m),
          diasCumplidosHorno(0), diasCumplidosAmasadora(0), semanaSimulada(false) {
        for (int d = 0; d < DIAS_SEMANA; d++) {
            for (int h = 0; h < HORAS_DIA; h++) {
                costosCon[d][h] = 0.0f;
                costosSin[d][h] = 0.0f;
            }
        }
    }

    void agregarCarga(Carga* c) {
        if (numCargas < MAX_CARGAS) {
            cargas[numCargas++] = c;
        }
    }

    void mostrarConfiguracion() {
        linea(60, '=');
        cout << " GESTOR ENERGETICO WATTCONTROL - EA3 SEMANA REAL" << endl;
        cout << " Negocio: " << nombre << " | Responsable: " << responsable << endl;
        cout << " Potencia Contratada: " << POTENCIA_CONTRATADA << " kW" << endl;
        linea(60);
        cout << " Equipos registrados en el gestor:" << endl;
        for (int i = 0; i < numCargas; i++) {
            cout << "  - " << setw(18) << left << cargas[i]->getNombre()
                 << right << setw(5) << cargas[i]->getPotencia() << " kW" << endl;
        }
        linea(60, '=');
    }

    // Evaluacion manual de una sola hora (Opcion 1)
    void evaluarManual(int hora, float precio, float embalses) {
        cout << "\nEvaluando decisiones para lectura manual:" << endl;
        cout << " Hora: " << hora << ":00 | Precio: $" << precio 
             << " COP/kWh | Embalses: " << embalses << " %" << endl;
        linea(50);
        float potTotal = 0.0f;
        for (int i = 0; i < numCargas; i++) {
            bool on = cargas[i]->debeEncender(hora, precio, embalses);
            cout << "  " << setw(18) << left << cargas[i]->getNombre() << right;
            if (on) {
                cout << "[ENCENDER]" << endl;
                potTotal += cargas[i]->getPotencia();
            } else {
                cout << "[APAGAR / APLAZAR]" << endl;
            }
        }
        cout << "  Potencia requerida: " << potTotal << " kW / " 
             << POTENCIA_CONTRATADA << " kW contratados." << endl;
    }

    // REGLA 6: Control de potencia contratada
    void aplicarControlPotencia(int hora) {
        float sumaPotencia = 0.0f;
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->estaEncendido()) {
                sumaPotencia += cargas[i]->getPotencia();
            }
        }

        if (sumaPotencia > POTENCIA_CONTRATADA) {
            int idxMayor = -1;
            float maxPot = 0.0f;
            // Desconectar la aplazable mas pesada (omite cargas[0] que es critica)
            for (int i = 1; i < numCargas; i++) {
                if (cargas[i]->estaEncendido() && cargas[i]->getPotencia() > maxPot) {
                    maxPot = cargas[i]->getPotencia();
                    idxMayor = i;
                }
            }
            if (idxMayor != -1) {
                cargas[idxMayor]->apagar();
            }
        }
    }

};
