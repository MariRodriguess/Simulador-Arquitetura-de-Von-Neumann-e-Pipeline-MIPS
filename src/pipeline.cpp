#include "pipeline.hpp"
#include "ula.hpp"
#include "functions.hpp"
#include "cpu.hpp"

#include "include.hpp"

void WriteBack(int resultado) {

    principal.push_back(resultado);
    CLOCK++; // Incremento de clock na etapa WRITEBACK
    LogSaida("CLOCK: " + to_string(CLOCK) + ", PC: " + to_string(PC) + ", ETAPA: WRITEBACK, Escrevendo: " + to_string(resultado) + " na memoria.");
}


void MemoryAccess(int resultado, int *registradores, int info1) {

    registradores[info1] = resultado;
    //cout << "\nResultado = " << resultado << endl;

    CLOCK++; // Incremento de clock na etapa MEMORY_ACCESS
    LogSaida("CLOCK: " + to_string(CLOCK) + ", PC: " + to_string(PC) + ", ETAPA: MEMORY_ACCESS, Resultado: " + to_string(resultado));

    WriteBack(resultado);
}

void Execute(char instrucao, int info1, int info2, int info3, string info4, int *registradores) {
    int soma = 0, registradorAtual = info1;

    if (instrucao == '=') {

        registradores[info1] = info2;
        LogSaida("CLOCK: " + to_string(CLOCK) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao);

    } else if (instrucao == '@') {

        for (int i = 0; i < info3; i++) {
            soma += registradores[registradorAtual];
            registradorAtual++;
            if (registradorAtual > info2) {
                registradorAtual = info1;
            }
            CLOCK++; // CLOCK incrementado durante loop de somas
        }
        LogSaida("CLOCK: " + to_string(CLOCK) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao);
        MemoryAccess(soma, registradores, info1);

    } else if ((instrucao != '&') && (instrucao != '@') && (instrucao != '?')) {

        int resultado = ULA(registradores[info2], registradores[info3], instrucao);
        CLOCK++; // CLOCK incrementado antes de chamar a próxima etapa
        LogSaida("CLOCK: " + to_string(CLOCK) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao);
        MemoryAccess(resultado, registradores, info1);

    } else if (instrucao == '?') {

        cout << registradores[info1] << " " << info4 << " " << registradores[info2] << ": ";
        
        if (info4 == "<") {
            cout << (registradores[info1] < registradores[info2] ? "True" : "False") << endl;
        }
        else if (info4 == ">") {
            cout << (registradores[info1] > registradores[info2] ? "True" : "False") << endl;
        }
        else if (info4 == "=") {
            cout << (registradores[info1] == registradores[info2] ? "True" : "False") << endl;
        }
        else if (info4 == "!") {
            cout << (registradores[info1] != registradores[info2] ? "True" : "False") << endl;
        }
        CLOCK++; // Incremento de clock na comparação
        LogSaida("CLOCK: " + to_string(CLOCK) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao);

    }
}

void InstructionDecode(char instrucao, int info1, int info2, int info3, string info4, int *registradores) {
    CLOCK++; // Incremento de clock na etapa DECODE
    LogSaida("CLOCK: " + to_string(CLOCK) + ", PC: " + to_string(PC) + ", ETAPA: DECODE ");
    Execute(instrucao, info1, info2, info3, info4, registradores);
}

void InstructionFetch(int *registradores, string linha) {
    char instrucao;
    int info1 = 0, info2 = 0, info3 = 0;
    string info4 = "";

    stringstream ss(linha);
    ss >> instrucao >> info1;

    if (instrucao != '&') ss >> info2;
    if ((instrucao != '=') && (instrucao != '?')) ss >> info3;
    if (instrucao == '?') ss >> info4;

    CLOCK++; // Incremento de clock na etapa FETCH

    LogSaida("CLOCK: " + to_string(CLOCK) + ", PC: " + to_string(PC) + ", ETAPA: FETCH, Instrucao: " + linha);

    InstructionDecode(instrucao, info1, info2, info3, info4, registradores);
    PC++;
}
