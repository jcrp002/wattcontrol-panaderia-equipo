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


// ============================================================================
// CLASE Medidor: COMPOSICION con Negocio [PERSONA 2]
// ============================================================================
// Representa el medidor bidireccional / inteligente que registra energia y costos.
// Nace y muere como parte integral del Negocio.
class Medidor {
private:
    float energiaTotalKWh; // Acumulador de energia en kilovatios-hora
    float costoTotalCOP;   // Acumulador financiero en pesos colombianos

public:
    Medidor() {
        energiaTotalKWh = 0.0;
        costoTotalCOP = 0.0;
    }

    void registrarConsumo(float kwh, float costo) {
        energiaTotalKWh += kwh;
        costoTotalCOP += costo;
    }

    void reiniciar() {
        energiaTotalKWh = 0.0;
        costoTotalCOP = 0.0;
    }

    float getEnergiaTotal() { return energiaTotalKWh; }
    float getCostoTotal() { return costoTotalCOP; }
};

// ============================================================================
// CLASE Mercado: ASOCIACION con Negocio [PERSONA 2]
// ============================================================================
// Simula el mercado mayorista de energia (precios de bolsa segun XM Colombia)
// y el estado hidrologico nacional (nivel de embalses).
class Mercado {
public:
    // Retorna el precio del kWh segun la franja horaria colombiana
    float precioHora(int hora) {
        int minimo, maximo;
        if (hora <= 6) {               // Madrugada
            minimo = 398; maximo = 640;
        } else if (hora <= 14) {        // Manana y mediodia
            minimo = 456; maximo = 970;
        } else if (hora <= 17) {        // Tarde
            minimo = 596; maximo = 1040;
        } else if (hora <= 20) {        // Hora pico
            minimo = 705; maximo = 1612;
        } else {                        // Noche
            minimo = 565; maximo = 970;
        }
        return minimo + rand() % (maximo - minimo + 1);
    }

    // Simula el porcentaje de volumen util de los embalses hidroelectricos (0 a 100 %)
    float nivelEmbalses(int hora) {
        // En periodo de estiaje / El Nino, fluctua alrededor del 42 % con leves variaciones
        float base = 42.0;
        float variacion = (hora % 5) - 2.0; // Variacion entre -2 y +2 %
        return base + variacion;
    }
};

// ============================================================================
// FUNCION PRINCIPAL Y MENU DE USUARIO [PERSONA 1]
// ============================================================================
int main() {
    // Fijar semilla pseudoaleatoria reproducible para la evaluacion del docente
    srand(SEMILLA);

    // Instanciacion de objetos del sistema
    Mercado mercado; // Asociacion

    // Cargas de la Panaderia:
    // Carga critica: Refrigeracion (no se apaga)
    CargaCritica refrigeracion("Refrigeracion", POTENCIA_REFRIGERACION);
    // Cargas aplazables: Horno rotativo y Amasadora industrial con sus horas minimas requeridas
    CargaAplazable horno("Horno Rotativo", POTENCIA_HORNO, HORAS_MIN_HORNO);
    CargaAplazable amasadora("Amasadora", POTENCIA_AMASADORA, HORAS_MIN_AMASADORA);

    // Negocio: integra medidor (composicion) y mercado (asociacion)
    Negocio panaderia("Panaderia La Especial", &mercado);

    // Agregacion: se anaden las cargas creadas independientemente
    panaderia.agregarCarga(&refrigeracion);
    panaderia.agregarCarga(&horno);
    panaderia.agregarCarga(&amasadora);

    panaderia.mostrarConfiguracion();

    // Variables de lectura manual (inicializadas con valores por defecto)
    int hora = 12;
    float precio = 550.0;
    float embalses = 45.0;
    bool haySimulacion = false;
    int opcion = -1;

    // Menu interactivo mediante ciclo do-while
    do {
        cout << "\n=========== MENU WATTCONTROL (PANADERIA) ===========" << endl;
        cout << "1. Ingresar lectura manual (hora, embalses, precio)" << endl;
        cout << "2. Evaluar decision de cada carga" << endl;
        cout << "3. Simular un dia completo (24 h)" << endl;
        cout << "4. Ver resumen de la ultima simulacion" << endl;
        cout << "0. Salir" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
        // CASO 1: Ingreso de datos con validacion individual mediante do-while
        case 1:
            // Validacion de la hora (0 a 23)
            do {
                cout << "Hora del dia (0 a 23): ";
                cin >> hora;
                if (hora < 0 || hora > 23) {
                    cout << "  [ERROR] La hora debe estar en el rango de 0 a 23." << endl;
                }
            } while (hora < 0 || hora > 23);

            // Validacion del nivel de embalses (0 a 100 %)
            do {
                cout << "Nivel de embalses nacional (0 a 100 %): ";
                cin >> embalses;
                if (embalses < 0.0 || embalses > 100.0) {
                    cout << "  [ERROR] El porcentaje de embalses debe estar entre 0 y 100." << endl;
                }
            } while (embalses < 0.0 || embalses > 100.0);

            // Validacion del precio de bolsa (> 0 COP/kWh)
            do {
                cout << "Precio de bolsa (COP/kWh, mayor a 0): ";
                cin >> precio;
                if (precio <= 0.0) {
                    cout << "  [ERROR] El precio de energia debe ser un valor positivo." << endl;
                }
            } while (precio <= 0.0);

            cout << ">> Lectura manual registrada y validada correctamente <<" << endl;
            break;

        // CASO 2: Evaluacion polimorfica de las decisiones con la lectura actual
        case 2:
            panaderia.evaluarManual(hora, precio, embalses);
            break;

        // CASO 3: Simulacion horaria de 24 horas
        case 3:
            panaderia.simularDia();
            haySimulacion = true;
            break;

        // CASO 4: Resumen de resultados, alertas y ahorro
        case 4:
            if (haySimulacion) {
                panaderia.resumen();
            } else {
                cout << "[AVISO] Aun no se ha ejecutado ninguna simulacion. Seleccione la opcion 3 primero." << endl;
            }
            break;

        // CASO 0: Finalizar ejecucion
        case 0:
            cout << "Finalizando ejecucion de WattControl. Buen dia." << endl;
            break;

        default:
            cout << "[ERROR] Opcion no valida. Intente nuevamente." << endl;
            break;
        }

    } while (opcion != 0);

    return 0;
}
