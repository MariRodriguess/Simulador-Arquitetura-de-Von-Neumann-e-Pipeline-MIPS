#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "unidadeControle.hpp"
#include <filesystem> // Para manipulação de diretórios

void carregarProcessos(const string &diretorio);

void LerInstrucoesDoArquivo(const string &nomeArquivo, int *registradores, PaginaMemoria *pm);

void LogSaida(const string &mensagem);

void* executarProcesso(void* arg);

void* executarCpu();

void atualizarTimestamps(std::vector<PaginaMemoria *> &filaPaginasMemoria, int quantumGasto);

void recalcularQuantum (PaginaMemoria *pm);

void imprimirDados (PaginaMemoria *pm);

#endif
