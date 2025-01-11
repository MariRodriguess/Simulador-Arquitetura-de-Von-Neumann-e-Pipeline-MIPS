#include "functions.hpp"

vector<PaginaMemoria*> filaPaginasMemoria; // Usando ponteiros

pthread_mutex_t filaMutex = PTHREAD_MUTEX_INITIALIZER; // Inicializando o mutex

int nArquivos = 0;

struct Args {
    PaginaMemoria* pm;// Ponteiro para a fila
};

unordered_map<char, int> mapaInstrucoes = {{'=',2}, {'+', 5}, {'-', 5}, {'*', 5}, {'/', 5}, {'$',3}, {'?', 3}};


void LerInstrucoesDoArquivo(const string &nomeArquivo, int *registradores, PaginaMemoria *pm) {
    
    ifstream arquivo(nomeArquivo);
    string linha;
    int linhaAtual = 1;
    tempoGasto[pm->pcb.idCpuAtual - 1] = 0;

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo!" << endl;
        return;
    }

    while (getline(arquivo, linha)) {

        UnidadeControle(registradores, linha, linhaAtual, pm);

        linhaAtual++;
        if(pm->pcb.estado == "Bloqueado"){
            cout << "\nProcesso " << pm->pcb.id << " bloqueado! - Periférico em uso\n";
            LogSaida("\nProcesso " + to_string(pm->pcb.id) + " bloqueado! - Periférico em uso\n");
            
            // Atualizar timestamps
            atualizarTimestamps(filaPaginasMemoria, tempoGasto[pm->pcb.idCpuAtual - 1],pm);
            pm->pcb.linhasProcessadasAnt = pm->pcb.linhasProcessadasAtual;
            pm->pcb.linhasProcessadasAtual = 0;
            pm->pcb.estado = "Bloqueado";
            pm->pcb.idCpuAtual = -1;
            break;
        }

        else if(pm->pcb.linhasProcessadasAtual == pm->pcb.linhasArquivo){

            cout << "\n\nProcesso " << pm->pcb.id + 1<< " encerrado!\n";

            // Atualizar estado do processo
            pthread_mutex_lock(&pm->mutex);
            pm->pcb.estado = "Finalizado";
            pm->pcb.idCpuAtual = -1;
            pthread_mutex_unlock(&pm->mutex);
            
            // Atualizar timestamps
            atualizarTimestamps(filaPaginasMemoria, tempoGasto[pm->pcb.idCpuAtual - 1],pm);

            imprimirDados(pm);

            //Liberar recursos associados
            for(int i = 0; i < (int)pm->pcb.recursos.size(); i++){
                perifericos[pm->pcb.recursos[i]] = true;
                pm->pcb.recursos.erase(pm->pcb.recursos.begin() + i);
            }
            
            // Remover o processo finalizado da fila
            pthread_mutex_lock(&filaMutex);
            filaPaginasMemoria.erase(filaPaginasMemoria.begin());
            /*
            //houve modificações no quantum
            if(SRTN){
                if(!filaPaginasMemoria.empty()){
                    ordenarFila_SRTN();
                    // pm = filaPaginasMemoria.front();
                }
            }
            */
            pthread_mutex_unlock(&filaMutex);

        }
        else if (pm->pcb.quantum == 0){

            cout << "\nQuantum insuficiente!\n";
            LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> Quantum esgotado!");
            
            // Atualizar timestamps
            atualizarTimestamps(filaPaginasMemoria, tempoGasto[pm->pcb.idCpuAtual - 1],pm);
            pm->pcb.linhasProcessadasAnt = pm->pcb.linhasProcessadasAtual;
            pm->pcb.linhasProcessadasAtual = 0;
            pm->pcb.estado = "Pronto";
            pm->jaFoiZeradoQuantum = true;
            pm->pcb.idCpuAtual = -1;

            //Liberar recursos associados
            for(int i = 0; i < (int)pm->pcb.recursos.size(); i++){
                perifericos[pm->pcb.recursos[i]] = true;
                pm->pcb.recursos.erase(pm->pcb.recursos.begin() + i);
            }
            break;
        }
        else{
            //houve modificações no quantum
            pthread_mutex_lock(&filaMutex);
            if(SRTN){
                if(!filaPaginasMemoria.empty()){
                    ordenarFila_SRTN();
                }
                if (pm != filaPaginasMemoria.front()){
                    pm->pcb.estado = "Pronto";
                    break;
                }
            }
            pthread_mutex_unlock(&filaMutex);
        }
    }

    arquivo.close();
}

pair<int,int> nLinhas(const string &nomeArquivo){
    ifstream arquivo(nomeArquivo);
    string linha;
    int cont = 0, quantumNecessario = 0;

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo!" << endl;
        return make_pair(0,0);
    }

    while (getline(arquivo, linha)) {
        if(linha.front() != '@'){
            quantumNecessario += mapaInstrucoes[linha.front()];
        }
        else{
            quantumNecessario = quantumNecessario + mapaInstrucoes[linha.front()] + mapaInstrucoes[linha.back()];
        }
        cont++;
    }

    arquivo.close();
    return make_pair(cont,quantumNecessario);
}

void* executarProcesso(void* arg) {

    sleep(2);
    auto* pm = static_cast<PaginaMemoria*>(arg);

    while (true) {

        pthread_mutex_lock(&pm->mutex); // Bloqueia até a CPU sinalizar para iniciar a execução

        // Sai do loop se o estado for "Finalizado"
        if (pm->pcb.estado == "Finalizado") {
            pthread_mutex_unlock(&pm->mutex);
            break; // Finaliza a thread
        }

        // Aguarda liberação da CPU
        cout << "Processo " << pm->pcb.id + 1 << " aguardando liberacao da CPU..." << endl;
        pthread_cond_wait(&pm->cond, &pm->mutex);

        // Coloca a thread em execução
        pm->pcb.estado = "Executando";
        pthread_mutex_unlock(&pm->mutex);
        LerInstrucoesDoArquivo(pm->pcb.caminhoArquivo, pm->pcb.registradores, pm);
        pthread_mutex_lock(&pm->mutex);
        
        if (pm->pcb.estado == "Pronto"){
            recalcularQuantum(pm);
            /*
            pthread_mutex_lock(&filaMutex);
            if(SRTN){
                if(!filaPaginasMemoria.empty()){
                    ordenarFila_SRTN();
                    //pm = filaPaginasMemoria.front();
                }
            }
            pthread_mutex_unlock(&filaMutex);
            */
            pm->pcb.recebeuRecurso = true; // Marcar processo como atendido
            pm->pcb.quantumOriginal = pm->pcb.quantum;
            incrementarBilhetesNaoAtendidos(filaPaginasMemoria); // Aumentar as chances para processos não atendidos a cada execução
        }
        else if(pm->pcb.estado == "Bloqueado"){
            cout << "\nProcesso bloqueado após acesso de periferico ocupado.\n";
            recalcularQuantum(pm);
            /*
            pthread_mutex_lock(&filaMutex);
            if(SRTN){
                if(!filaPaginasMemoria.empty()){
                    ordenarFila_SRTN();
                    //pm = filaPaginasMemoria.front();
                }
            }
            pthread_mutex_unlock(&filaMutex);
            */
            pm->pcb.quantumOriginal = pm->pcb.quantum;
            pm->pcb.recebeuRecurso = true; // Marcar processo como atendido
            incrementarBilhetesNaoAtendidos(filaPaginasMemoria); // Aumentar as chances para processos não atendidos a cada execução
        }else if (pm->pcb.estado == "Finalizado"){
            incrementarBilhetesNaoAtendidos(filaPaginasMemoria); // Aumentar as chances para processos não atendidos a cada execução
            /*
            pthread_mutex_lock(&filaMutex);
            if(SRTN){
                if(!filaPaginasMemoria.empty()){
                    ordenarFila_SRTN();
                    //pm = filaPaginasMemoria.front();
                }
            }
            pthread_mutex_unlock(&filaMutex);
            */
        }
        pthread_mutex_unlock(&pm->mutex);
    }
    return nullptr;
}

void* executarCpu_FCFS(void* arg) {

    CPU* cpu = static_cast<CPU*>(arg); // CPU específica
    int cont = 0;

    while (true) {

        PaginaMemoria* pm = nullptr;

        // Tenta pegar um processo da fila
        pthread_mutex_lock(&filaMutex);

        if (filaPaginasMemoria.empty()) {
            pthread_mutex_unlock(&filaMutex);
            break; // Sai do loop se a fila estiver vazia
        }

        int indicePm = cont % filaPaginasMemoria.size();
        cont++;
        pm = filaPaginasMemoria[indicePm]; 
        pm->pcb.quantum = 2147483647; // Para que o processo não sofra preempção
       
        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == -1){
                // Executa o processo
                pm->pcb.idCpuAtual = cpu->id;

                sleep(1);
                pthread_mutex_lock(&pm->mutex);
                sleep(1);
                pthread_cond_signal(&pm->cond); // Libera o processo para execução
                sleep(1);
                pthread_mutex_unlock(&pm->mutex);
            }
        } else {
            sleep(1); // Espera por novos processos
        }
        
    }
    return nullptr;
}

void* executarCpu_Loteria(void* arg) {

    CPU* cpu = static_cast<CPU*>(arg); // CPU específica

    while (true) {

        PaginaMemoria* pm = nullptr;

        // Tenta pegar um processo da fila
        pthread_mutex_lock(&filaMutex);

        if (filaPaginasMemoria.empty()) {
            pthread_mutex_unlock(&filaMutex);
            break; // Sai do loop se a fila estiver vazia
        }

        // Calcular o total de bilhetes
        int totalBilhetes = 0;
        for (auto pagina : filaPaginasMemoria) { 
            totalBilhetes += pagina->pcb.numBilhetes;
        }   

        // Sortear um bilhete
        int bilheteExecucao = rand() % totalBilhetes; // Gera um número entre 0 e totalBilhetes-1.

        int aux = 0;
        for (auto& pagina : filaPaginasMemoria) {
            aux += pagina->pcb.numBilhetes;
            if (aux > bilheteExecucao) {
                pm = pagina;
                break;
            }
        }

        // Processo 1 com 3 bilhetes → [0, 1, 2]
        // Processo 2 com 5 bilhetes → [3, 4, 5, 6, 7]
        // Processo 3 com 2 bilhetes → [8, 9] etc
        //
        // Se bilheteExecucao=4, pertence ao processo 2 [3, 4, 5, 6, 7] porque 4 < 8.
        // Se bilheteExecucao=9, pertence ao processo 3 [8, 9] porque 9 < 10.
       
        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == -1){
                // Executa o processo
                pm->pcb.idCpuAtual = cpu->id;

                sleep(1);
                pthread_mutex_lock(&pm->mutex);
                sleep(1);
                pthread_cond_signal(&pm->cond); // Libera o processo para execução
                sleep(1);
                pthread_mutex_unlock(&pm->mutex);
            }
        } else {
            sleep(1); // Espera por novos processos
        }
        
    }
    return nullptr;
}

bool compararPaginas_SJF(const PaginaMemoria* a, const PaginaMemoria* b) {
    return a->pcb.quantumNecessario < b->pcb.quantumNecessario;
}

void ordenarFila_SJF() {
    sort(filaPaginasMemoria.begin(), filaPaginasMemoria.end(), compararPaginas_SJF);
}

void* executarCpu_SJF(void* arg){
    CPU* cpu = static_cast<CPU*>(arg); // CPU específica

    ordenarFila_SJF();

    while (true) {

        PaginaMemoria* pm = nullptr;

        // Tenta pegar um processo da fila
        pthread_mutex_lock(&filaMutex);

        if (filaPaginasMemoria.empty()) {
            pthread_mutex_unlock(&filaMutex);
            break; // Sai do loop se a fila estiver vazia
        }

        pm = filaPaginasMemoria.front(); 
        pm->pcb.quantum = 2147483647; // Para que o processo não sofra preempção
       
        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == -1){
                // Executa o processo
                pm->pcb.idCpuAtual = cpu->id;

                sleep(1);
                pthread_mutex_lock(&pm->mutex);
                sleep(1);
                pthread_cond_signal(&pm->cond); // Libera o processo para execução
                sleep(1);
                pthread_mutex_unlock(&pm->mutex);
            }
        } else {
            sleep(1); // Espera por novos processos
        }
        
    }
    return nullptr;
}

bool compararPaginas_SRTN(const PaginaMemoria* a, const PaginaMemoria* b) {
    return a->pcb.quantum < b->pcb.quantum;
}

void ordenarFila_SRTN() {

    /*
    cout << "\nAntes da ordenação:\n";
    for (auto *pagina : filaPaginasMemoria) { 
        cout << "Processo " << pagina->pcb.id+1 << "(" << pagina->pcb.quantum << ")"<< " - ";
    }
    */
    sleep(1);
    sort(filaPaginasMemoria.begin(), filaPaginasMemoria.end(), compararPaginas_SRTN);
    sleep(1);
    /*
    cout << "\nDepois da ordenação:\n";
    for (auto *pagina : filaPaginasMemoria) { 
        cout << "Processo " << pagina->pcb.id+1 << "(" << pagina->pcb.quantum << ")"<< " - ";
    }
    cout << endl;
    */

}

void* executarCpu_SRTN(void* arg){
    
    CPU* cpu = static_cast<CPU*>(arg); // CPU específica

    while (true) {

        PaginaMemoria* pm = nullptr;

        // Tenta pegar um processo da fila
        pthread_mutex_lock(&filaMutex);

        if (filaPaginasMemoria.empty()) {
            pthread_mutex_unlock(&filaMutex);
            break; // Sai do loop se a fila estiver vazia
        }else{
            ordenarFila_SRTN();
            pm = filaPaginasMemoria.front();
        }
        
        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == -1){
                // Executa o processo
                pm->pcb.idCpuAtual = cpu->id;

                sleep(1);
                pthread_mutex_lock(&pm->mutex);
                sleep(1);
                pthread_cond_signal(&pm->cond); // Libera o processo para execução
                sleep(1);
                pthread_mutex_unlock(&pm->mutex);
            }
        } else {
            sleep(1); // Espera por novos processos
        }
        
    }
    return nullptr;
}

void carregarProcessos(const string& diretorio) {
    namespace fs = filesystem;
    int idProcesso = 0;

    for (const auto& entry : fs::directory_iterator(diretorio)) {
        if (entry.is_regular_file() && entry.path().extension() == ".data") {
            nArquivos++;
            PCB processo;
            processo.id = idProcesso++;
            processo.prioridade = rand() % 10;
            processo.quantum = (rand() % 6) + 1;
            //processo.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50
            processo.quantumOriginal = processo.quantum;
            processo.registradores = (int*)malloc(10 * sizeof(int));
            processo.caminhoArquivo = entry.path().string();
            processo.estado = "Pronto";
            auto resultado = nLinhas(processo.caminhoArquivo);
            processo.linhasArquivo = resultado.first;
            processo.quantumNecessario = resultado.second;
            processo.idCpuAtual = -1;
            processo.numBilhetes = processo.prioridade * 2;
            processo.recebeuRecurso = false;

            // Inicializa o semáforo
            sem_init(&processo.semaforo, 0, 1);

            auto* pm = new PaginaMemoria();
            pm->pcb = processo;
            
            pthread_mutex_init(&pm->mutex, nullptr);
            pthread_cond_init(&pm->cond, nullptr);

            if (pthread_create(&pm->th_processo, nullptr, executarProcesso, pm) != 0) {
                cout << "Erro ao criar a thread para o processo " << processo.id << endl;
                delete pm;
                continue;
            }

            filaPaginasMemoria.push_back(pm);
            cout << "\nCarregando processo " << processo.id + 1 << " do arquivo: " << processo.caminhoArquivo << endl;

            sleep(3);
        }
    }
}

void LogSaida(const string &mensagem) {
    ofstream logFile("log_output.txt", ios::app); // Abre o arquivo no modo append
    time_t agora = time(0);                       // Pega o tempo atual
    char *dt = ctime(&agora);                     // Converte para string legível

    // Remove o '\n' ao final da string de tempo
    string tempoAtual(dt);
    tempoAtual.pop_back();

    // Mensagem formatada
    // cout << "[" << tempoAtual << "] " << mensagem << endl; // Exibe no console
    if (logFile.is_open()) {
        logFile << mensagem << endl; // Escreve no arquivo
        logFile.close();
    }
}

void recalcularQuantum (PaginaMemoria *pm){
    
    pm->pcb.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50 (inclusive)
    //pm->pcb.quantum = (rand() % 6) + 1;
}

void imprimirDados (PaginaMemoria *pm){

    if (!FCFS){
        LogSaida("\nProcesso " + to_string(pm->pcb.id + 1) + " encerrado!" + 
            "\nDados finais: \nLinhas Processadas: " + to_string(pm->pcb.linhasProcessadasAtual) +  
            "\nPrioridade: " + to_string(pm->pcb.prioridade) + 
            "\nNúmeros de bilhetes: " + to_string(pm->pcb.numBilhetes) + 
            "\nTimestamp: " + to_string(pm->pcb.timestamp) + 
            "\nQuantum original: " + to_string(pm->pcb.quantumOriginal) +
            "\nQuantum necessário: " + to_string(pm->pcb.quantumNecessario) +
            "\nQuantum final: " + to_string(pm->pcb.quantum) + "\n");
    }else{
        LogSaida("\nProcesso " + to_string(pm->pcb.id + 1) + " encerrado!" + 
            "\nDados finais: \nLinhas Processadas: " + to_string(pm->pcb.linhasProcessadasAtual) +  
            "\nPrioridade: " + to_string(pm->pcb.prioridade) + 
            "\nNúmeros de bilhetes: " + to_string(pm->pcb.numBilhetes) + 
            "\nQuantum necessário: " + to_string(pm->pcb.quantumNecessario) +
            "\nTimestamp: " + to_string(pm->pcb.timestamp) + "\n");
    }
    
}

void atualizarTimestamps(vector<PaginaMemoria *> &filaPaginasMemoria, int quantumGasto, PaginaMemoria * pm) {

    pthread_mutex_lock(&filaMutex); // Protege a fila contra acesso simultâneo
    for (auto *pagina : filaPaginasMemoria) { // Itera sobre os ponteiros
        if((pm->pcb.idCpuAtual == pagina->pcb.idCpuAtual) || (pagina->pcb.idCpuAtual == -1)){
            pagina->pcb.timestamp += quantumGasto; // Atualiza o timestamp
        }
    }
    pthread_mutex_unlock(&filaMutex);
}

void incrementarBilhetesNaoAtendidos(vector<PaginaMemoria *> &filaPaginasMemoria) {

    pthread_mutex_lock(&filaMutex);
    for (auto& pagina : filaPaginasMemoria) {
        if (!pagina->pcb.recebeuRecurso) {
            pagina->pcb.numBilhetes++; // Incrementa os bilhetes
        }
    }
    pthread_mutex_unlock(&filaMutex);
}
    