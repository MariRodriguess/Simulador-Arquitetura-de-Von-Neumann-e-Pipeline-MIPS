#include "mmu.hpp"

string transfDecimalBinario(int n) {

    if (n == 0){
        return "0";
    }

    string binario = "";
    while (n > 0) {
        binario = to_string(n % 2) + binario;
        n /= 2;
    }
    return binario;
}

int transBinarioDecimal(const string &binario) {

    int decimal = 0;

    for (char c : binario) {
        decimal = decimal * 2 + (c - '0');
    }
    return decimal;
}