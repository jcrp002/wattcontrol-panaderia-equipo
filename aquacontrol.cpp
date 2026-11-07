// =============================================================
// AquaControl v2 - Gestor de energia de un edificio (El Nino 2026)
// Paso 8: resumen y ahorro
// =============================================================
// --- Librerias: codigo que ya existe y que se trae al programa -------------
// iostream: cout (escribir en pantalla) y cin (leer del teclado)
#include <iostream>
// iomanip: setw (ancho de columna) y setprecision (cantidad de decimales)
#include <iomanip>
// string: el tipo string, para guardar textos
#include <string>
// cstdlib: rand() y srand(), para generar numeros al azar
#include <cstdlib>

// Permite escribir cout en vez de std::cout (todo lo de la libreria esta en "std")
using namespace std;

// --- Directivas del preprocesador ------------------------------------------
// Antes de compilar, el preprocesador cambia cada nombre por su valor en todo el archivo.
// No llevan = ni ; al final.
// Horas de la simulacion: un dia
#define HORAS_DIA 24
// Cuantos equipos puede tener el edificio, como maximo (tamano del arreglo del paso 4)
#define MAX_EQUIPOS 5
// Semilla de rand(): con la misma semilla, todos obtienen los mismos precios al azar
#define SEMILLA 2026

// --- Constantes: todos los valores del problema, en un solo lugar ----------
// const = no se pueden cambiar despues. Si cambia un valor, se cambia solo aqui.
// kW de la bomba de agua
const float POTENCIA_BOMBA = 5.5;
// kW de la iluminacion de las zonas comunes
const float POTENCIA_LUCES = 1.2;
// % que sube el tanque en una hora de bombeo
const float SUBIDA_HORA = 12.0;
// % del tanque al empezar la simulacion
const float NIVEL_INICIAL = 50.0;
// %: si el tanque esta por debajo, se bombea siempre, para no quedarse sin agua
const float NIVEL_CRITICO = 20.0;
// %: con energia barata, el tanque se llena hasta aqui
const float NIVEL_OBJETIVO = 80.0;
// COP/kWh: si el precio esta por debajo, la energia esta barata
const float PRECIO_BARATO = 600.0;
// COP/kWh: si el precio pasa de aqui, se detiene la simulacion (paso 7)
const float PRECIO_ESCASEZ = 1500.0;
// La hora pico empieza a las 18 h (6 pm)...
const int PICO_INICIO = 18;
// ...y termina a las 20 h (la hora 20 esta incluida: hasta las 9 pm)
const int PICO_FIN = 20;
// Las luces se prenden desde las 18 h (6 pm)...
const int NOCHE_INICIO = 18;
// ...hasta las 5 h (la hora 5 esta incluida)
const int NOCHE_FIN = 5;
// 1 de cada 20 lecturas del precio falla (paso 7)
const int FALLA_CADA = 20;

// =============================================================
// Clase base Equipo: lo que tienen en comun todos los equipos
// =============================================================
// class Equipo { ... }; define un molde. Cada equipo real (bomba, luces) se crea con el
// molde de su clase hija, que hereda todo lo de aqui
class Equipo {
// protected: estos atributos los ven esta clase y sus hijas (Bomba, Iluminacion),
// pero no el resto del programa
protected:
    // El nombre del equipo: "Bomba", "Luces"
    string nombre;
    // La potencia en kW: 5.5 la bomba, 1.2 las luces
    float potenciaKW;
    // true si esta encendido en esta hora; false si esta apagado
    bool encendida;
    // Cuantas horas ha estado encendido (para el resumen)
    int horasUso;

// public: lo que puede usar cualquier parte del programa
public:
    // Constructor: se ejecuta al crear el equipo. Recibe el nombre (n) y la potencia (p).
    // Ejemplo: Equipo("Bomba", 5.5)
    Equipo(string n, float p) {
        // Guarda el nombre recibido
        nombre = n;
        // Guarda la potencia recibida
        potenciaKW = p;
        // Todo equipo empieza apagado...
        encendida = false;
        // ...y sin horas de uso
        horasUso = 0;
    }

    // Destructor virtual: obligatorio en una clase base con metodos virtuales.
    // Asegura que, al borrar un equipo, se borre bien aunque se use un puntero Equipo*
    virtual ~Equipo() {}

    // Polimorfismo: cada clase hija decide a su manera si se enciende.
    // virtual = cada hija lo puede escribir a su manera.
    // "= 0" = metodo virtual puro: Equipo no lo implementa; cada hija debe hacerlo.
    // Por eso Equipo es una clase ABSTRACTA: no se pueden crear objetos Equipo directamente
    virtual bool debeEncender(int hora, float precio, float nivel) = 0;

    // Enciende el equipo durante esta hora
    void encender() {
        // Queda encendido...
        encendida = true;
        // ...y suma una hora de uso (++ suma 1)
        horasUso++;
    }

    // Apaga el equipo durante esta hora
    void apagar() {
        encendida = false;
    }

    // Costo de esta hora: kW x precio si esta encendido; 0 si esta apagado
    float costoHora(float precio) {
        // Si esta encendido...
        if (encendida) {
            // ...el costo es la potencia por el precio: 5.5 kW x 450 COP/kWh = 2475 COP
            return potenciaKW * precio;
        }
        // Si esta apagado no gasta nada
        return 0;
    }

    // Getters: metodos que devuelven el valor de un atributo protegido,
    // para que el resto del programa lo pueda leer (pero no cambiar)
    string getNombre() { return nombre; }
    float getPotencia() { return potenciaKW; }
    bool estaEncendido() { return encendida; }
    int getHorasUso() { return horasUso; }
};

// =============================================================
// Herencia: Bomba ES UN Equipo
// =============================================================
// ": public Equipo" = Bomba hereda todo lo de Equipo (nombre, potencia, encender, apagar...)
class Bomba : public Equipo {
public:
    // Constructor de Bomba. ": Equipo(n, p)" le pasa el nombre y la potencia al
    // constructor de Equipo, que es el que los guarda. Las llaves {} quedan vacias
    // porque Bomba no tiene nada mas que guardar
    Bomba(string n, float p) : Equipo(n, p) {}

    // La regla de la bomba. override = reemplaza el metodo virtual puro de Equipo
    bool debeEncender(int hora, float precio, float nivel) override {
        // horaPico es true si la hora esta entre 18 y 20 (&& = "y": las dos condiciones)
        bool horaPico = hora >= PICO_INICIO && hora <= PICO_FIN;
        // Bombea si el tanque esta por debajo del 20 % (siempre, para no quedarse sin agua)
        // || ("o") si NO es hora pico (!horaPico) y la energia esta barata y el tanque
        // esta por debajo del 80 %. return devuelve true (bombear) o false (esperar)
        return nivel < NIVEL_CRITICO ||
               (!horaPico && precio < PRECIO_BARATO && nivel < NIVEL_OBJETIVO);
    }
};

// =============================================================
// Herencia: Iluminacion ES UN Equipo (carga critica de noche)
// =============================================================
// Iluminacion tambien hereda todo lo de Equipo
class Iluminacion : public Equipo {
public:
    // Le pasa el nombre y la potencia al constructor de Equipo
    Iluminacion(string n, float p) : Equipo(n, p) {}

    // La regla de las luces: no se apagan por precio, solo por la hora.
    // Recibe precio y nivel aunque no los use, porque debe tener la misma forma
    // que debeEncender de Equipo
    bool debeEncender(int hora, float precio, float nivel) override {
        // Prende si es de noche: desde las 18 h || ("o") hasta las 5 h
        return hora >= NOCHE_INICIO || hora <= NOCHE_FIN;
    }
};

// =============================================================
// Tanque: parte del Edificio (composicion)
// =============================================================
class Tanque {
// private: solo los metodos del Tanque pueden cambiar el nivel
private:
    // % de llenado: 0 = vacio, 100 = lleno
    float nivel;

public:
    // Constructor: el tanque empieza con el nivel que se le indique (50 %)
    Tanque(float nivelInicial) {
        nivel = nivelInicial;
    }

    // Getter: devuelve el nivel actual
    float getNivel() { return nivel; }

    // El edificio gasta agua: el nivel baja
    void consumir(float porcentaje) {
        // Resta lo que se gasto
        nivel = nivel - porcentaje;
        // El nivel nunca queda por debajo de 0
        if (nivel < 0) nivel = 0;
    }

    // La bomba llena el tanque: el nivel sube
    void llenar(float porcentaje) {
        // Suma lo que se bombeo
        nivel = nivel + porcentaje;
        // El nivel nunca pasa de 100
        if (nivel > 100) nivel = 100;
    }
};

// =============================================================
// Mercado: da el precio de bolsa de cada hora (asociacion)
// =============================================================
class Mercado {
public:
    // Precio al azar dentro del rango real de la franja (XM, 21-27 sep 2026)
    float precioHora(int hora) {
        // El precio minimo y maximo de la franja de esta hora
        int minimo, maximo;
        // De 0 a 6 h: madrugada
        if (hora <= 6) {
            minimo = 398; maximo = 640;
        // De 7 a 14 h: manana y mediodia
        } else if (hora <= 14) {
            minimo = 456; maximo = 970;
        // De 15 a 17 h: tarde
        } else if (hora <= 17) {
            minimo = 596; maximo = 1040;
        // De 18 a 20 h: hora pico
        } else if (hora <= 20) {
            minimo = 705; maximo = 1612;
        // De 21 a 23 h: noche
        } else {
            minimo = 565; maximo = 970;
        }
        // rand() da un numero entero al azar. % (maximo - minimo + 1) lo deja entre 0 y
        // la diferencia; al sumarle el minimo queda entre minimo y maximo.
        // Ejemplo en la madrugada: 398 + (un numero de 0 a 242) = de 398 a 640
        return minimo + rand() % (maximo - minimo + 1);
    }

    // Agua que gasta el edificio en esa hora (% del tanque)
    float consumoAgua(int hora) {
        // Madrugada: casi nadie usa agua
        if (hora <= 6) return 2;
        // Manana y mediodia: se usa bastante
        if (hora <= 14) return 5;
        // Tarde
        if (hora <= 17) return 4;
        // Hora pico: la gente llega a la casa y gasta mas
        if (hora <= 20) return 7;
        // Noche
        return 4;
    }
};

// =============================================================
// Edificio: junta todo
// =============================================================
class Edificio {
private:
    // El nombre del edificio: "Torres del Rio"
    string nombre;
    // COMPOSICION: el tanque es un atributo objeto. Nace y muere con el edificio
    Tanque tanque;
    // AGREGACION: un arreglo de punteros (*) a Equipo, con espacio para MAX_EQUIPOS (5).
    // Los equipos se crean afuera y aqui solo se guarda su direccion
    Equipo* equipos[MAX_EQUIPOS];
    // Cuantos equipos se han agregado (y en que posicion va el siguiente)
    int numEquipos;
    // ASOCIACION: un puntero al mercado. El edificio no lo crea: solo lo conoce
    // y le pide los precios
    Mercado* mercado;

    // Resultados de la ultima simulacion, para mostrarlos en el resumen (opcion 4)
    // costoCon: lo que se paga con el gestor; costoSin: sin el gestor;
    // nivelMinimo: el nivel mas bajo que tuvo el tanque
    float costoCon, costoSin, nivelMinimo;
    // horasSimuladas: cuantas horas alcanzo a simular; fallas: lecturas que fallaron
    int horasSimuladas, fallas;
public:
    // Constructor: recibe el nombre y la direccion del mercado (Mercado* m).
    // ": tanque(NIVEL_INICIAL)" crea el tanque del edificio con 50 %
    Edificio(string n, Mercado* m) : tanque(NIVEL_INICIAL) {
        // Guarda el nombre
        nombre = n;
        // Guarda la direccion del mercado: desde aqui el edificio lo conoce
        mercado = m;
        // Todavia no tiene equipos
        numEquipos = 0;
        // Los resultados empiezan en 0, porque todavia no se ha simulado
        costoCon = 0; costoSin = 0; nivelMinimo = NIVEL_INICIAL;
        horasSimuladas = 0; fallas = 0;
    }

    // Agrega un equipo al edificio. Recibe su direccion (Equipo* e)
    void agregarEquipo(Equipo* e) {
        // Solo si todavia hay espacio en el arreglo
        if (numEquipos < MAX_EQUIPOS) {
            // Guarda el equipo en la siguiente posicion libre
            equipos[numEquipos] = e;
            // Ahora hay un equipo mas
            numEquipos++;
        }
    }

    // Muestra el edificio, el nivel del tanque y sus equipos
    void mostrar() {
        cout << "Edificio " << nombre << " | tanque al " << tanque.getNivel() << " %" << endl;
        // Recorre los equipos: i va de 0 hasta numEquipos - 1
        for (int i = 0; i < numEquipos; i++) {
            // equipos[i]->getNombre(): -> se usa para llamar un metodo a traves de un puntero
            cout << "  " << equipos[i]->getNombre() << " (" << equipos[i]->getPotencia() << " kW)" << endl;
        }
    }

    // Que haria cada equipo con una lectura (hora, precio, nivel)
    void evaluar(int hora, float precio, float nivel) {
        // Recorre los equipos
        for (int i = 0; i < numEquipos; i++) {
            // Polimorfismo: la misma llamada; la bomba y las luces responden cada una con su regla
            bool on = equipos[i]->debeEncender(hora, precio, nivel);
            // setw(12) ocupa 12 espacios; left alinea a la izquierda y right vuelve a la derecha.
            // (on ? "ENCENDER" : "APAGAR / ESPERAR"): si on es true escribe ENCENDER; si no, lo otro
            cout << "  " << setw(12) << left << equipos[i]->getNombre() << right
                 << (on ? "ENCENDER" : "APAGAR / ESPERAR") << endl;
        }
    }

    // Simula 24 horas, con gestor y sin gestor
    void simularDia() {
        // Un segundo tanque con el mismo nivel: es el mismo edificio, pero SIN gestor
        Tanque tanqueSin(tanque.getNivel());
        // Los resultados empiezan de nuevo en 0 (son atributos: se guardan para el resumen)
        costoCon = 0; costoSin = 0; fallas = 0; horasSimuladas = 0;
        // El nivel minimo empieza en el nivel actual del tanque
        nivelMinimo = tanque.getNivel();

        // fixed y setprecision(0): los numeros se muestran sin decimales
        cout << fixed << setprecision(0);
        // El encabezado de la tabla; setw(5) ocupa 5 espacios para alinear las columnas
        cout << setw(5) << "Hora" << setw(8) << "Precio" << setw(8) << "Nivel";
        // Una columna por cada equipo, con su nombre
        for (int i = 0; i < numEquipos; i++) cout << setw(12) << equipos[i]->getNombre();
        cout << setw(9) << "Costo" << endl;

        // Repite para cada hora del dia: h = 0, 1, 2, ..., 23
        for (int h = 0; h < HORAS_DIA; h++) {
            // ASOCIACION: el edificio le pide al mercado el precio de esta hora
            float precio = mercado->precioHora(h);
            // Y cuanta agua se gasta en esta hora
            float consumo = mercado->consumoAgua(h);

            // Sin gestor: las luces prendidas todo el dia y la bomba por nivel, a cualquier hora
            // Las luces siempre gastan: potencia x precio
            costoSin += POTENCIA_LUCES * precio;
            // La bomba prende si el tanque esta por debajo del 80 %, sin mirar el precio...
            if (tanqueSin.getNivel() < NIVEL_OBJETIVO) {
                // ...y se paga su energia de esta hora...
                costoSin += POTENCIA_BOMBA * precio;
                // ...y el tanque sube
                tanqueSin.llenar(SUBIDA_HORA);
            }
            // El edificio gasta agua de ese tanque
            tanqueSin.consumir(consumo);

            // break: precio de escasez -> se detiene la simulacion
            // Si el precio pasa de 1500 COP/kWh...
            if (precio > PRECIO_ESCASEZ) {
                // ...lo avisa en la tabla...
                cout << setw(5) << h << setw(8) << precio
                     << "  PRECIO DE ESCASEZ: se detiene la simulacion" << endl;
                // ...y break SALE del for: no se simulan las horas que faltan
                break;
            }
            // Si llego aqui, esta hora si se simula: se cuenta
            horasSimuladas++;

            // continue: falla la lectura del precio -> esa hora todo queda apagado
            // rand() % FALLA_CADA da un numero de 0 a 19; si da 0 (1 de cada 20 veces)...
            if (rand() % FALLA_CADA == 0) {
                // ...por seguridad, todos los equipos se apagan...
                for (int i = 0; i < numEquipos; i++) equipos[i]->apagar();
                // ...el edificio igual gasta agua...
                tanque.consumir(consumo);
                // ...se cuenta la falla...
                fallas++;
                // ...se avisa en la tabla...
                cout << setw(5) << h << setw(8) << "--" << setw(8) << tanque.getNivel()
                     << "  falla de lectura: todo apagado" << endl;
                // ...y continue SALTA a la siguiente hora: lo que sigue no se ejecuta en esta
                continue;
            }

            // Con gestor: cada equipo decide (polimorfismo)
            // El costo de esta hora empieza en 0
            float costoHora = 0;
            // Escribe la hora y el precio en la tabla
            cout << setw(5) << h << setw(8) << precio;
            // Recorre los equipos
            for (int i = 0; i < numEquipos; i++) {
                // Polimorfismo: cada equipo aplica SU regla con la hora, el precio y el nivel
                if (equipos[i]->debeEncender(h, precio, tanque.getNivel())) {
                    // Si su regla dice que si, se enciende
                    equipos[i]->encender();
                } else {
                    // Si no, se apaga
                    equipos[i]->apagar();
                }
                // Suma lo que gasto este equipo en esta hora (0 si esta apagado)
                costoHora += equipos[i]->costoHora(precio);
            }
            // equipos[0] es la bomba: si quedo encendida, el tanque sube
            if (equipos[0]->estaEncendido()) tanque.llenar(SUBIDA_HORA);
            // El edificio gasta agua
            tanque.consumir(consumo);
            // Si el tanque bajo mas que nunca, se guarda ese nivel como el minimo
            if (tanque.getNivel() < nivelMinimo) nivelMinimo = tanque.getNivel();
            // Suma el costo de esta hora al costo del dia
            costoCon += costoHora;

            // Termina la fila de la tabla: el nivel, ON u off por equipo y el costo de la hora
            cout << setw(8) << tanque.getNivel();
            for (int i = 0; i < numEquipos; i++) cout << setw(12) << (equipos[i]->estaEncendido() ? "ON" : "off");
            cout << setw(9) << costoHora << endl;
        }
        // Los resultados quedaron guardados en los atributos: se ven con la opcion 4
        cout << "Simulacion terminada. Vea el resumen con la opcion 4." << endl;
    }

    // Opcion 4: resumen y ahorro
    void resumen() {
        // Numeros sin decimales
        cout << fixed << setprecision(0);
        cout << "Edificio " << nombre << endl;
        // Cuantas horas se simularon y cuantas lecturas fallaron
        cout << "Horas simuladas: " << horasSimuladas << " de " << HORAS_DIA
             << " | fallas de lectura: " << fallas << endl;
        // Una linea por equipo: horas encendido y energia gastada (horas x kW = kWh)
        for (int i = 0; i < numEquipos; i++) {
            cout << "  " << setw(12) << left << equipos[i]->getNombre() << right
                 << equipos[i]->getHorasUso() << " h, "
                 << equipos[i]->getHorasUso() * equipos[i]->getPotencia() << " kWh" << endl;
        }
        // El nivel mas bajo del tanque: confirma que el edificio nunca se quedo sin agua
        cout << "Nivel minimo del tanque: " << nivelMinimo << " %" << endl;
        cout << "Costo con gestor: $" << costoCon << endl;
        cout << "Costo sin gestor: $" << costoSin << endl;
        // El ahorro: lo que se paga sin gestor menos lo que se paga con gestor
        cout << "Ahorro:           $" << costoSin - costoCon;
        // El ahorro en porcentaje, con 1 decimal. Solo si costoSin > 0, para no dividir por 0
        if (costoSin > 0) cout << " (" << setprecision(1) << (costoSin - costoCon) / costoSin * 100 << " %)";
        cout << endl;
    }
};

// main: el programa empieza a ejecutarse aqui
int main() {
    // srand fija el punto de partida de rand(): misma semilla = mismos precios para todos
    srand(SEMILLA);

    // El mercado existe por su cuenta: no es parte del edificio
    Mercado mercado;
    // Crea los dos equipos
    Bomba bomba("Bomba", POTENCIA_BOMBA);
    Iluminacion luces("Luces", POTENCIA_LUCES);
    // Crea el edificio y le pasa la direccion del mercado: asociacion
    Edificio edificio("Torres del Rio", &mercado);
    // Le agrega los equipos al edificio (agregacion)
    edificio.agregarEquipo(&bomba);
    edificio.agregarEquipo(&luces);
    edificio.mostrar();

    // Lectura manual: empieza con valores de ejemplo
    int hora = 12;
    float nivel = NIVEL_INICIAL, precio = 500;
    // true cuando ya se simulo un dia (para poder ver el resumen)
    bool haySimulacion = false;
    // La opcion que escoja el usuario en el menu
    int opcion;

    // do-while: el menu se muestra al menos una vez y se repite mientras la opcion no sea 0
    do {
        cout << endl << "===== AquaControl v2 =====" << endl;
        cout << "1. Ingresar lectura manual (hora, nivel, precio)" << endl;
        cout << "2. Evaluar decision de cada equipo" << endl;
        cout << "3. Simular un dia completo (24 h)" << endl;
        cout << "4. Ver resumen de la ultima simulacion" << endl;
        cout << "0. Salir" << endl;
        cout << "Opcion: ";
        // cin lee del teclado lo que escriba el usuario y lo guarda en opcion
        cin >> opcion;

        // switch: segun el valor de opcion, ejecuta un caso u otro
        switch (opcion) {
        // Opcion 1: pedir la lectura, validando cada dato
        case 1:
            // Validacion con do-while: pide la hora y la vuelve a pedir mientras sea invalida
            do {
                cout << "Hora (0 a 23): ";
                cin >> hora;
                // || = "o": invalida si es menor que 0 o mayor que 23
                if (hora < 0 || hora > 23) cout << "  Hora invalida." << endl;
            } while (hora < 0 || hora > 23);
            // Lo mismo con el nivel: debe estar entre 0 y 100
            do {
                cout << "Nivel del tanque (0 a 100 %): ";
                cin >> nivel;
                if (nivel < 0 || nivel > 100) cout << "  Nivel invalido." << endl;
            } while (nivel < 0 || nivel > 100);
            // Y con el precio: debe ser mayor que 0
            do {
                cout << "Precio de bolsa (COP/kWh): ";
                cin >> precio;
                if (precio <= 0) cout << "  El precio debe ser mayor que 0." << endl;
            } while (precio <= 0);
            cout << "Lectura guardada." << endl;
            // break: termina este caso y sale del switch (sin el, seguiria con el caso 2)
            break;
        // Opcion 2: que haria cada equipo con la lectura guardada
        case 2:
            cout << "Hora " << hora << " | nivel " << nivel << " % | precio " << precio << endl;
            edificio.evaluar(hora, precio, nivel);
            break;
        // Opcion 3: simular las 24 horas y recordar que ya hay simulacion
        case 3:
            edificio.simularDia();
            haySimulacion = true;
            break;
        // Opcion 4: el resumen, solo si ya se simulo un dia
        case 4:
            if (haySimulacion) edificio.resumen();
            else cout << "Primero simule un dia con la opcion 3." << endl;
            break;
        // Opcion 0: salir
        case 0:
            cout << "Hasta luego." << endl;
            break;
        // default: cualquier otro numero
        default:
            cout << "Opcion invalida." << endl;
        }
    // Vuelve a mostrar el menu mientras la opcion sea distinta de 0 (!= significa "distinto de")
    } while (opcion != 0);

    return 0;
}
