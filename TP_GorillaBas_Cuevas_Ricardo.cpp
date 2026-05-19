/*
Autor: Ricardo Cuevas
Fecha: 2024-06-15
Descripcion: Implementacion de Gorilla.bas en C++ con apuntado con mouse, viento y edificios.    
*/


#include <iostream>   // cin, cout
#include <string>     // string
#include <vector>     // vector
#include <ctime>      // time
#include <cstdlib>    // srand, rand
#include <cmath>      // sin, cos, atan2, round
#include <limits>     // numeric_limits
#include <cctype>     // tolower
#include <windows.h>  // consola de Windows: color, cursor, mouse, Sleep, Beep

using namespace std;

// =========================
// CONSTANTES DEL PROGRAMA
// =========================

const int ANCHO_PANTALLA = 100;
const int ALTO_ESCENARIO = 30;
const int SUELO_Y = 26;
const double PI = 3.141592653589793;

// Constantes de fisica. Se pueden modificar para cambiar la dificultad.
const double GRAVEDAD = 9.8;
const double PASO_TIEMPO = 0.10;
const double ESCALA_MOVIMIENTO = 0.18;

// Medidas del dibujo del mono.
const int ANCHO_MONO = 13;
const int ALTO_MONO = 6;

// =========================
// ESTRUCTURAS Y CLASES
// =========================

// Representa un edificio de la ciudad.
struct Edificio {
    int x;
    int ancho;
    int alto;
};

// Guarda la informacion de un disparo realizado.
struct Disparo {
    string nombreJugador;
    double angulo;
    double velocidad;
    int viento;
    bool impacto;
};

// Nodo de una lista enlazada simple para guardar historial.
struct NodoDisparo {
    Disparo dato;
    NodoDisparo* siguiente;
};

// Lista enlazada usada para registrar los ultimos disparos.
class HistorialDisparos {
private:
    NodoDisparo* cabeza;

public:
    HistorialDisparos();
    ~HistorialDisparos();
    void agregar(Disparo disparo);
    void mostrar(int maximo) const;
    void limpiar();
};

// Clase que representa a cada jugador.
class Jugador {
private:
    string nombre;
    int x;
    int y;
    int puntos;

public:
    Jugador(string nombreInicial = "Jugador");
    void colocar(int nuevoX, int nuevoY);
    string obtenerNombre() const;
    int obtenerX() const;
    int obtenerY() const;
    int obtenerPuntos() const;
    void sumarPunto();
};

// =========================
// PROTOTIPOS DE FUNCIONES
// =========================

void configurarConsola();
void configurarMouse();
void cambiarColor(int color);
void moverCursor(int x, int y);
void limpiarPantalla();
void ocultarCursor();
void pausar(int milisegundos);

int numeroAleatorio(int minimo, int maximo);
double leerDouble(string mensaje, double minimo, double maximo);
char leerSiNo(string mensaje);

void mostrarMenuInicial();
void jugarPartida();

vector<Edificio> generarEdificios();
void colocarJugadores(Jugador& jugador1, Jugador& jugador2, const vector<Edificio>& edificios);

void dibujarEscenario(const vector<Edificio>& edificios,
                      const Jugador& jugador1,
                      const Jugador& jugador2,
                      int viento,
                      const HistorialDisparos& historial);
void dibujarPanelSuperior(const Jugador& jugador1, const Jugador& jugador2, int viento);
void dibujarEdificios(const vector<Edificio>& edificios);
void dibujarJugador(const Jugador& jugador, int color);
void dibujarLineaApuntado(int inicioX, int inicioY, int destinoX, int destinoY);
void dibujarExplosion(int x, int y);

double leerAnguloConMouse(const Jugador& tirador, const Jugador& objetivo);
bool simularDisparo(const Jugador& tirador,
                    const Jugador& objetivo,
                    const vector<Edificio>& edificios,
                    double angulo,
                    double velocidad,
                    int viento);
bool impactoJugador(int x, int y, const Jugador& objetivo);
bool chocaConEdificio(int x, int y, const vector<Edificio>& edificios);

// =========================
// FUNCION PRINCIPAL
// =========================

int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    configurarConsola();

    char opcion;

    do {
        mostrarMenuInicial();
        jugarPartida();
        opcion = leerSiNo("Deseas jugar otra partida? (s/n): ");
    } while (opcion == 's');

    limpiarPantalla();
    cambiarColor(11);
    cout << "Gracias por jugar Gorilla.bas en C++." << endl;
    cambiarColor(7);

    return 0;
}

// =========================
// METODOS DE HISTORIAL
// =========================

HistorialDisparos::HistorialDisparos() {
    cabeza = NULL;
}

HistorialDisparos::~HistorialDisparos() {
    limpiar();
}

void HistorialDisparos::agregar(Disparo disparo) {
    NodoDisparo* nuevo = new NodoDisparo;
    nuevo->dato = disparo;
    nuevo->siguiente = cabeza;
    cabeza = nuevo;
}

void HistorialDisparos::mostrar(int maximo) const {
    NodoDisparo* actual = cabeza;
    int contador = 0;

    cout << "Ultimos disparos:" << endl;

    if (actual == NULL) {
        cout << "  Sin disparos registrados." << endl;
        return;
    }

    while (actual != NULL && contador < maximo) {
        cout << "  " << actual->dato.nombreJugador
             << " | Angulo: " << static_cast<int>(round(actual->dato.angulo))
             << " | Velocidad: " << actual->dato.velocidad
             << " | Viento: " << actual->dato.viento
             << " | " << (actual->dato.impacto ? "Impacto" : "Fallo")
             << endl;

        actual = actual->siguiente;
        contador++;
    }
}

void HistorialDisparos::limpiar() {
    NodoDisparo* actual = cabeza;

    while (actual != NULL) {
        NodoDisparo* borrar = actual;
        actual = actual->siguiente;
        delete borrar;
    }

    cabeza = NULL;
}

// =========================
// METODOS DE JUGADOR
// =========================

Jugador::Jugador(string nombreInicial) {
    nombre = nombreInicial;
    x = 0;
    y = 0;
    puntos = 0;
}

void Jugador::colocar(int nuevoX, int nuevoY) {
    x = nuevoX;
    y = nuevoY;
}

string Jugador::obtenerNombre() const {
    return nombre;
}

int Jugador::obtenerX() const {
    return x;
}

int Jugador::obtenerY() const {
    return y;
}

int Jugador::obtenerPuntos() const {
    return puntos;
}

void Jugador::sumarPunto() {
    puntos++;
}

// =========================
// FUNCIONES DE CONSOLA
// =========================

void configurarConsola() {
    SetConsoleTitleA("Gorilla.bas - version con mouse y viento");
    ocultarCursor();
    configurarMouse();
}

void configurarMouse() {
    HANDLE entrada = GetStdHandle(STD_INPUT_HANDLE);
    DWORD modo = 0;

    GetConsoleMode(entrada, &modo);
    modo = modo | ENABLE_EXTENDED_FLAGS;
    modo = modo & ~ENABLE_QUICK_EDIT_MODE;
    modo = modo | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT;
    SetConsoleMode(entrada, modo);
}

void cambiarColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void moverCursor(int x, int y) {
    COORD posicion;
    posicion.X = static_cast<SHORT>(x);
    posicion.Y = static_cast<SHORT>(y);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), posicion);
}

void limpiarPantalla() {
    HANDLE consola = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    DWORD escritos;
    DWORD celdas;
    COORD inicio = {0, 0};

    GetConsoleScreenBufferInfo(consola, &info);
    celdas = info.dwSize.X * info.dwSize.Y;

    FillConsoleOutputCharacter(consola, ' ', celdas, inicio, &escritos);
    FillConsoleOutputAttribute(consola, info.wAttributes, celdas, inicio, &escritos);
    SetConsoleCursorPosition(consola, inicio);
}

void ocultarCursor() {
    HANDLE consola = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor;

    GetConsoleCursorInfo(consola, &cursor);
    cursor.bVisible = false;
    SetConsoleCursorInfo(consola, &cursor);
}

void pausar(int milisegundos) {
    Sleep(milisegundos);
}

// =========================
// FUNCIONES DE ENTRADA
// =========================

int numeroAleatorio(int minimo, int maximo) {
    return minimo + rand() % (maximo - minimo + 1);
}

double leerDouble(string mensaje, double minimo, double maximo) {
    double valor;

    while (true) {
        cout << mensaje;
        cin >> valor;

        if (!cin.fail() && valor >= minimo && valor <= maximo) {
            return valor;
        }

        cin.clear();
        cin.ignore((numeric_limits<streamsize>::max)(), '\n');
        cambiarColor(12);
        cout << "Valor invalido. Debe estar entre " << minimo << " y " << maximo << "." << endl;
        cambiarColor(7);
    }
}

char leerSiNo(string mensaje) {
    char opcion;

    while (true) {
        cambiarColor(7);
        cout << mensaje;
        cin >> opcion;
        opcion = static_cast<char>(tolower(opcion));

        if (opcion == 's' || opcion == 'n') {
            return opcion;
        }

        cambiarColor(12);
        cout << "Respuesta invalida. Escribe s o n." << endl;
        cambiarColor(7);
    }
}

// =========================
// MENU Y FLUJO DEL JUEGO
// =========================

void mostrarMenuInicial() {
    limpiarPantalla();

    cambiarColor(11);
    moverCursor(18, 3);
    cout << "+------------------------------------------------------------+";
    moverCursor(18, 4);
    cout << "|                GORILLA.BAS                          |";
    moverCursor(18, 5);
    cout << "|          Angulo con mouse, viento y ciudad                 |";
    moverCursor(18, 6);
    cout << "+------------------------------------------------------------+";

    cambiarColor(7);
    moverCursor(18, 9);
    cout << "Objetivo: golpear al mono rival lanzando una banana.";
    moverCursor(18, 11);
    cout << "1. Haz clic hacia donde quieres apuntar.";
    moverCursor(18, 12);
    cout << "2. Escribe la velocidad del disparo.";
    moverCursor(18, 13);
    cout << "3. El viento empuja la banana a izquierda o derecha.";
    moverCursor(18, 15);
    cout << "Si el mouse no responde, presiona T para escribir el angulo.";

    cambiarColor(14);
    moverCursor(18, 19);
    cout << "Presiona ENTER para comenzar...";

    cin.ignore((numeric_limits<streamsize>::max)(), '\n');
    cin.get();
}

void jugarPartida() {
    Jugador jugador1("Jugador 1");
    Jugador jugador2("Jugador 2");
    HistorialDisparos historial;

    const int PUNTOS_PARA_GANAR = 2;
    int turno = 1;

    vector<Edificio> edificios = generarEdificios();
    colocarJugadores(jugador1, jugador2, edificios);

    while (jugador1.obtenerPuntos() < PUNTOS_PARA_GANAR &&
           jugador2.obtenerPuntos() < PUNTOS_PARA_GANAR) {
        int viento = numeroAleatorio(-7, 7);

        dibujarEscenario(edificios, jugador1, jugador2, viento, historial);

        Jugador* tirador;
        Jugador* objetivo;

        if (turno == 1) {
            tirador = &jugador1;
            objetivo = &jugador2;
        } else {
            tirador = &jugador2;
            objetivo = &jugador1;
        }

        moverCursor(0, ALTO_ESCENARIO + 4);
        cambiarColor(14);
        cout << "Turno de " << tirador->obtenerNombre() << ".                              " << endl;
        cambiarColor(7);

        double angulo = leerAnguloConMouse(*tirador, *objetivo);
        double velocidad = leerDouble("Ingresa velocidad entre 10 y 120: ", 10, 120);

        bool impacto = simularDisparo(*tirador, *objetivo, edificios, angulo, velocidad, viento);

        Disparo disparo;
        disparo.nombreJugador = tirador->obtenerNombre();
        disparo.angulo = angulo;
        disparo.velocidad = velocidad;
        disparo.viento = viento;
        disparo.impacto = impacto;
        historial.agregar(disparo);

        moverCursor(0, ALTO_ESCENARIO + 9);

        if (impacto) {
            tirador->sumarPunto();
            cambiarColor(10);
            cout << "Directo! " << tirador->obtenerNombre()
                 << " ahora tiene " << tirador->obtenerPuntos()
                 << " punto(s)." << endl;
            Beep(850, 180);
            Beep(1150, 220);
            pausar(1400);

            edificios = generarEdificios();
            colocarJugadores(jugador1, jugador2, edificios);
        } else {
            cambiarColor(12);
            cout << "Fallo el disparo. Cambia el turno.                         " << endl;
            Beep(300, 180);
            pausar(1000);

            if (turno == 1) {
                turno = 2;
            } else {
                turno = 1;
            }
        }
    }

    dibujarEscenario(edificios, jugador1, jugador2, 0, historial);
    moverCursor(0, ALTO_ESCENARIO + 4);
    cambiarColor(14);

    if (jugador1.obtenerPuntos() > jugador2.obtenerPuntos()) {
        cout << "Ganador de la partida: " << jugador1.obtenerNombre() << endl;
    } else {
        cout << "Ganador de la partida: " << jugador2.obtenerNombre() << endl;
    }

    cambiarColor(7);
}

// =========================
// ESCENARIO Y DIBUJO
// =========================

vector<Edificio> generarEdificios() {
    vector<Edificio> edificios;
    const int cantidadEdificios = 10;
    const int anchoEdificio = ANCHO_PANTALLA / cantidadEdificios;

    for (int i = 0; i < cantidadEdificios; i++) {
        Edificio edificio;
        edificio.x = i * anchoEdificio;
        edificio.ancho = anchoEdificio;
        edificio.alto = numeroAleatorio(6, 16);
        edificios.push_back(edificio);
    }

    return edificios;
}

void colocarJugadores(Jugador& jugador1, Jugador& jugador2, const vector<Edificio>& edificios) {
    Edificio izquierdo = edificios[1];
    Edificio derecho = edificios[edificios.size() - 2];

    jugador1.colocar(izquierdo.x + izquierdo.ancho / 2, SUELO_Y - izquierdo.alto);
    jugador2.colocar(derecho.x + derecho.ancho / 2, SUELO_Y - derecho.alto);
}

void dibujarEscenario(const vector<Edificio>& edificios,
                      const Jugador& jugador1,
                      const Jugador& jugador2,
                      int viento,
                      const HistorialDisparos& historial) {
    limpiarPantalla();
    dibujarPanelSuperior(jugador1, jugador2, viento);
    dibujarEdificios(edificios);
    dibujarJugador(jugador1, 10);
    dibujarJugador(jugador2, 12);

    cambiarColor(7);
    moverCursor(0, ALTO_ESCENARIO);
    historial.mostrar(4);
}

void dibujarPanelSuperior(const Jugador& jugador1, const Jugador& jugador2, int viento) {
    cambiarColor(11);
    moverCursor(0, 0);
    cout << "+--------------------------------------------------------------------------------------------------+";
    moverCursor(0, 1);
    cout << "| Gorilla.bas C++                                                                                  |";
    moverCursor(0, 2);
    cout << "+--------------------------------------------------------------------------------------------------+";

    cambiarColor(7);
    moverCursor(2, 1);
    cout << jugador1.obtenerNombre() << ": " << jugador1.obtenerPuntos()
         << "      " << jugador2.obtenerNombre() << ": " << jugador2.obtenerPuntos();

    moverCursor(55, 1);
    cout << "Viento: ";

    if (viento > 0) {
        cambiarColor(10);
        cout << "derecha >>> " << viento;
    } else if (viento < 0) {
        cambiarColor(12);
        cout << "izquierda <<< " << abs(viento);
    } else {
        cambiarColor(7);
        cout << "sin viento";
    }
}

void dibujarEdificios(const vector<Edificio>& edificios) {
    for (int i = 0; i < static_cast<int>(edificios.size()); i++) {
        int colorEdificio = 1 + (i % 6);
        cambiarColor(colorEdificio);

        for (int y = SUELO_Y; y > SUELO_Y - edificios[i].alto; y--) {
            for (int x = edificios[i].x; x < edificios[i].x + edificios[i].ancho; x++) {
                bool borde = y == SUELO_Y - edificios[i].alto + 1 ||
                              x == edificios[i].x ||
                              x == edificios[i].x + edificios[i].ancho - 1;
                bool ventana = (x - edificios[i].x) % 3 == 1 && (SUELO_Y - y) % 3 == 1;

                moverCursor(x, y);

                if (borde) {
                    cout << char(178);
                } else if (ventana) {
                    cambiarColor(14);
                    cout << char(254);
                    cambiarColor(colorEdificio);
                } else {
                    cout << char(219);
                }
            }
        }
    }

    cambiarColor(8);
    moverCursor(0, SUELO_Y + 1);
    for (int x = 0; x < ANCHO_PANTALLA; x++) {
        cout << char(205);
    }
}

void dibujarJugador(const Jugador& jugador, int color) {
    int x = jugador.obtenerX() - ANCHO_MONO / 2;
    int y = jugador.obtenerY();

    cambiarColor(color);
    moverCursor(x, y - 5);
    cout << "    .---.    ";
    moverCursor(x, y - 4);
    cout << "   (o   o)   ";
    moverCursor(x, y - 3);
    cout << "  /(  ^  )\\  ";
    moverCursor(x, y - 2);
    cout << " /  \\___/  \\ ";
    moverCursor(x, y - 1);
    cout << "    /| |\\    ";
    moverCursor(x, y);
    cout << "   _/   \\_   ";
}

void dibujarLineaApuntado(int inicioX, int inicioY, int destinoX, int destinoY) {
    int pasos = abs(destinoX - inicioX);
    int pasosY = abs(destinoY - inicioY);

    if (pasosY > pasos) {
        pasos = pasosY;
    }

    if (pasos == 0) {
        return;
    }

    cambiarColor(15);

    for (int i = 1; i <= pasos && i <= 24; i++) {
        double proporcion = static_cast<double>(i) / pasos;
        int x = static_cast<int>(round(inicioX + (destinoX - inicioX) * proporcion));
        int y = static_cast<int>(round(inicioY + (destinoY - inicioY) * proporcion));

        if (x >= 0 && x < ANCHO_PANTALLA && y >= 3 && y <= SUELO_Y) {
            moverCursor(x, y);
            cout << "*";
        }
    }
}

void dibujarExplosion(int x, int y) {
    cambiarColor(14);
    moverCursor(x, y);
    cout << "*";
    moverCursor(x - 1, y);
    cout << "***";
    moverCursor(x, y - 1);
    cout << "*";
    moverCursor(x, y + 1);
    cout << "*";
    Beep(1000, 120);
    pausar(250);
}

// =========================
// MOUSE, ANGULO Y FISICA
// =========================

double leerAnguloConMouse(const Jugador& tirador, const Jugador& objetivo) {
    HANDLE entrada = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD evento;
    DWORD cantidadLeida;

    int direccion = (tirador.obtenerX() < objetivo.obtenerX()) ? 1 : -1;
    int origenX = tirador.obtenerX();
    int origenY = tirador.obtenerY() - ALTO_MONO;

    moverCursor(0, ALTO_ESCENARIO + 5);
    cambiarColor(14);
    cout << "Haz clic con el mouse hacia donde quieres apuntar. Presiona T para teclado.        " << endl;
    cambiarColor(7);

    while (true) {
        ReadConsoleInput(entrada, &evento, 1, &cantidadLeida);

        if (evento.EventType == KEY_EVENT && evento.Event.KeyEvent.bKeyDown) {
            char tecla = evento.Event.KeyEvent.uChar.AsciiChar;

            if (tecla == 't' || tecla == 'T') {
                return leerDouble("Ingresa angulo entre 0 y 180: ", 0, 180);
            }
        }

        if (evento.EventType == MOUSE_EVENT) {
            MOUSE_EVENT_RECORD mouse = evento.Event.MouseEvent;
            bool clicIzquierdo = (mouse.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;

            if (clicIzquierdo) {
                int clicX = mouse.dwMousePosition.X;
                int clicY = mouse.dwMousePosition.Y;
                int distanciaHorizontal = (clicX - origenX) * direccion;
                int distanciaVertical = origenY - clicY;

                if (distanciaHorizontal <= 0 || distanciaVertical < 0) {
                    moverCursor(0, ALTO_ESCENARIO + 7);
                    cambiarColor(12);
                    cout << "Clic invalido: apunta hacia el rival y por encima del mono.                       ";
                    cambiarColor(7);
                } else {
                    double angulo = atan2(static_cast<double>(distanciaVertical),
                                          static_cast<double>(distanciaHorizontal)) * 180.0 / PI;

                    dibujarLineaApuntado(origenX, origenY, clicX, clicY);

                    moverCursor(0, ALTO_ESCENARIO + 7);
                    cambiarColor(10);
                    cout << "Angulo elegido con mouse: " << static_cast<int>(round(angulo)) << " grados.                  " << endl;
                    cambiarColor(7);

                    FlushConsoleInputBuffer(entrada);
                    return angulo;
                }
            }
        }
    }
}

bool simularDisparo(const Jugador& tirador,
                    const Jugador& objetivo,
                    const vector<Edificio>& edificios,
                    double angulo,
                    double velocidad,
                    int viento) {
    double radianes = angulo * PI / 180.0;
    int direccion = (tirador.obtenerX() < objetivo.obtenerX()) ? 1 : -1;

    double inicioX = tirador.obtenerX();
    double inicioY = tirador.obtenerY() - ALTO_MONO;

    double velocidadX = cos(radianes) * velocidad * direccion;
    double velocidadY = sin(radianes) * velocidad;

    for (double tiempo = 0; tiempo < 12; tiempo += PASO_TIEMPO) {
        // El viento se suma a la velocidad horizontal, por eso afecta la curva.
        double posicionX = inicioX + (velocidadX + viento * 2.0) * tiempo * ESCALA_MOVIMIENTO;
        double posicionY = inicioY - (velocidadY * tiempo - 0.5 * GRAVEDAD * tiempo * tiempo) * ESCALA_MOVIMIENTO;

        int pantallaX = static_cast<int>(round(posicionX));
        int pantallaY = static_cast<int>(round(posicionY));

        if (pantallaX < 0 || pantallaX >= ANCHO_PANTALLA || pantallaY < 0 || pantallaY > SUELO_Y) {
            return false;
        }

        cambiarColor(14);
        moverCursor(pantallaX, pantallaY);
        cout << "o";
        pausar(45);

        if (impactoJugador(pantallaX, pantallaY, objetivo)) {
            dibujarExplosion(pantallaX, pantallaY);
            return true;
        }

        if (chocaConEdificio(pantallaX, pantallaY, edificios)) {
            dibujarExplosion(pantallaX, pantallaY);
            return false;
        }
    }

    return false;
}

bool impactoJugador(int x, int y, const Jugador& objetivo) {
    int centroX = objetivo.obtenerX();
    int baseY = objetivo.obtenerY();
    bool dentroDelAncho = abs(x - centroX) <= ANCHO_MONO / 2;
    bool dentroDelAlto = y >= baseY - ALTO_MONO && y <= baseY;

    return dentroDelAncho && dentroDelAlto;
}

bool chocaConEdificio(int x, int y, const vector<Edificio>& edificios) {
    for (int i = 0; i < static_cast<int>(edificios.size()); i++) {
        int izquierda = edificios[i].x;
        int derecha = edificios[i].x + edificios[i].ancho - 1;
        int techo = SUELO_Y - edificios[i].alto + 1;

        if (x >= izquierda && x <= derecha && y >= techo && y <= SUELO_Y) {
            return true;
        }
    }

    return false;
}
