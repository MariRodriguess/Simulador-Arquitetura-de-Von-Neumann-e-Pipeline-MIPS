#include "include.hpp"
#include "ula.hpp"

int ULA(int operando1, int operando2, char operacao) {

    if (operacao == '+') return operando1 + operando2;
    else if (operacao == '-') return operando1 - operando2;
    else if (operacao == '*') return operando1 * operando2;
    else if (operacao == '/') return operando2 != 0 ? operando1 / operando2 : 0; 
    
    return 0;
}