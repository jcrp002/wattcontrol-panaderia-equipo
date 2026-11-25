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

    // SIMULAR LA SEMANA REAL DE XM (7 dias x 24 horas = 168 horas)
    void simularSemana() {
        medidor.reiniciar();
        diasCumplidosHorno = 0;
        diasCumplidosAmasadora = 0;

        for (int d = 0; d < DIAS_SEMANA; d++) {
            // Cada nuevo dia reinicia el acumulador de horas de los equipos
            for (int i = 0; i < numCargas; i++) {
                cargas[i]->reiniciar();
            }

            for (int h = 0; h < HORAS_DIA; h++) {
                // Consulta el precio real de XM desde la matriz en Mercado
                float precio = mercado->precioHora(d, h);
                float embalses = mercado->nivelEmbalses(h);

                // Escenario SIN GESTOR: todo encendido las 24 horas
                float costoHoraSin = 0.0f;
                for (int i = 0; i < numCargas; i++) {
                    costoHoraSin += cargas[i]->getPotencia() * precio;
                }
                costosSin[d][h] = costoHoraSin;

                // Escenario CON GESTOR:
                // Evaluacion polimorfica
                for (int i = 0; i < numCargas; i++) {
                    if (cargas[i]->debeEncender(h, precio, embalses)) {
                        cargas[i]->encender();
                    } else {
                        cargas[i]->apagar();
                    }
                }

                // Control de potencia contratada (Regla 6)
                aplicarControlPotencia(h);

                // Calculo de costo real y acumulacion de horas efectivas
                float costoHoraCon = 0.0f;
                float kwhHoraCon = 0.0f;
                for (int i = 0; i < numCargas; i++) {
                    if (cargas[i]->estaEncendido()) {
                        cargas[i]->acumularHora();
                        costoHoraCon += cargas[i]->costoHora(precio);
                        kwhHoraCon += cargas[i]->getPotencia();
                    }
                }

                costosCon[d][h] = costoHoraCon;
                medidor.registrarConsumo(kwhHoraCon, costoHoraCon);
            }

            // Auditoria de horas minimas al cierre de cada dia
            if (cargas[1]->getHorasUso() >= HORAS_MIN_HORNO) diasCumplidosHorno++;
            if (cargas[2]->getHorasUso() >= HORAS_MIN_AMASADORA) diasCumplidosAmasadora++;
        }

        semanaSimulada = true;
        cout << "\n>> Simulacion de la semana real completada exitosamente <<" << endl;
        cout << "   Total horas evaluadas: 7 dias x 24 horas = 168 horas." << endl;
    }

    // REPORTE SEMANAL DETALLADO (Opcion 3)
    void reporteSemanal() {
        if (!semanaSimulada) {
            cout << "[AVISO] Primero debe simular la semana con la opcion 2." << endl;
            return;
        }

        float totalesCon[DIAS_SEMANA];
        float totalesSin[DIAS_SEMANA];

        cout << fixed << setprecision(0);
        cout << "\n";
        linea(68, '=');
        cout << "           REPORTE FINANCIERO SEMANAL - WATTCONTROL" << endl;
        cout << " Negocio: " << nombre << " | Responsable: " << responsable << endl;
        cout << " Datos reales XM: 21 al 27 de Septiembre de 2026" << endl;
        linea(68, '=');
        cout << setw(10) << left << "Dia" 
             << setw(16) << right << "Sin Gestor($)" 
             << setw(16) << "Con Gestor($)" 
             << setw(14) << "Ahorro($)" 
             << setw(10) << "Ahorro%" << endl;
        linea(68);

        for (int d = 0; d < DIAS_SEMANA; d++) {
            // Uso de la funcion libre sobrecargada sumar()
            totalesCon[d] = sumar(costosCon[d], HORAS_DIA);
            totalesSin[d] = sumar(costosSin[d], HORAS_DIA);
            float ahorroDia = totalesSin[d] - totalesCon[d];
            float pctDia = (totalesSin[d] > 0) ? (ahorroDia / totalesSin[d] * 100.0f) : 0.0f;

            cout << setw(10) << left << DIAS[d]
                 << setw(16) << right << totalesSin[d]
                 << setw(16) << totalesCon[d]
                 << setw(14) << ahorroDia
                 << setw(9) << setprecision(1) << pctDia << "%" 
                 << setprecision(0) << endl;
        }
        linea(68);

        // Identificacion de picos mediante posicionMaximo()
        int idxDiaCaro = posicionMaximo(totalesSin, DIAS_SEMANA);

        // Buscar la hora con el precio mas alto de toda la semana
        int diaPico = 0;
        int horaPico = 0;
        float maxPrecio = 0.0f;
        for (int d = 0; d < DIAS_SEMANA; d++) {
            for (int h = 0; h < HORAS_DIA; h++) {
                float p = mercado->precioHora(d, h);
                if (p > maxPrecio) {
                    maxPrecio = p;
                    diaPico = d;
                    horaPico = h;
                }
            }
        }

        float totalSemanaCon = sumar(totalesCon, DIAS_SEMANA);
        float totalSemanaSin = sumar(totalesSin, DIAS_SEMANA);
        float ahorroSemanal  = totalSemanaSin - totalSemanaCon;
        float pctAhorroSem   = (totalSemanaSin > 0) ? (ahorroSemanal / totalSemanaSin * 100.0f) : 0.0f;

        // Proyeccion mensual con la funcion propia proyectarMes()
        float proyMensualCon = proyectarMes(totalSemanaCon);
        float proyMensualSin = proyectarMes(totalSemanaSin);
        float proyAhorroMes  = proyectarMes(ahorroSemanal);

        cout << "\n--- ANALISIS DE PICOS Y DIAS CRITICOS ---" << endl;
        cout << " - Dia mas costoso de la semana: " << DIAS[idxDiaCaro] 
             << " ($" << totalesSin[idxDiaCaro] << " COP sin gestor)" << endl;
        cout << " - Hora pico mas cara de la semana: " << DIAS[diaPico] 
             << " a las " << horaPico << ":00 h ($" << maxPrecio << " COP/kWh)" << endl;
        cout << " - Promedio diario con gestor:   $" << promedio(totalesCon, DIAS_SEMANA) << " COP" << endl;

        cout << "\n--- CUMPLIMIENTO OPERATIVO DE PRODUCCION (7 DIAS) ---" << endl;
        cout << " - Horno Rotativo (meta >= 6h/dia):  " << diasCumplidosHorno << " de 7 dias cumplidos." << endl;
        cout << " - Amasadora (meta >= 4h/dia):      " << diasCumplidosAmasadora << " de 7 dias cumplidos." << endl;
        if (diasCumplidosHorno == 7 && diasCumplidosAmasadora == 7) {
            cout << " >> ESTADO: 100% DE LAS TANDAS DE PAN ASEGURADAS EN LA SEMANA <<" << endl;
        } else {
            cout << " >> ESTADO: ALERTA DE PRODUCCION PARCIAL EN ALGUNOS DIAS <<" << endl;
        }

        cout << "\n--- BALANCE CONSOLIDADO Y PROYECCION FINANCIERA ---" << endl;
        cout << " Costo total semana SIN gestor:   $" << setw(10) << totalSemanaSin << " COP" << endl;
        cout << " Costo total semana CON gestor:   $" << setw(10) << totalSemanaCon << " COP" << endl;
        cout << " AHORRO ECONOMICO SEMANAL:       $" << setw(10) << ahorroSemanal 
             << " COP (" << setprecision(1) << pctAhorroSem << " %)" << setprecision(0) << endl;
        linea(68);
        cout << " Proyeccion mensual SIN gestor:  $" << setw(10) << proyMensualSin << " COP" << endl;
        cout << " Proyeccion mensual CON gestor:  $" << setw(10) << proyMensualCon << " COP" << endl;
        cout << " PROYECCION AHORRO MENSUAL:      $" << setw(10) << proyAhorroMes << " COP" << endl;
        linea(68, '=');
    }
};
