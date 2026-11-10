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
// CLASE Negocio: INTEGRACION Y RELACIONES DE POO [PERSONA 2]
// ============================================================================
// Modela la Panaderia que gestiona sus cargas energeticas.
class Negocio {
private:
    string nombre;

    // 1. COMPOSICION: El medidor es un objeto directo; su ciclo de vida depende de Negocio.
    Medidor medidor;

    // 2. AGREGACION: Arreglo de punteros polimorficos a Carga. Las cargas existen fuera.
    Carga* cargas[MAX_CARGAS];
    int numCargas;

    // 3. ASOCIACION: Puntero al Mercado. El negocio conoce al mercado para pedir precios.
    Mercado* mercado;

    // Variables de control de la ultima simulacion
    float costoConGestor;
    float costoSinGestor;
    float energiaConGestor;
    float energiaSinGestor;
    int horasSimuladas;
    int fallasLectura;

public:
    // Constructor
    Negocio(string n, Mercado* m) {
        nombre = n;
        mercado = m;
        numCargas = 0;
        costoConGestor = 0.0;
        costoSinGestor = 0.0;
        energiaConGestor = 0.0;
        energiaSinGestor = 0.0;
        horasSimuladas = 0;
        fallasLectura = 0;
    }

    // Metodo de Agregacion: anade una carga externa al negocio
    void agregarCarga(Carga* c) {
        if (numCargas < MAX_CARGAS) {
            cargas[numCargas] = c;
            numCargas++;
        }
    }

    // Presentacion inicial del negocio
    void mostrarConfiguracion() {
        cout << "=====================================================" << endl;
        cout << " Negocio: " << nombre << " (Panaderia)" << endl;
        cout << " Potencia Contratada: " << POTENCIA_CONTRATADA << " kW" << endl;
        cout << " Equipos registrados:" << endl;
        for (int i = 0; i < numCargas; i++) {
            cout << "  - " << setw(18) << left << cargas[i]->getNombre()
                 << right << setw(5) << cargas[i]->getPotencia() << " kW" << endl;
        }
        cout << "=====================================================" << endl;
    }

    // Evaluacion manual polimorfica (Opcion 2 del menu)
    void evaluarManual(int hora, float precio, float embalses) {
        cout << "\nEvaluando decision de cada equipo para la lectura:" << endl;
        cout << "  Hora: " << hora << ":00 | Precio: $" << precio 
             << " COP/kWh | Embalses: " << embalses << " %" << endl;
        cout << "-----------------------------------------------------" << endl;
        
        float potenciaDemandada = 0.0;
        for (int i = 0; i < numCargas; i++) {
            // LLAMADA POLIMORFICA: cada carga evalua su propia regla
            bool encender = cargas[i]->debeEncender(hora, precio, embalses);
            cout << "  " << setw(18) << left << cargas[i]->getNombre() << right;
            if (encender) {
                cout << "[ENCENDER]" << endl;
                potenciaDemandada += cargas[i]->getPotencia();
            } else {
                cout << "[APAGAR / APLAZAR]" << endl;
            }
        }
        cout << "  Potencia total estimada: " << potenciaDemandada << " kW / " 
             << POTENCIA_CONTRATADA << " kW contratados." << endl;
    }

    // REGLA 6: Control de potencia contratada.
    // Si la potencia acumulada de los equipos encendidos sobrepasa la contratada,
    // se apaga la carga aplazable de mayor potencia (Horno rotativo) para evitar disparar el breaker.
    void aplicarControlPotencia(int hora) {
        float sumaPotencia = 0.0;
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->estaEncendido()) {
                sumaPotencia += cargas[i]->getPotencia();
            }
        }

        if (sumaPotencia > POTENCIA_CONTRATADA) {
            cout << " [ALERTA HORA " << setw(2) << hora << "] Sobrecarga detectada (" 
                 << sumaPotencia << " kW > " << POTENCIA_CONTRATADA 
                 << " kW). Desconectando carga aplazable mas pesada..." << endl;
            
            // Buscar la carga aplazable encendida con mayor consumo y apagarla
            int idxMayor = -1;
            float maxPot = 0.0;
            // Omitimos la primera (cargas[0] es critica)
            for (int i = 1; i < numCargas; i++) {
                if (cargas[i]->estaEncendido() && cargas[i]->getPotencia() > maxPot) {
                    maxPot = cargas[i]->getPotencia();
                    idxMayor = i;
                }
            }
            if (idxMayor != -1) {
                cargas[idxMayor]->apagar();
                cout << "   -> " << cargas[idxMayor]->getNombre() 
                     << " apagado por proteccion de potencia." << endl;
            }
        }
    }

    
    // Simulacion de 24 horas continuas (Opcion 3 del menu)
    void simularDia() {
        // Reiniciar medidores y estados de las cargas
        medidor.reiniciar();
        costoConGestor = 0.0;
        costoSinGestor = 0.0;
        energiaConGestor = 0.0;
        energiaSinGestor = 0.0;
        horasSimuladas = 0;
        fallasLectura = 0;

        for (int i = 0; i < numCargas; i++) {
            cargas[i]->reiniciar();
        }

        cout << fixed << setprecision(0);
        cout << "\n================ SIMULACION 24 HORAS: WATTCONTROL PANADERIA ================" << endl;
        cout << setw(5) << "Hora" << setw(8) << "Precio" << setw(9) << "Embalse";
        for (int i = 0; i < numCargas; i++) {
            cout << setw(13) << cargas[i]->getNombre().substr(0, 11);
        }
        cout << setw(10) << "Costo(COP)" << endl;
        cout << "----------------------------------------------------------------------------" << endl;

        for (int h = 0; h < HORAS_DIA; h++) {
            // ASOCIACION: Consulta de precios y embalses al Mercado
            float precio = mercado->precioHora(h);
            float embalses = mercado->nivelEmbalses(h);

            // Escenario SIN GESTOR: todo encendido indiscriminadamente las 24 horas
            for (int i = 0; i < numCargas; i++) {
                costoSinGestor += cargas[i]->getPotencia() * precio;
                energiaSinGestor += cargas[i]->getPotencia();
            }

            // INSTRUCCION DE CONTROL BREAK: Emergencia por escasez extrema
            if (precio > PRECIO_ESCASEZ) {
                cout << setw(5) << h << setw(8) << precio 
                     << "   [BREAK] PRECIO DE ESCASEZ EXTREMA: Operacion detenida por emergencia." << endl;
                break;
            }

            horasSimuladas++;

            // INSTRUCCION DE CONTROL CONTINUE: Falla en telemedicion
            if (rand() % FALLA_CADA == 0) {
                fallasLectura++;
                // Modo seguro: solo opera la carga critica (refrigeracion); aplazables apagadas
                cargas[0]->encender();
                for (int i = 1; i < numCargas; i++) {
                    cargas[i]->apagar();
                }
                float costoFalla = cargas[0]->costoHora(precio);
                costoConGestor += costoFalla;
                energiaConGestor += cargas[0]->getPotencia();
                medidor.registrarConsumo(cargas[0]->getPotencia(), costoFalla);

                cout << setw(5) << h << setw(8) << "--" << setw(9) << "--"
                     << "   [CONTINUE] Falla sensor: Solo refrigeracion activa." << endl;
                continue;
            }

            // Escenario CON GESTOR INTELIGENTE:
            // 1. Evaluacion polimorfica individual
            for (int i = 0; i < numCargas; i++) {
                if (cargas[i]->debeEncender(h, precio, embalses)) {
                    cargas[i]->encender();
                } else {
                    cargas[i]->apagar();
                }
            }

            // 2. Aplicacion de la Regla 6: Control de potencia contratada
            aplicarControlPotencia(h);

            // 3. Calculo de costos de la hora
            float costoHoraActual = 0.0;
            float energiaHoraActual = 0.0;
            for (int i = 0; i < numCargas; i++) {
                if (cargas[i]->estaEncendido()) {
                    costoHoraActual += cargas[i]->costoHora(precio);
                    energiaHoraActual += cargas[i]->getPotencia();
                }
            }

            costoConGestor += costoHoraActual;
            energiaConGestor += energiaHoraActual;
            medidor.registrarConsumo(energiaHoraActual, costoHoraActual);

            // Impresion de fila horaria
            cout << setw(5) << h << setw(8) << precio << setw(8) << embalses << "%";
            for (int i = 0; i < numCargas; i++) {
                cout << setw(13) << (cargas[i]->estaEncendido() ? "ON" : "off");
            }
            cout << setw(10) << costoHoraActual << endl;
        }

        cout << "----------------------------------------------------------------------------" << endl;
        cout << "Simulacion completada con exito. Consulte el resumen con la opcion 4." << endl;
    }

    // Resumen analitico y calculo financiero de ahorro (Opcion 4 del menu)
    void resumen() {
        cout << fixed << setprecision(0);
        cout << "\n============================================================" << endl;
        cout << "        RESUMEN FINANCIERO Y ENERGETICO - WATTCONTROL        " << endl;
        cout << "                   Panaderia Central                        " << endl;
        cout << "============================================================" << endl;
        cout << "Horas simuladas: " << horasSimuladas << " de " << HORAS_DIA 
             << " | Fallas de medicion controladas: " << fallasLectura << endl;
        cout << "\n--- Detalle por Equipo ---" << endl;
        
        bool produccionCompleta = true;
        for (int i = 0; i < numCargas; i++) {
            float kwh = cargas[i]->getHorasUso() * cargas[i]->getPotencia();
            cout << " - " << setw(18) << left << cargas[i]->getNombre() << right
                 << setw(3) << cargas[i]->getHorasUso() << " h operadas ("
                 << setw(5) << kwh << " kWh consumidos)" << endl;

            // Verificacion de cumplimiento de horas minimas requeridas
            if (i > 0) { // Cargas aplazables
                CargaAplazable* cap = dynamic_cast<CargaAplazable*>(cargas[i]);
                if (cap && cargas[i]->getHorasUso() < cap->getHorasMinimas()) {
                    cout << "   [ALERTA PRODUCCION] " << cargas[i]->getNombre() 
                         << " no alcanzo el minimo de " << cap->getHorasMinimas() << " horas." << endl;
                    produccionCompleta = false;
                }
            }
        }

        if (produccionCompleta) {
            cout << "\n>> Cumplimiento operativo: TANDAS DE PAN COMPLETADAS AL 100% <<" << endl;
        } else {
            cout << "\n>> Cumplimiento operativo: ALERTA DE PRODUCCION PARCIAL <<" << endl;
        }

        // Metricas registradas por el Medidor (Composicion)
        cout << "\n--- Balance Energetico y Economico ---" << endl;
        cout << "Energia sin gestor:  " << setw(8) << energiaSinGestor << " kWh" << endl;
        cout << "Energia con gestor:  " << setw(8) << medidor.getEnergiaTotal() << " kWh" << endl;
        cout << "Costo sin gestor:   $" << setw(8) << costoSinGestor << " COP" << endl;
        cout << "Costo con gestor:   $" << setw(8) << costoConGestor << " COP" << endl;
        
        float ahorroCOP = costoSinGestor - costoConGestor;
        float ahorroPorc = (costoSinGestor > 0) ? (ahorroCOP / costoSinGestor * 100.0) : 0.0;

        cout << "------------------------------------------------------------" << endl;
        cout << "AHORRO ECONOMICO:   $" << setw(8) << ahorroCOP << " COP (" 
             << setprecision(1) << ahorroPorc << " %)" << endl;
        cout << "============================================================" << endl;
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
