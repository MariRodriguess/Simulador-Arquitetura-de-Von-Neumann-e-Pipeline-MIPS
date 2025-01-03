#include "unidadeControle.hpp"

void UnidadeControle(int *registradores, string linha, int linhaAtual, PaginaMemoria *pm)
{
    InstructionFetch(registradores, linha, linhaAtual, pm);
}