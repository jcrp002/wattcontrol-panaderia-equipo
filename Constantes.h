#ifndef CONSTANTES_H
#define CONSTANTES_H

// --- Directivas del preprocesador -------------------------------------------
#define HORAS_DIA 24
#define MAX_CARGAS 5
#define SEMILLA 2026

// ============================================================================
// CONSTANTES DE VALIDACION (Menu y Entradas de Usuario)
// ============================================================================
const int HORA_MIN_VALIDA       = 0;
const int HORA_MAX_VALIDA       = 23;
const float EMBALSE_MIN_VALIDO  = 0.0f;
const float EMBALSE_MAX_VALIDO  = 100.0f;
const float PRECIO_MIN_VALIDO   = 0.0f;

// ============================================================================
// CONSTANTES DEL MERCADO MAYORISTA DE ENERGIA (XM Colombia)
// ============================================================================
// Limites de franjas horarias
const int FRANJA_MADRUGADA_FIN = 6;
const int FRANJA_MANANA_FIN    = 14;
const int FRANJA_TARDE_FIN     = 17;
const int FRANJA_PICO_FIN      = 20;

// Precios minimos y maximos por franja horaria (COP/kWh)
const int PRECIO_MIN_MADRUGADA = 398;
const int PRECIO_MAX_MADRUGADA = 640;

const int PRECIO_MIN_MANANA    = 456;
const int PRECIO_MAX_MANANA    = 970;

const int PRECIO_MIN_TARDE     = 596;
const int PRECIO_MAX_TARDE     = 1040;

const int PRECIO_MIN_PICO      = 705;
const int PRECIO_MAX_PICO      = 1612;

const int PRECIO_MIN_NOCHE     = 565;
const int PRECIO_MAX_NOCHE     = 970;

// Parametros para simulacion del nivel de embalses
const float EMBALSE_NIVEL_BASE     = 42.0f;
const float EMBALSE_AMPLITUD_VAR   = 2.0f;
const int   EMBALSE_PERIODO_VAR    = 5;

// ============================================================================
// CONSTANTES DEL NEGOCIO: PANADERIA
// ============================================================================
// Potencias nominales en kilovatios (kW)
const float POTENCIA_REFRIGERACION = 1.5f; // Carga critica: conservador de levaduras y lacteos
const float POTENCIA_HORNO         = 6.0f; // Carga aplazable 1: horno rotativo
const float POTENCIA_AMASADORA     = 2.5f; // Carga aplazable 2: amasadora espiral

// Limites operativos y de tarifas
const float POTENCIA_CONTRATADA     = 8.0f;   // kW maximos contratados antes de sobrecarga (Regla 6)
const float PRECIO_CARO             = 750.0f;  // COP/kWh: por encima se aplazan cargas flexibles
const float PRECIO_ESCASEZ          = 1500.0f; // COP/kWh: precio critico de emergencia
const float UMBRAL_EMBALSES_CRITICO = 40.0f;   // % util de embalses: alerta nacional de escasez

// Franja horaria pico nocturna (6:00 pm a 8:59 pm)
const int PICO_INICIO = 18;
const int PICO_FIN    = 20;

// Requisitos minimos de operacion diaria (horas de produccion)
const int HORAS_MIN_HORNO     = 6; // Minimo 6 horas para cumplir tandas de horneado
const int HORAS_MIN_AMASADORA = 4; // Minimo 4 horas para preparacion de masa diaria

// Frecuencia de falla en medicion (1 de cada 20 lecturas para salto continue)
const int FALLA_CADA = 20;

#endif // CONSTANTES_H
