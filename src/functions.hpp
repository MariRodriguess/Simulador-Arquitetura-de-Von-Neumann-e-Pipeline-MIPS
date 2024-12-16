#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "include.hpp"
#include <filesystem> // Para manipulação de diretórios

void carregarProcessos(const string &diretorio, queue<PCB> &filaProcessos);

void LerInstrucoesDoArquivo(const string &nomeArquivo, int *registradores);

void LogSaida(const string &mensagem);

#endif
