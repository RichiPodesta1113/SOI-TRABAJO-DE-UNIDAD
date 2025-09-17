// === Simulador de Sistema Operativo ===
// Autor: (tu nombre)
// Este programa simula planificación de CPU y asignación de memoria
// usando algoritmos FCFS, SPN y Round Robin.

#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
using namespace std;

// ==== Definición de estructuras ====
// Estructura que representa un proceso en el sistema
struct Proceso {
    int pid;       // Identificador único del proceso (generado automáticamente)
    int llegada;   // Tiempo en que el proceso llega al sistema (orden de ingreso)
    int servicio;  // Tiempo total de CPU que necesita
    int inicio;    // Tiempo en que se ejecuta por primera vez
    int fin;       // Tiempo en que el proceso termina
    int restante;  // Tiempo de CPU que le falta (usado para RR)

    // Funciones para calcular métricas
    int respuesta() const { return inicio - llegada; }
    int espera() const { return fin - llegada - servicio; }
    int retorno() const { return fin - llegada; }
};

// ==== Función para mostrar resultados de planificación ====
void mostrarTabla(const vector<Proceso>& procesos) {
    cout << "\nPID | Llegada | Servicio | Inicio | Fin | Respuesta | Espera | Retorno\n";
    cout << "--- | ------- | -------- | ------ | --- | --------- | ------ | -------\n";

    double sumaResp = 0, sumaEsp = 0, sumaRet = 0;
    int tiempoTotal = 0;

    for (auto &p : procesos) {
        cout << p.pid << "   | " << p.llegada << "       | " << p.servicio << "        | "
             << p.inicio << "      | " << p.fin << "   | "
             << p.respuesta() << "         | " << p.espera() << "      | " << p.retorno() << "\n";
        sumaResp += p.respuesta();
        sumaEsp += p.espera();
        sumaRet += p.retorno();
        if (p.fin > tiempoTotal) tiempoTotal = p.fin;
    }

    // Mostrar métricas globales
    cout << "\nPromedio Respuesta: " << sumaResp / procesos.size();
    cout << "\nPromedio Espera: " << sumaEsp / procesos.size();
    cout << "\nPromedio Retorno: " << sumaRet / procesos.size();
    cout << "\nThroughput: " << (double)procesos.size() / tiempoTotal << " procesos/unidad de tiempo\n";
}

// ==== Algoritmo FCFS ====
void planificar_FCFS(vector<Proceso>& procesos) {
    sort(procesos.begin(), procesos.end(), [](Proceso a, Proceso b) {
        return a.llegada < b.llegada;
    });

    int tiempoActual = 0;
    for (auto &p : procesos) {
        if (tiempoActual < p.llegada)
            tiempoActual = p.llegada;
        p.inicio = tiempoActual;
        tiempoActual += p.servicio;
        p.fin = tiempoActual;
    }
    mostrarTabla(procesos);
}

// ==== Algoritmo SPN (Shortest Process Next) ====
void planificar_SPN(vector<Proceso>& procesos) {
    vector<Proceso> listaListos;
    vector<Proceso> resultado;
    int tiempoActual = 0;

    while (!procesos.empty() || !listaListos.empty()) {
        for (auto it = procesos.begin(); it != procesos.end();) {
            if (it->llegada <= tiempoActual) {
                listaListos.push_back(*it);
                it = procesos.erase(it);
            } else {
                ++it;
            }
        }

        if (!listaListos.empty()) {
            sort(listaListos.begin(), listaListos.end(), [](Proceso a, Proceso b) {
                if (a.servicio == b.servicio) return a.llegada < b.llegada;
                return a.servicio < b.servicio;
            });

            Proceso p = listaListos.front();
            listaListos.erase(listaListos.begin());
            p.inicio = tiempoActual;
            tiempoActual += p.servicio;
            p.fin = tiempoActual;
            resultado.push_back(p);
        } else {
            tiempoActual++;
        }
    }
    mostrarTabla(resultado);
}

// ==== Algoritmo Round Robin ====
void planificar_RR(vector<Proceso>& procesos, int quantum) {
    queue<Proceso> cola;
    vector<Proceso> resultado;
    int tiempoActual = 0;

    for (auto &p : procesos) {
        p.restante = p.servicio;
    }

    while (!procesos.empty() || !cola.empty()) {
        for (auto it = procesos.begin(); it != procesos.end();) {
            if (it->llegada <= tiempoActual) {
                cola.push(*it);
                it = procesos.erase(it);
            } else {
                ++it;
            }
        }

        if (!cola.empty()) {
            Proceso p = cola.front(); cola.pop();
            if (p.inicio == -1) p.inicio = tiempoActual;

            int ejecucion = min(quantum, p.restante);
            p.restante -= ejecucion;
            tiempoActual += ejecucion;

            if (p.restante > 0) {
                cola.push(p);
            } else {
                p.fin = tiempoActual;
                resultado.push_back(p);
            }
        } else {
            tiempoActual++;
        }
    }
    mostrarTabla(resultado);
}

// ==== Función principal ====
int main() {
    cout << "=== Simulador de Sistema Operativo ===\n";
    cout << "Seleccione el algoritmo de planificación:\n1) FCFS\n2) SPN\n3) Round Robin\n";

    int opcion; cin >> opcion;
    vector<Proceso> procesos;

    int n; cout << "Ingrese cantidad de procesos: "; cin >> n;
    for (int i = 0; i < n; i++) {
        Proceso p;
        p.pid = i + 1;         // PID generado automáticamente
        p.llegada = i;         // Llegada asignada en orden de ingreso
        cout << "\nProceso ID #" << p.pid << " | Llegada: " << p.llegada;
        cout << "\nIngrese Servicio: ";
        cin >> p.servicio;
        p.inicio = -1; p.fin = 0; p.restante = p.servicio;
        procesos.push_back(p);
    }

    if (opcion == 1) planificar_FCFS(procesos);
    else if (opcion == 2) planificar_SPN(procesos);
    else if (opcion == 3) {
        int quantum; cout << "Ingrese quantum: "; cin >> quantum;
        planificar_RR(procesos, quantum);
    }

    return 0;
}
