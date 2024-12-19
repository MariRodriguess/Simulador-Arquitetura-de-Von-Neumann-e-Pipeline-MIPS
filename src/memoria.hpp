#ifndef MEMORIA_HPP
#define MEMORIA_HPP

#include "include.hpp"
#include <thread>
#include <vector>
#include <functional>

class Memoria {
private:
    std::vector<PaginaMemoria> paginas;  // Armazena as páginas de memória

public:
    Memoria();   // Construtor
    ~Memoria();  // Destruidor

    void adicionarPagina(int id, int base, int limite, int linhas, std::function<void()> func);
    void iniciarProcessos();  // Inicia todos os processos nas páginas de memória

    // Função auxiliar para exibir o estado da memória
    void exibirEstado();
};

struct PaginaMemoria {
    PCB pcb;               // PCB associado
    thread processo;  // Thread vinculada ao processo
    
    PaginaMemoria(int id, int prioridade, int quantum, int base, int limite, int linhas, const std::string& caminhoArquivo, std::function<void()> func) 
        : pcb{id, prioridade, quantum, nullptr, caminhoArquivo, base, limite, linhas, "Pronto"}, 
          processo(func) {}

    void iniciar() {
        pcb.estado = "Em execução";
        if (processo.joinable()) processo.join();
    }
};

#endif
