#ifndef NEGOCIO_H
#define NEGOCIO_H

#include <iostream>
#include <iomanip>
#include <string>
#include "Constantes.h"
#include "Carga.h"
#include "CargaAplazable.h"
#include "Medidor.h"
#include "Mercado.h"

// ============================================================================
// CLASE Negocio: AGREGACION de Cargas, COMPOSICION de Medidor, ASOCIACION de Mercado
// ============================================================================
class Negocio {
private:
    std::string nombre;
    Medidor medidor;             // Composicion
    Carga* cargas[MAX_CARGAS];   // Agregacion
    int numCargas;
    Mercado* mercado;            // Asociacion

    float costoConGestor;
    float costoSinGestor;
    float energiaConGestor;
    float energiaSinGestor;
    int horasSimuladas;
    int fallasLectura;

public:
    Negocio(std::string n, Mercado* m)
        : nombre(n), numCargas(0), mercado(m),
          costoConGestor(0.0f), costoSinGestor(0.0f),
          energiaConGestor(0.0f), energiaSinGestor(0.0f),
          horasSimuladas(0), fallasLectura(0) {}

    void agregarCarga(Carga* c) {
        if (numCargas < MAX_CARGAS) {
            cargas[numCargas++] = c;
        }
    }

    void mostrarConfiguracion() {
        std::cout << "=====================================================" << std::endl;
        std::cout << " Negocio: " << nombre << " (Panaderia)" << std::endl;
        std::cout << " Potencia Contratada: " << POTENCIA_CONTRATADA << " kW" << std::endl;
        std::cout << " Equipos registrados en el gestor:" << std::endl;
        for (int i = 0; i < numCargas; i++) {
            std::cout << "  - " << std::setw(18) << std::left << cargas[i]->getNombre()
                      << std::right << std::setw(5) << cargas[i]->getPotencia() << " kW" << std::endl;
        }
        std::cout << "=====================================================" << std::endl;
    }

    // Evaluacion manual polimorfica (Opcion 2)
    void evaluarManual(int hora, float precio, float embalses) {
        std::cout << "\nEvaluando decision de cada equipo para la lectura:" << std::endl;
        std::cout << "  Hora: " << hora << ":00 | Precio: $" << precio 
                  << " COP/kWh | Embalses: " << embalses << " %" << std::endl;
        std::cout << "-----------------------------------------------------" << std::endl;
        
        float potenciaDemandada = 0.0f;
        for (int i = 0; i < numCargas; i++) {
            bool encender = cargas[i]->debeEncender(hora, precio, embalses);
            std::cout << "  " << std::setw(18) << std::left << cargas[i]->getNombre() << std::right;
            if (encender) {
                std::cout << "[ENCENDER]" << std::endl;
                potenciaDemandada += cargas[i]->getPotencia();
            } else {
                std::cout << "[APAGAR / APLAZAR]" << std::endl;
            }
        }
        std::cout << "  Potencia total estimada: " << potenciaDemandada << " kW / " 
                  << POTENCIA_CONTRATADA << " kW contratados." << std::endl;
    }

    // REGLA 6: Control de potencia contratada.
    // Si la potencia encendida supera la contratada, desconecta la aplazable mas pesada.
    void aplicarControlPotencia(int hora) {
        float sumaPotencia = 0.0f;
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->estaEncendido()) {
                sumaPotencia += cargas[i]->getPotencia();
            }
        }

        if (sumaPotencia > POTENCIA_CONTRATADA) {
            std::cout << " [ALERTA HORA " << std::setw(2) << hora << "] Sobrecarga detectada (" 
                      << sumaPotencia << " kW > " << POTENCIA_CONTRATADA 
                      << " kW). Desconectando carga aplazable de mayor potencia..." << std::endl;
            
            int idxMayor = -1;
            float maxPot = 0.0f;
            // Se omite cargas[0] porque es critica y no debe desconectarse
            for (int i = 1; i < numCargas; i++) {
                if (cargas[i]->estaEncendido() && cargas[i]->getPotencia() > maxPot) {
                    maxPot = cargas[i]->getPotencia();
                    idxMayor = i;
                }
            }
            if (idxMayor != -1) {
                cargas[idxMayor]->apagar();
                std::cout << "   -> " << cargas[idxMayor]->getNombre() 
                          << " apagado preventivamente por limite de potencia." << std::endl;
            }
        }
    }

    // Simulacion de 24 horas continuas (Opcion 3)
    void simularDia() {
        medidor.reiniciar();
        costoConGestor = 0.0f;
        costoSinGestor = 0.0f;
        energiaConGestor = 0.0f;
        energiaSinGestor = 0.0f;
        horasSimuladas = 0;
        fallasLectura = 0;

        for (int i = 0; i < numCargas; i++) {
            cargas[i]->reiniciar();
        }

        std::cout << std::fixed << std::setprecision(0);
        std::cout << "\n================ SIMULACION 24 HORAS: WATTCONTROL PANADERIA ================" << std::endl;
        std::cout << std::setw(5) << "Hora" << std::setw(8) << "Precio" << std::setw(9) << "Embalse";
        for (int i = 0; i < numCargas; i++) {
            std::cout << std::setw(13) << cargas[i]->getNombre().substr(0, 11);
        }
        std::cout << std::setw(10) << "Costo(COP)" << std::endl;
        std::cout << "----------------------------------------------------------------------------" << std::endl;

        for (int h = 0; h < HORAS_DIA; h++) {
            float precio = mercado->precioHora(h);
            float embalses = mercado->nivelEmbalses(h);

            // Escenario SIN GESTOR: todo encendido indiscriminadamente
            for (int i = 0; i < numCargas; i++) {
                costoSinGestor += cargas[i]->getPotencia() * precio;
                energiaSinGestor += cargas[i]->getPotencia();
            }

            // INSTRUCCION DE CONTROL BREAK:
            // Requiere SIMULTANEAMENTE precio > 1500 COP/kWh Y embalses < 40 %
            if (precio > PRECIO_ESCASEZ && embalses < UMBRAL_EMBALSES_CRITICO) {
                std::cout << std::setw(5) << h << std::setw(8) << precio 
                          << "   [BREAK] EMERGENCIA DE RACIONAMIENTO (Precio > " << PRECIO_ESCASEZ 
                          << " COP y Embalses < " << UMBRAL_EMBALSES_CRITICO << "%). Simulacion detenida." << std::endl;
                break;
            }

            horasSimuladas++;

            // INSTRUCCION DE CONTROL CONTINUE: Falla de medicion (1 de cada 20 lecturas)
            if (rand() % FALLA_CADA == 0) {
                fallasLectura++;
                // En falla, solo opera la carga critica
                cargas[0]->encender();
                cargas[0]->acumularHora();
                for (int i = 1; i < numCargas; i++) {
                    cargas[i]->apagar();
                }
                float costoFalla = cargas[0]->costoHora(precio);
                costoConGestor += costoFalla;
                energiaConGestor += cargas[0]->getPotencia();
                medidor.registrarConsumo(cargas[0]->getPotencia(), costoFalla);

                std::cout << std::setw(5) << h << std::setw(8) << "--" << std::setw(9) << "--"
                          << "   [CONTINUE] Falla telemedicion: Solo refrigeracion activa." << std::endl;
                continue;
            }

            // Escenario CON GESTOR INTELIGENTE:
            // 1. Evaluacion polimorfica de cada equipo
            for (int i = 0; i < numCargas; i++) {
                if (cargas[i]->debeEncender(h, precio, embalses)) {
                    cargas[i]->encender();
                } else {
                    cargas[i]->apagar();
                }
            }

            // 2. Aplicacion de la Regla 6: Control de potencia contratada
            aplicarControlPotencia(h);

            // 3. Calculo de costos y acumulacion real de horas efectivas
            float costoHoraActual = 0.0f;
            float energiaHoraActual = 0.0f;
            for (int i = 0; i < numCargas; i++) {
                if (cargas[i]->estaEncendido()) {
                    cargas[i]->acumularHora(); // Solo suma la hora si REALMENTE quedo encendida
                    costoHoraActual += cargas[i]->costoHora(precio);
                    energiaHoraActual += cargas[i]->getPotencia();
                }
            }

            costoConGestor += costoHoraActual;
            energiaConGestor += energiaHoraActual;
            medidor.registrarConsumo(energiaHoraActual, costoHoraActual);

            // Impresion de fila en la tabla
            std::cout << std::setw(5) << h << std::setw(8) << precio << std::setw(8) << embalses << "%";
            for (int i = 0; i < numCargas; i++) {
                std::cout << std::setw(13) << (cargas[i]->estaEncendido() ? "ON" : "off");
            }
            std::cout << std::setw(10) << costoHoraActual << std::endl;
        }

        std::cout << "----------------------------------------------------------------------------" << std::endl;
        std::cout << "Simulacion completada. Vea el resumen con la opcion 4." << std::endl;
    }

    // Resumen financiero y operativo (Opcion 4)
    void resumen() {
        std::cout << std::fixed << std::setprecision(0);
        std::cout << "\n============================================================" << std::endl;
        std::cout << "        RESUMEN FINANCIERO Y ENERGETICO - WATTCONTROL        " << std::endl;
        std::cout << "                   Panaderia Central                        " << std::endl;
        std::cout << "============================================================" << std::endl;
        std::cout << "Horas simuladas: " << horasSimuladas << " de " << HORAS_DIA 
                  << " | Fallas de medicion controladas: " << fallasLectura << std::endl;
        std::cout << "\n--- Detalle por Equipo ---" << std::endl;
        
        bool produccionCompleta = true;
        for (int i = 0; i < numCargas; i++) {
            float kwh = cargas[i]->getHorasUso() * cargas[i]->getPotencia();
            std::cout << " - " << std::setw(18) << std::left << cargas[i]->getNombre() << std::right
                      << std::setw(3) << cargas[i]->getHorasUso() << " h operadas ("
                      << std::setw(5) << kwh << " kWh consumidos)" << std::endl;

            if (i > 0) { // Cargas aplazables
                CargaAplazable* cap = dynamic_cast<CargaAplazable*>(cargas[i]);
                if (cap && cargas[i]->getHorasUso() < cap->getHorasMinimas()) {
                    std::cout << "   [ALERTA PRODUCCION] " << cargas[i]->getNombre() 
                              << " no alcanzo el minimo requerido de " << cap->getHorasMinimas() << " horas." << std::endl;
                    produccionCompleta = false;
                }
            }
        }

        if (produccionCompleta) {
            std::cout << "\n>> Cumplimiento operativo: TANDAS DE PAN COMPLETADAS AL 100% <<" << std::endl;
        } else {
            std::cout << "\n>> Cumplimiento operativo: ALERTA DE PRODUCCION PARCIAL <<" << std::endl;
        }

        std::cout << "\n--- Balance Energetico y Economico ---" << std::endl;
        std::cout << "Energia sin gestor:  " << std::setw(8) << energiaSinGestor << " kWh" << std::endl;
        std::cout << "Energia con gestor:  " << std::setw(8) << medidor.getEnergiaTotal() << " kWh" << std::endl;
        std::cout << "Costo sin gestor:   $" << std::setw(8) << costoSinGestor << " COP" << std::endl;
        std::cout << "Costo con gestor:   $" << std::setw(8) << costoConGestor << " COP" << std::endl;
        
        float ahorroCOP = costoSinGestor - costoConGestor;
        float ahorroPorc = (costoSinGestor > 0) ? (ahorroCOP / costoSinGestor * 100.0f) : 0.0f;

        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "AHORRO ECONOMICO:   $" << std::setw(8) << ahorroCOP << " COP (" 
                  << std::setprecision(1) << ahorroPorc << " %)" << std::endl;
        std::cout << "============================================================" << std::endl;
    }
};

#endif // NEGOCIO_H
