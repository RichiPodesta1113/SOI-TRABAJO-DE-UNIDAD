// === Simulador de Sistema Operativo ===
// Este programa simula:
// 1. Planificación de procesos (FCFS, SPN, Round Robin).
// 2. Gestión de memoria con First-Fit y Best-Fit (particiones variables).

#include <iostream>     // Para entradas y salidas en consola (cout, cin)
#include <vector>       // Para manejar listas dinámicas de procesos y bloques de memoria
#include <algorithm>    // Para ordenar vectores (sort)
#include <queue>        // Para usar colas en Round Robin (FIFO)
#include <climits>      // Para usar INT_MAX (valor entero más grande disponible)

using namespace std;

// ==== Estructura de proceso ====
// Representa cada proceso con sus tiempos y cálculos
struct Proceso {
    int pid;        // Identificador único del proceso
    int llegada;    // Momento en que llega al sistema
    int servicio;   // Tiempo total que requiere de CPU
    int inicio;     // Momento en que empieza su ejecución
    int fin;        // Momento en que termina su ejecución
    int restante;   // Tiempo de servicio restante (usado en Round Robin)

    // Métodos para calcular métricas
    int respuesta() const { return inicio - llegada; }          // Tiempo de respuesta
    int espera() const { return fin - llegada - servicio; }     // Tiempo de espera
    int retorno() const { return fin - llegada; }               // Tiempo de retorno
};

// ==== Estructura de bloque de memoria ====
// Representa un segmento de memoria (libre u ocupado)
struct BloqueMemoria {
    int id;            // Número identificador del bloque
    int inicio;        // Dirección inicial del bloque en memoria
    int tamano;        // Tamaño del bloque
    bool libre;        // true = está disponible, false = está ocupado
    int pid_asignado;  // PID del proceso que ocupa el bloque (-1 si está libre)
};

// ==== Mostrar tabla de procesos y métricas ====
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
        if (p.fin > tiempoTotal) tiempoTotal = p.fin;  // Para calcular throughput
    }

    // Mostrar métricas promedio
    cout << "\nPromedio Respuesta: " << sumaResp / procesos.size();
    cout << "\nPromedio Espera: " << sumaEsp / procesos.size();
    cout << "\nPromedio Retorno: " << sumaRet / procesos.size();
    cout << "\nThroughput: " << (double)procesos.size() / tiempoTotal << " procesos/unidad de tiempo\n";
}

// ==== Algoritmo FCFS (First-Come, First-Served) ====
void planificar_FCFS(vector<Proceso>& procesos) {
    // Ordenar por tiempo de llegada
    sort(procesos.begin(), procesos.end(), [](Proceso a, Proceso b) {
        return a.llegada < b.llegada;
    });

    int tiempoActual = 0;
    for (auto &p : procesos) {
        if (tiempoActual < p.llegada)
            tiempoActual = p.llegada;  // Espera si el CPU está inactivo
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
        // Mover procesos que ya llegaron a la lista de listos
        for (auto it = procesos.begin(); it != procesos.end();) {
            if (it->llegada <= tiempoActual) {
                listaListos.push_back(*it);
                it = procesos.erase(it);
            } else {
                ++it;
            }
        }

        if (!listaListos.empty()) {
            // Selecciona el proceso con menor tiempo de servicio
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
            tiempoActual++;  // Avanzar tiempo si no hay procesos listos
        }
    }
    mostrarTabla(resultado);
}

// ==== Algoritmo Round Robin ====
void planificar_RR(vector<Proceso>& procesos, int quantum) {
    queue<Proceso> cola;        // Cola circular de procesos
    vector<Proceso> resultado;  // Procesos completados
    int tiempoActual = 0;
    // Inicializar tiempo restante
    for (auto &p : procesos) {
        p.restante = p.servicio;
    }
    while (!procesos.empty() || !cola.empty()) {
        // Mover procesos que ya llegaron a la cola
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
                cola.push(p);  // Vuelve a la cola si no terminó
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

// ==== Gestión de Memoria ====
// Inicializa memoria con un único bloque libre
vector<BloqueMemoria> inicializarMemoria(int tamanoTotal) {
    return { {1, 0, tamanoTotal, true, -1} };
}

// Muestra todos los bloques de memoria
void mostrarMemoria(const vector<BloqueMemoria>& memoria) {
    cout << "\nEstado de memoria:\n";
    cout << "ID | Inicio | Tamano | Libre | PID\n";
    for (auto &b : memoria) {
        cout << b.id << "  | " << b.inicio << "     | " << b.tamano << "    | "
             << (b.libre ? "SI" : "NO") << "   | " << (b.pid_asignado==-1?0:b.pid_asignado) << "\n";
    }
}

// Asignación de memoria con First-Fit o Best-Fit
bool asignarMemoria(vector<BloqueMemoria>& memoria, int pid, int tamano, int estrategia) {
    int idx = -1;                     // Índice del bloque elegido
    int mejorSobra = INT_MAX;         // Diferencia mínima para Best-Fit

    for (int i = 0; i < memoria.size(); i++) {
        if (memoria[i].libre && memoria[i].tamano >= tamano) {
            if (estrategia == 1) { idx = i; break; }  // First-Fit: toma el primero que encuentra
            else { 
                int sobra = memoria[i].tamano - tamano;
                if (sobra < mejorSobra) { mejorSobra = sobra; idx = i; }
            }
        }
    }
    if (idx == -1) return false; // No hay bloque que quepa

    BloqueMemoria &b = memoria[idx];
    int inicioAsignado = b.inicio;
    b.libre = false;
    b.pid_asignado = pid;
    int espacioRestante = b.tamano - tamano;
    b.tamano = tamano;

    // Si sobró espacio, crear un nuevo bloque libre
    if (espacioRestante > 0) {
        BloqueMemoria nuevo = { (int)memoria.size()+1, inicioAsignado + tamano, espacioRestante, true, -1 };
        memoria.insert(memoria.begin() + idx + 1, nuevo);
    }
    return true;
}

// ==== Programa Principal ====
int main() {
    cout << "=== Simulador de Sistema Operativo ===\n";
    cout << "Seleccione el algoritmo de planificacion:\n1) FCFS\n2) SPN\n3) Round Robin\n";

    int opcion; cin >> opcion;
    vector<Proceso> procesos;

    int n; cout << "Ingrese cantidad de procesos: "; cin >> n;
    for (int i = 0; i < n; i++) {
        Proceso p;
        p.pid = i + 1;       // Identificador del proceso
        p.llegada = i;       // Llegada escalonada
        cout << "\nProceso ID #" << p.pid << " | Llegada: " << p.llegada;
        cout << "\nIngrese Servicio: ";
        cin >> p.servicio;
        p.inicio = -1; 
        p.fin = 0; 
        p.restante = p.servicio;
        procesos.push_back(p);
    }

    // Ejecutar el algoritmo seleccionado
    if (opcion == 1) planificar_FCFS(procesos);
    else if (opcion == 2) planificar_SPN(procesos);
    else if (opcion == 3) {
        int quantum; cout << "Ingrese quantum: "; cin >> quantum;
        planificar_RR(procesos, quantum);
    }

    // ==== Sección de memoria ====
    int tamMemoria;
    cout << "\nIngrese tamano total de memoria: ";
    cin >> tamMemoria;
    auto memoria = inicializarMemoria(tamMemoria);

    int estrategia;
    cout << "Estrategia de asignacion [(1) first-fit/(2) best-fit: ]";
    cin >> estrategia;

    int solicitudes;
    cout << "Cantidad de solicitudes de memoria: ";
    cin >> solicitudes;

    for (int i = 0; i < solicitudes; i++) {
        int pid, tamano;
        cout << "Solicitud #" << i+1 << " PID: "; cin >> pid;
        cout << "Tamano solicitado: "; cin >> tamano;
        if (asignarMemoria(memoria, pid, tamano, estrategia))
            cout << "Memoria asignada al PID " << pid << " (" << tamano << " unidades)\n";
        else
            cout << "No se encontro bloque disponible para PID " << pid << "\n";
    }

    mostrarMemoria(memoria);
    cout << "\nSimulacion finalizada.\n";
    return 0;
}
