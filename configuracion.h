#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>

using namespace std;

// --- Directivas del preprocesador -------------------------------------------
#define HORAS_DIA 24
#define DIAS_SEMANA 7
#define MAX_CARGAS 5
#define SEMANAS_MES 4

// ============================================================================
// CONSTANTES DE VALIDACION (Menu y Entradas de Usuario)
// ============================================================================
const int HORA_MIN_VALIDA       = 0;
const int HORA_MAX_VALIDA       = 23;
const float EMBALSE_MIN_VALIDO  = 0.0f;
const float EMBALSE_MAX_VALIDO  = 100.0f;
const float PRECIO_MIN_VALIDO   = 0.0f;

// ============================================================================
// CONSTANTES DEL NEGOCIO: PANADERIA (Cédula 0 - 1)
// ============================================================================
// Potencias nominales en kilovatios (kW)
const float POTENCIA_REFRIGERACION = 1.5f; // Carga critica: conservador de masa y lacteos
const float POTENCIA_HORNO         = 6.0f; // Carga aplazable 1: horno rotativo
const float POTENCIA_AMASADORA     = 2.5f; // Carga aplazable 2: amasadora espiral

// Limites operativos y de mercado electrico
const float POTENCIA_CONTRATADA     = 8.0f;   // kW maximos contratados antes de sobrecarga (Regla 6)
const float PRECIO_CARO             = 750.0f;  // COP/kWh: por encima se aplazan cargas flexibles
const float PRECIO_ESCASEZ          = 1500.0f; // COP/kWh: precio critico de emergencia
const float UMBRAL_EMBALSES_CRITICO = 40.0f;   // % util de embalses: alerta nacional

// Franja horaria pico nocturna (6:00 pm a 8:59 pm)
const int PICO_INICIO = 18;
const int PICO_FIN    = 20;

// Requisitos minimos de operacion diaria (horas de produccion)
const int HORAS_MIN_HORNO     = 6; // Minimo 6 horas para cumplir tandas de horneado
const int HORAS_MIN_AMASADORA = 4; // Minimo 4 horas para preparacion de masa diaria

// Frecuencia de falla en medicion (1 de cada 20 lecturas)
const int FALLA_CADA = 20;

// Nombres constantes de los dias de la semana real
const string DIAS[DIAS_SEMANA] = {
    "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado", "Domingo"
};
