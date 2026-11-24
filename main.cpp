#include <iostream>
#include <cstdlib>
#include "Constantes.h"
#include "Mercado.h"
#include "CargaCritica.h"
#include "CargaAplazable.h"
#include "Negocio.h"

using namespace std;

// ============================================================================
// FUNCION PRINCIPAL Y MENU DE USUARIO
// ============================================================================
int main() {
    srand(SEMILLA);

    Mercado mercado;

    CargaCritica refrigeracion("Refrigeracion", POTENCIA_REFRIGERACION);
    CargaAplazable horno("Horno Rotativo", POTENCIA_HORNO, HORAS_MIN_HORNO);
    CargaAplazable amasadora("Amasadora", POTENCIA_AMASADORA, HORAS_MIN_AMASADORA);

    Negocio panaderia("Panaderia La Especial", &mercado);
    panaderia.agregarCarga(&refrigeracion);
    panaderia.agregarCarga(&horno);
    panaderia.agregarCarga(&amasadora);

    panaderia.mostrarConfiguracion();

    int hora = 12;
    float precio = 550.0f;
    float embalses = 45.0f;
    bool haySimulacion = false;
    int opcion = -1;

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
        case 1:
            // Validacion de la hora mediante constantes HORA_MIN_VALIDA y HORA_MAX_VALIDA
            do {
                cout << "Hora del dia (" << HORA_MIN_VALIDA << " a " << HORA_MAX_VALIDA << "): ";
                cin >> hora;
                if (hora < HORA_MIN_VALIDA || hora > HORA_MAX_VALIDA) {
                    cout << "  [ERROR] La hora debe estar entre " 
                         << HORA_MIN_VALIDA << " y " << HORA_MAX_VALIDA << "." << endl;
                }
            } while (hora < HORA_MIN_VALIDA || hora > HORA_MAX_VALIDA);

            // Validacion de embalses mediante constantes EMBALSE_MIN_VALIDO y EMBALSE_MAX_VALIDO
            do {
                cout << "Nivel de embalses (" << EMBALSE_MIN_VALIDO << " a " << EMBALSE_MAX_VALIDO << " %): ";
                cin >> embalses;
                if (embalses < EMBALSE_MIN_VALIDO || embalses > EMBALSE_MAX_VALIDO) {
                    cout << "  [ERROR] El porcentaje de embalses debe estar entre " 
                         << EMBALSE_MIN_VALIDO << " y " << EMBALSE_MAX_VALIDO << "." << endl;
                }
            } while (embalses < EMBALSE_MIN_VALIDO || embalses > EMBALSE_MAX_VALIDO);

            // Validacion de precio mayor que PRECIO_MIN_VALIDO
            do {
                cout << "Precio de bolsa (COP/kWh, mayor a " << PRECIO_MIN_VALIDO << "): ";
                cin >> precio;
                if (precio <= PRECIO_MIN_VALIDO) {
                    cout << "  [ERROR] El precio debe ser estrictamente mayor a " 
                         << PRECIO_MIN_VALIDO << "." << endl;
                }
            } while (precio <= PRECIO_MIN_VALIDO);

            cout << ">> Lectura manual registrada y validada correctamente <<" << endl;
            break;

        case 2:
            panaderia.evaluarManual(hora, precio, embalses);
            break;

        case 3:
            panaderia.simularDia();
            haySimulacion = true;
            break;

        case 4:
            if (haySimulacion) {
                panaderia.resumen();
            } else {
                cout << "[AVISO] Aun no se ha ejecutado ninguna simulacion. Seleccione la opcion 3 primero." << endl;
            }
            break;

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
