#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "include.hpp"
#include "ula.hpp"
#include "cpu.hpp"
#include "memoria.hpp"
#include "functions.hpp"

extern pthread_mutex_t lockPipeline;

void WriteBack(int resultado, int linhaAtual, PaginaMemoria *pm);
void MemoryAccess(int resultado, int *registradores, int info1, int linhaAtual, PaginaMemoria *pm);
void Execute(char instrucao, int info1, int info2, int info3, string info4, int *registradores, int linhaAtual, PaginaMemoria *pm);
void InstructionDecode(char instrucao, int info1, int info2, int info3, string info4, int *registradores, int linhaAtual, PaginaMemoria *pm);
void InstructionFetch(int *registradores, string linha, int linhaAtual, PaginaMemoria *pm);

#endif
