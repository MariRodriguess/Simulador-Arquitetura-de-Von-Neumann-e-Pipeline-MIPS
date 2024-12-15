#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "include.hpp"

void WriteBack(int resultado);
void MemoryAccess(int resultado, int *registradores, int info1);
void Execute(char instrucao, int info1, int info2, int info3, string info4, int *registradores);
void InstructionDecode(char instrucao, int info1, int info2, int info3, string info4, int *registradores);
void InstructionFetch(int *registradores, string linha);

#endif