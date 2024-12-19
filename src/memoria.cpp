#include "memoria.hpp"
#include <iostream>

Memoria::Memoria() {
    cout << "Memória: Inicializando...\n";
}

Memoria::~Memoria() {
    cout << "Memória: Liberando recursos...\n";
    // A thread de cada página é automaticamente limpa quando a memória é desalocada
}

void Memoria::adicionarPagina(int id, int base, int limite, int linhas, function<void()> func) {
    // Adiciona uma nova página de memória com um processo
    PaginaMemoria novaPagina(id, base, limite, linhas, func);
    paginas.push_back(novaPagina);
}

void Memoria::iniciarProcessos() {
    cout << "Memória: Iniciando processos...\n";
    for (auto& pagina : paginas) {
        pagina.iniciar();  // Inicia o processo de cada página
    }
}

void Memoria::exibirEstado() {
    cout << "Memória: Exibindo estado...\n";
    for (const auto& pagina : paginas) {
        cout << "Página " << pagina.pcb.id << " - Estado: " << pagina.pcb.estado << "\n";
    }
}
