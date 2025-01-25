#include "pipeline.hpp"

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
unordered_map<string, int> cacheInstrucoes;
const int CACHE_SIZE = 50;

vector<string> infosInstrucaoAtual[NUM_CPUS];
bool instrucaoExiste[NUM_CPUS];

void WriteBack(int resultado, int linhaAtual, PaginaMemoria *pm) {
    
    principal.push_back(resultado);
    // CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa WRITEBACK

    if (linhaAtual > pm->pcb.linhasProcessadasAnt){

        pm->pcb.quantum -= 1;
        CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa WRITEBACK
        
        if(pm->pcb.quantum == 0){
            infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
            return;
        }
    }

    pm->pcb.linhasProcessadasAtual++;
    if(pm->pcb.linhasProcessadasAtual == pm->pcb.linhasArquivo){
        infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
    }
    infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
    LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: WRITEBACK, Escrevendo: " + to_string(resultado) + " na memoria. (CPU " + to_string(pm->pcb.idCpuAtual) + ")");
}


void MemoryAccess(int resultado, int *registradores, int info1, int linhaAtual, PaginaMemoria *pm) {

    registradores[info1] = resultado;

    // CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa MEMORY_ACCESS

    if (linhaAtual > pm->pcb.linhasProcessadasAnt){

        pm->pcb.quantum -= 1; 
        CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa MEMORY_ACCESS
        
        if(pm->pcb.quantum == 0){
            infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
            return;
        }
    }
    

    LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: MEMORY_ACCESS, Resultado: " + to_string(resultado) + " (CPU " + to_string(pm->pcb.idCpuAtual) + ")");

    WriteBack(resultado, linhaAtual, pm);
}

void Execute(char instrucao, int info1, int info2, int info3, string info4, int *registradores, int linhaAtual, PaginaMemoria *pm) {

    int soma = 0, registradorAtual = info1;

    if (instrucao == '=') {

        registradores[info1] = info2;
        LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao  + " (CPU " + to_string(pm->pcb.idCpuAtual) + ")");

        pm->pcb.linhasProcessadasAtual++;
        if(pm->pcb.linhasProcessadasAtual == pm->pcb.linhasArquivo){
            infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
        }

        if(pm->pcb.quantum == 0){
            infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
            return;
        } 

    } else if (instrucao == '@') {

        for (int i = 0; i < info3; i++) {
            soma += registradores[registradorAtual];
            registradorAtual++;
            if (registradorAtual > info2) {
                registradorAtual = info1;
            }
            // CLOCK[pm->pcb.idCpuAtual - 1]++; // CLOCK incrementado durante loop de somas

            if (linhaAtual > pm->pcb.linhasProcessadasAnt){

                pm->pcb.quantum -= 1;
    
                CLOCK[pm->pcb.idCpuAtual - 1]++; // CLOCK incrementado durante loop de somas
                
                if(pm->pcb.quantum == 0){
                    infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
                    return;
                }
            }

        }
        LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao + " (CPU " + to_string(pm->pcb.idCpuAtual) + ")");
        MemoryAccess(soma, registradores, info1, linhaAtual, pm);

    } else if ((instrucao != '&') && (instrucao != '@') && (instrucao != '?') && (instrucao != '$')) {
        //+ 3 0 1

        int resultado = ULA(registradores[info2], registradores[info3], instrucao);
        if(!(instrucaoExiste[pm->pcb.idCpuAtual-1]) && (infosInstrucaoAtual[pm->pcb.idCpuAtual-1].size() == 3)){
            string hashInstrucao = infosInstrucaoAtual[pm->pcb.idCpuAtual-1][0] + " " + infosInstrucaoAtual[pm->pcb.idCpuAtual-1][1] + " " + infosInstrucaoAtual[pm->pcb.idCpuAtual-1][2];
            adicionarAoCache(hashInstrucao, resultado);
        }
        // CLOCK[pm->pcb.idCpuAtual - 1]++; // CLOCK incrementado antes de chamar a próxima etapa

        if (linhaAtual > pm->pcb.linhasProcessadasAnt){

            pm->pcb.quantum -= 1;

            CLOCK[pm->pcb.idCpuAtual - 1]++; // CLOCK incrementado antes de chamar a próxima etapa

            if(pm->pcb.quantum == 0){
                infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
                return;
            }
        }

        LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao + " (CPU " + to_string(pm->pcb.idCpuAtual) + ")");
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
        // CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na comparação

        LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao + " (CPU " + to_string(pm->pcb.idCpuAtual) + ")");

        // Para que considere operaçoes com "?" como processadas também
        pm->pcb.linhasProcessadasAtual++;
        if(pm->pcb.linhasProcessadasAtual == pm->pcb.linhasArquivo){
            infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
        }
        if (linhaAtual > pm->pcb.linhasProcessadasAnt){

            pm->pcb.quantum -= 1;

            CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na comparação
            
            if(pm->pcb.quantum == 0){
                infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
                return;
            }
        }
        
    }else if (instrucao == '$'){

        pthread_mutex_lock(&mtx);
        
        if(perifericos[info1]){ //liberado

            perifericos[info1] = false;
            pthread_mutex_unlock(&mtx);
            pm->pcb.recursos.push_back(info1);

            cout << "\nAcesso ao periferico $ " << info1 << endl;

            // CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na comparação

            LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: EXECUTE, Instrucao: " + instrucao+ " (CPU " + to_string(pm->pcb.idCpuAtual) + ")");

            // Para que considere operaçoes com "$" como processadas também
            pm->pcb.linhasProcessadasAtual++;
            if(pm->pcb.linhasProcessadasAtual == pm->pcb.linhasArquivo){
                infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
            }
            if (linhaAtual > pm->pcb.linhasProcessadasAnt){

                pm->pcb.quantum -= 1;
    
                CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na comparação
                
                if(pm->pcb.quantum == 0){
                    infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
                    return;
                }
            }
        }
        else{

            pthread_mutex_unlock(&mtx);
            pm->pcb.estado = "Bloqueado";
            cout << "\nPeriferico $ " << info1 << " esta bloqueado!\n";

            //Liberar recursos associados - Exceto periférico inacessível
            for(int i = 0; i < (int)pm->pcb.recursos.size(); i++){
                perifericos[pm->pcb.recursos[i]] = true;
                pm->pcb.recursos.erase(pm->pcb.recursos.begin() + i);
            }
            infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();

            return;
        }
    }
}

void InstructionDecode(char instrucao, int info1, int info2, int info3, string info4, int *registradores, int linhaAtual, PaginaMemoria *pm) {

    // CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa DECODE
    if (linhaAtual > pm->pcb.linhasProcessadasAnt){

        pm->pcb.quantum -= 1; 
        CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa DECODE
        
        if(pm->pcb.quantum == 0){
            infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
            return;
        }
    }
    
    LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: DECODE " + " (CPU " + to_string(pm->pcb.idCpuAtual) + ")");
    Execute(instrucao, info1, info2, info3, info4, registradores, linhaAtual, pm);
}

void InstructionFetch(int *registradores, string linha, int linhaAtual, PaginaMemoria *pm) { 
    char instrucao;
    int info1 = 0, info2 = 0, info3 = 0;
    string info4 = "";
    string hashInstrucao = "";
    instrucaoExiste[pm->pcb.idCpuAtual-1] = false;

    stringstream ss(linha);
    ss >> instrucao >> info1;

    if (instrucao != '&') ss >> info2;
    if ((instrucao != '=') && (instrucao != '?')) ss >> info3;
    if (instrucao == '?') ss >> info4;
    // = 0 8
    // = 1 10
    // + 3 1 0
    if((infosInstrucaoAtual[pm->pcb.idCpuAtual-1].empty()) && (instrucao == '=')){//adicionar primeiro valor
        infosInstrucaoAtual[pm->pcb.idCpuAtual-1].push_back(to_string(info2));
    }
    else if((infosInstrucaoAtual[pm->pcb.idCpuAtual-1].size() == 1) && (instrucao == '=')){//adicionar segundo valor
        infosInstrucaoAtual[pm->pcb.idCpuAtual-1].push_back(to_string(info2));
    }
    else if((infosInstrucaoAtual[pm->pcb.idCpuAtual-1].size() == 2) && ((instrucao == '+') || (instrucao == '-') || (instrucao == '*') || (instrucao == '/'))){//adicionar instrucao
        infosInstrucaoAtual[pm->pcb.idCpuAtual-1].push_back(to_string(instrucao));
    }
    // 8 10 +

    if(infosInstrucaoAtual[pm->pcb.idCpuAtual-1].size() == 3){
        hashInstrucao = infosInstrucaoAtual[pm->pcb.idCpuAtual-1][0] + " " + infosInstrucaoAtual[pm->pcb.idCpuAtual-1][1] + " " + infosInstrucaoAtual[pm->pcb.idCpuAtual-1][2];
        if(verificarCache(hashInstrucao)){
            cout << "\nHash reutilizada: " << hashInstrucao << " -> processo " << pm->pcb.id+1 << endl;
            LogSaida("Processo " + to_string(pm->pcb.id+1) + ": Reutilização de informações presentes na cache para execução da instrução: " + hashInstrucao);
            instrucaoExiste[pm->pcb.idCpuAtual-1] = true;
            MemoryAccess(cacheInstrucoes[hashInstrucao], registradores, info1, linhaAtual, pm);
        }
        //infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
    }

    /*cout << "Hash construida: " << hashInstrucao << " - Processo: " << pm->pcb.id+1 << " - Tamanho infoIntrucao: " << infosInstrucaoAtual[pm->pcb.idCpuAtual-1].size() << " - ID CPU: " << pm->pcb.idCpuAtual << endl;
    for (const auto& par : cacheInstrucoes) {
        std::cout << "Chave: " << par.first << ", Valor: " << par.second << std::endl;
    }*/


    if(!instrucaoExiste[pm->pcb.idCpuAtual-1]){
        // CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa FETCH

        if (linhaAtual > pm->pcb.linhasProcessadasAnt){

            pm->pcb.quantum -= 1; 
            CLOCK[pm->pcb.idCpuAtual - 1]++; // Incremento de clock na etapa FETCH
            
            if(pm->pcb.quantum == 0){
                infosInstrucaoAtual[pm->pcb.idCpuAtual-1].clear();
                return;
            }
        }
        
        LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> " + "CLOCK "+ to_string(pm->pcb.idCpuAtual) +": " + to_string(CLOCK[pm->pcb.idCpuAtual - 1]) + ", PC: " + to_string(PC) + ", ETAPA: FETCH, Instrucao: " + linha + " (CPU " + to_string(pm->pcb.idCpuAtual) + ")");

        InstructionDecode(instrucao, info1, info2, info3, info4, registradores, linhaAtual, pm);
    }
    PC++;
}