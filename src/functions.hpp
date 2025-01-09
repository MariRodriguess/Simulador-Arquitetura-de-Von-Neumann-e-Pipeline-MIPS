#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "unidadeControle.hpp"
#include <filesystem> // Para manipulação de diretórios

void carregarProcessos(const string &diretorio);

void LerInstrucoesDoArquivo(const string &nomeArquivo, int *registradores, PaginaMemoria *pm);

void LogSaida(const string &mensagem);

void* executarProcesso(void* arg);

void* executarCpu_FCFS(void* arg);

void* executarCpu_Loteria(void* arg);

void atualizarTimestamps(vector<PaginaMemoria *> &filaPaginasMemoria, int quantumGasto, PaginaMemoria * pm);

void recalcularQuantum (PaginaMemoria *pm);

void imprimirDados (PaginaMemoria *pm);

void incrementarBilhetesNaoAtendidos(vector<PaginaMemoria *> &filaPaginasMemoria);

#endif
