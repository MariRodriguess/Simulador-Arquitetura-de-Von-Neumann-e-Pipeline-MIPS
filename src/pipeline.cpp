#include "pipeline.hpp"

void WriteBack(int resultado, int linhaAtual, PaginaMemoria *pm) {

    // cout << "\nWrite back." << endl;

    principal.push_back(resultado);
    CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa WRITEBACK

    if (linhaAtual > pm->pcb.linhasProcessadasAnt){
        pm->pcb.quantum -= 1;
        tempoGasto[pm->pcb.idCpuAtual]++;
        
        if(pm->pcb.quantum == 0){
            return;
        }
    }

    pm->pcb.linhasProcessadasAtual++;
    LogSaida("CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: WRITEBACK, Escrevendo: " + to_string(resultado) + " na memoria.");
}


void MemoryAccess(int resultado, int *registradores, int info1, int linhaAtual, PaginaMemoria *pm) {

    // cout << "\nMemoryAcess." << endl;

    registradores[info1] = resultado;
    //cout << "\nResultado = " << resultado << endl;

    CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa MEMORY_ACCESS

    if (linhaAtual > pm->pcb.linhasProcessadasAnt){
        pm->pcb.quantum -= 1; 
        tempoGasto[pm->pcb.idCpuAtual]++;
        
        if(pm->pcb.quantum == 0){
            return;
        }
    }
    

    LogSaida("CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: MEMORY_ACCESS, Resultado: " + to_string(resultado));

    WriteBack(resultado, linhaAtual, pm);
}

void Execute(char instrucao, int info1, int info2, int info3, string info4, int *registradores, int linhaAtual, PaginaMemoria *pm) {

    // cout << "\nExecute." << endl;

    int soma = 0, registradorAtual = info1;

    if (instrucao == '=') {

        registradores[info1] = info2;
        LogSaida("CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao);

        pm->pcb.linhasProcessadasAtual++;

        // Para que considere operaçoes com "=" como processadas também
        if (linhaAtual > pm->pcb.linhasProcessadasAnt){
            pm->pcb.quantum -= 1;
            tempoGasto[pm->pcb.idCpuAtual]++;
            
            if(pm->pcb.quantum == 0){
                return;
            }
        }  

    } else if (instrucao == '@') {

        for (int i = 0; i < info3; i++) {
            soma += registradores[registradorAtual];
            registradorAtual++;
            if (registradorAtual > info2) {
                registradorAtual = info1;
            }
            CLOCK[pm->pcb.idCpuAtual - 1]++; // CLOCK incrementado durante loop de somas

            if (linhaAtual > pm->pcb.linhasProcessadasAnt){
                pm->pcb.quantum -= 1;
                tempoGasto[pm->pcb.idCpuAtual]++;
                
                if(pm->pcb.quantum == 0){
                    return;
                }
            }

        }
        LogSaida("CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao);
        MemoryAccess(soma, registradores, info1, linhaAtual, pm);

    } else if ((instrucao != '&') && (instrucao != '@') && (instrucao != '?')) {

        int resultado = ULA(registradores[info2], registradores[info3], instrucao);
        CLOCK[pm->pcb.idCpuAtual - 1]++; // CLOCK incrementado antes de chamar a próxima etapa

        if (linhaAtual > pm->pcb.linhasProcessadasAnt){
            pm->pcb.quantum -= 1;
            tempoGasto[pm->pcb.idCpuAtual]++;

            if(pm->pcb.quantum == 0){
                return;
            }
        }

        LogSaida("CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao);
        MemoryAccess(resultado, registradores, info1, linhaAtual, pm);

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
        CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na comparação

        LogSaida("CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao);

        // Para que considere operaçoes com "?" como processadas também
        pm->pcb.linhasProcessadasAtual++;

        if (linhaAtual > pm->pcb.linhasProcessadasAnt){
            pm->pcb.quantum -= 1;
            tempoGasto[pm->pcb.idCpuAtual]++;
            
            if(pm->pcb.quantum == 0){
                return;
            }
        }
        
    }
}

void InstructionDecode(char instrucao, int info1, int info2, int info3, string info4, int *registradores, int linhaAtual, PaginaMemoria *pm) {

    // cout << "\nInstruction Decode." << endl;

    CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa DECODE
    if (linhaAtual > pm->pcb.linhasProcessadasAnt){
        pm->pcb.quantum -= 1; 
        tempoGasto[pm->pcb.idCpuAtual]++;
        
        if(pm->pcb.quantum == 0){
            return;
        }
    }
    
    LogSaida("CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: DECODE ");
    Execute(instrucao, info1, info2, info3, info4, registradores, linhaAtual, pm);
}

void InstructionFetch(int *registradores, string linha, int linhaAtual, PaginaMemoria *pm) {

    // cout << "\nInstruction Fetch." << endl;

    char instrucao;
    int info1 = 0, info2 = 0, info3 = 0;
    string info4 = "";

    stringstream ss(linha);
    ss >> instrucao >> info1;

    if (instrucao != '&') ss >> info2;
    if ((instrucao != '=') && (instrucao != '?')) ss >> info3;
    if (instrucao == '?') ss >> info4;

    CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa FETCH

    if (linhaAtual > pm->pcb.linhasProcessadasAnt){
        pm->pcb.quantum -= 1; 
        tempoGasto[pm->pcb.idCpuAtual]++;
        
        if(pm->pcb.quantum == 0){
            return;
        }
    }
    
    LogSaida("CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: FETCH, Instrucao: " + linha);

    InstructionDecode(instrucao, info1, info2, info3, info4, registradores, linhaAtual, pm);
    PC++;
}