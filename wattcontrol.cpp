#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>

using namespace std;

// --- Directivas del preprocesador -------------------------------------------
#define HORAS_DIA 24
#define MAX_CARGAS 5
#define SEMILLA 2026

// ============================================================================
// CONSTANTES DEL NEGOCIO: PANADERIA
// ============================================================================
// Potencias de los equipos en kilovatios (kW)
const float POTENCIA_REFRIGERACION = 1.5; // Carga critica: vitrina y conservador de masa
const float POTENCIA_HORNO         = 6.0; // Carga aplazable 1: horno rotativo
const float POTENCIA_AMASADORA     = 2.5; // Carga aplazable 2: amasadora espiral

// Limites operativos y de mercado electrico
const float POTENCIA_CONTRATADA    = 8.0; // kW maximos antes de sobrecarga (Regla 6)
const float PRECIO_CARO            = 750.0; // COP/kWh: por encima de esto se aplaza
const float PRECIO_ESCASEZ         = 1500.0; // COP/kWh: precio critico que detiene simulacion
const float UMBRAL_EMBALSES_CRITICO = 40.0; // % de embalses: si baja de aqui, ahorro forzoso

// Franja horaria pico (6:00 pm a 8:59 pm)
const int PICO_INICIO = 18;
const int PICO_FIN    = 20;

// Requisitos minimos de produccion diaria (horas necesarias)
const int HORAS_MIN_HORNO     = 6; // Para cumplir las tandas de pan del dia
const int HORAS_MIN_AMASADORA = 4; // Para la preparacion de mezclas

// Probabilidad de falla en medicion (1 de cada 20 lecturas)
const int FALLA_CADA = 20;

// ============================================================================
// CLASE BASE ABSTRACTA: Carga 
// ============================================================================
// Representa el molde general para cualquier consumo electrico del negocio.
class Carga {
protected:
    string nombre;       // Nombre de la carga ("Refrigeracion", "Horno", etc.)
    float potenciaKW;    // Consumo nominal en kW
    bool encendida;      // Estado actual en la hora simulada
    int horasUso;        // Acumulador de horas operadas en el dia

public:
    // Constructor
    Carga(string n, float p) {
        nombre = n;
        potenciaKW = p;
        encendida = false;
        horasUso = 0;
    }

    // Destructor virtual: obligatorio para polimorfismo con punteros base
    virtual ~Carga() {}

    // METODO VIRTUAL PURO (= 0): Convierte a Carga en clase abstracta.
    // Obliga a cada clase derivada a implementar su propia logica de decision (POLIMORFISMO).
    virtual bool debeEncender(int hora, float precio, float embalses) = 0;

    // Enciende la carga en la hora actual y acumula horas de operacion
    void encender() {
        encendida = true;
        horasUso++;
    }

    // Apaga la carga
    void apagar() {
        encendida = false;
    }

    // Calcula el costo de la hora actual en COP
    float costoHora(float precio) {
        if (encendida) {
            return potenciaKW * precio;
        }
        return 0.0;
    }

    // Reinicia contadores para una nueva simulacion
    void reiniciar() {
        encendida = false;
        horasUso = 0;
    }

    // Metodos Getters para encapsulamiento
    string getNombre() { return nombre; }
    float getPotencia() { return potenciaKW; }
    bool estaEncendido() { return encendida; }
    int getHorasUso() { return horasUso; }
};
