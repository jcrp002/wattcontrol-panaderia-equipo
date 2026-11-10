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

// ============================================================================
// HERENCIA: CargaCritica ES UNA Carga 
// ============================================================================
// Cargas vitales para la panaderia (Refrigeracion de levaduras y lacteos).
// REGLA 1: NUNCA se apagan por precio, horario ni nivel de embalses.
class CargaCritica : public Carga {
public:
    CargaCritica(string n, float p) : Carga(n, p) {}

    // Sobrescritura polimorfica (override): siempre debe estar encendida
    bool debeEncender(int hora, float precio, float embalses) override {
        // Ignora parametros; la cadena de frio no puede romperse
        (void)hora;
        (void)precio;
        (void)embalses;
        return true; 
    }
};

// ============================================================================
// HERENCIA: CargaAplazable ES UNA Carga 
// ============================================================================
// Equipos de produccion que pueden postergarse para horas economicas.
// REGLA 2: Se evitan en horas pico, si el precio supera el umbral caro
// o si los embalses estan en situacion critica.
class CargaAplazable : public Carga {
private:
    int horasMinimas; // Horas que idealmente necesita operar al dia

public:
    CargaAplazable(string n, float p, int hMin) : Carga(n, p) {
        horasMinimas = hMin;
    }

    int getHorasMinimas() { return horasMinimas; }

    // Sobrescritura polimorfica (override) con operadores logicos compuestos:
    // && (Y), || (O), ! (NO)
    bool debeEncender(int hora, float precio, float embalses) override {
        bool esPico = (hora >= PICO_INICIO && hora <= PICO_FIN);
        bool esPrecioCaro = (precio >= PRECIO_CARO);
        bool esEmbalseCritico = (embalses < UMBRAL_EMBALSES_CRITICO);

        // Si se encuentra en cualquiera de estas condiciones desfavorables, se aplaza
        if (esPico || esPrecioCaro || esEmbalseCritico) {
            return false;
        }

        // En condiciones normales y con tarifa favorable, se autoriza encendido
        return true;
    }
};