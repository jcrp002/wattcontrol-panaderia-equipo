#pragma once

#include "configuracion.h"

// ============================================================================
// FUNCIONES LIBRES Y SOBRECARGADAS (Unidad 3)
// ============================================================================

// 1. SOBRECARGA: Sumar arreglo de flotantes (ej. costos diarios o semanales)
float sumar(const float datos[], int n) {
    float total = 0.0f;
    for (int i = 0; i < n; i++) {
        total += datos[i];
    }
    return total;
}

// 2. SOBRECARGA: Sumar arreglo de enteros (ej. horas cumplidas o dias de produccion)
int sumar(const int datos[], int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += datos[i];
    }
    return total;
}

// 3. Calculo del promedio aritmetico de un arreglo
float promedio(const float datos[], int n) {
    if (n <= 0) return 0.0f;
    return sumar(datos, n) / n;
}

// 4. Posicion (indice) del valor maximo en un arreglo
int posicionMaximo(const float datos[], int n) {
    int pos = 0;
    for (int i = 1; i < n; i++) {
        if (datos[i] > datos[pos]) {
            pos = i;
        }
    }
    return pos;
}

// 5. Proyeccion mensual basada en el resultado semanal
float proyectarMes(float costoSemana) {
    return costoSemana * SEMANAS_MES;
}

// 6. SOBRECARGA: Dibujar linea separadora simple
void linea(int largo) {
    for (int i = 0; i < largo; i++) cout << '-';
    cout << endl;
}

// 7. SOBRECARGA: Dibujar linea separadora con caracter personalizado
void linea(int largo, char caracter) {
    for (int i = 0; i < largo; i++) cout << caracter;
    cout << endl;
}

// ============================================================================
// VALIDACION DE CADENAS DE TEXTO (string: empty, length/size, at)
// ============================================================================

// Revisa que un nombre no este vacio, tenga minimo 3 letras y empiece con mayuscula
string validarTexto(const string& texto, const string& campo) {
    if (texto.empty()) {
        return "El " + campo + " no puede estar vacio.";
    }
    if (texto.length() < 3) {
        return "El " + campo + " debe tener al menos 3 caracteres.";
    }
    if (!isupper(texto.at(0))) {
        return "El " + campo + " debe comenzar con letra mayuscula.";
    }
    return ""; // Texto valido
}

// Pide un texto mediante ciclo do-while hasta que cumpla las validaciones
string pedirTextoValido(const string& mensaje, const string& campo) {
    string entrada;
    string error;
    do {
        cout << mensaje;
        getline(cin, entrada);
        error = validarTexto(entrada, campo);
        if (!error.empty()) {
            cout << "  [ERROR] " << error << " Intente de nuevo." << endl;
        }
    } while (!error.empty());
    return entrada;
}
