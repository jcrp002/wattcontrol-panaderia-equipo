#include "configuracion.h"
#include "utilidades.h"
#include "Mercado.h"
#include "CargaCritica.h"
#include "CargaAplazable.h"
#include "Negocio.h"

// ============================================================================
// FUNCION PRINCIPAL (main) - LIMPIO Y MODULAR (EA3)
// ============================================================================
int main() {
    linea(60, '=');
    cout << "     SISTEMA INTELIGENTE DE GESTION ENERGETICA WATTCONTROL" << endl;
    cout << "          Unidad 3: Simulacion con Precios Reales XM" << endl;
    linea(60, '=');

    // VALIDACION DE CADENAS DE TEXTO CON METODOS DE STRING (empty, length, at)
    string nombreNegocio = pedirTextoValido(
        "Ingrese el nombre de la panaderia (ej. Panaderia La Union): ", 
        "nombre del negocio"
    );
    string nombreResponsable = pedirTextoValido(
        "Ingrese el nombre del responsable (ej. Carlos Castro): ", 
        "nombre del responsable"
    );

    // Instanciacion de objetos del sistema
    Mercado mercado; // Asociacion

    CargaCritica refrigeracion("Refrigeracion", POTENCIA_REFRIGERACION);
    CargaAplazable horno("Horno Rotativo", POTENCIA_HORNO, HORAS_MIN_HORNO);
    CargaAplazable amasadora("Amasadora", POTENCIA_AMASADORA, HORAS_MIN_AMASADORA);

    Negocio panaderia(nombreNegocio, nombreResponsable, &mercado);
    panaderia.agregarCarga(&refrigeracion);
    panaderia.agregarCarga(&horno);
    panaderia.agregarCarga(&amasadora);

    panaderia.mostrarConfiguracion();

    int opcion = -1;
    do {
        cout << "\n=============== MENU WATTCONTROL - SEMANA REAL ===============" << endl;
        cout << "1. Ingresar lectura manual (hora, embalses, precio)" << endl;
        cout << "2. Simular la semana real completa (7 dias x 24 horas = 168 h)" << endl;
        cout << "3. Ver reporte financiero semanal y proyeccion mensual" << endl;
        cout << "0. Salir" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
        case 1: {
            int hora;
            float embalses, precio;

            // Validacion con constantes
            do {
                cout << "Hora del dia (" << HORA_MIN_VALIDA << " a " << HORA_MAX_VALIDA << "): ";
                cin >> hora;
                if (hora < HORA_MIN_VALIDA || hora > HORA_MAX_VALIDA) {
                    cout << "  [ERROR] Hora invalida. Intente de nuevo." << endl;
                }
            } while (hora < HORA_MIN_VALIDA || hora > HORA_MAX_VALIDA);

            do {
                cout << "Nivel de embalses (" << EMBALSE_MIN_VALIDO << " a " << EMBALSE_MAX_VALIDO << " %): ";
                cin >> embalses;
                if (embalses < EMBALSE_MIN_VALIDO || embalses > EMBALSE_MAX_VALIDO) {
                    cout << "  [ERROR] Porcentaje invalido. Intente de nuevo." << endl;
                }
            } while (embalses < EMBALSE_MIN_VALIDO || embalses > EMBALSE_MAX_VALIDO);

            do {
                cout << "Precio de bolsa (COP/kWh, mayor a " << PRECIO_MIN_VALIDO << "): ";
                cin >> precio;
                if (precio <= PRECIO_MIN_VALIDO) {
                    cout << "  [ERROR] Precio invalido. Intente de nuevo." << endl;
                }
            } while (precio <= PRECIO_MIN_VALIDO);

            panaderia.evaluarManual(hora, precio, embalses);
            break;
        }

        case 2:
            panaderia.simularSemana();
            break;

        case 3:
            panaderia.reporteSemanal();
            break;

        case 0:
            cout << "Gracias por utilizar WattControl. Sesion finalizada con exito." << endl;
            break;

        default:
            cout << "[ERROR] Opcion no valida. Seleccione entre 0 y 3." << endl;
            break;
        }

    } while (opcion != 0);

    return 0;
}
