#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "unidadeControle.hpp"
#include <filesystem> // Para manipulação de diretórios

// ===== Execução Processos

void carregarProcessos(const string &diretorio);

void* executarProcesso(void* arg);

void LerInstrucoesDoArquivo(const string &nomeArquivo, int *registradores, PaginaMemoria *pm);

void atualizarTimestamps(vector<PaginaMemoria *> &filaPaginasMemoria, int quantumGasto, PaginaMemoria * pm);

void recalcularQuantum (PaginaMemoria *pm);

// ===== Escalonadores

void* executarCpu_FCFS(void* arg);

void* executarCpu_Loteria(void* arg);

void incrementarBilhetesNaoAtendidos(vector<PaginaMemoria *> &filaPaginasMemoria);

void* executarCpu_SJF(void* arg);

void ordenarFila_SJF();

bool compararPaginas_SJF(const PaginaMemoria* a, const PaginaMemoria* b);

void* executarCpu_RoundRobin(void* arg);

// ===== Funções auxiliares

pair<int,int> nLinhas(const string &nomeArquivo);

void LogSaida(const string &mensagem);

void imprimirDados (PaginaMemoria *pm);

#endif
