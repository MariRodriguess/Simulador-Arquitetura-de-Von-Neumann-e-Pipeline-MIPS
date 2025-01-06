#include "functions.hpp"

vector<PaginaMemoria*> filaPaginasMemoria; // Usando ponteiros

pthread_mutex_t filaMutex = PTHREAD_MUTEX_INITIALIZER; // Inicializando o mutex

int nArquivos = 0;

struct Args {
    PaginaMemoria* pm;// Ponteiro para a fila
};


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
    }

    arquivo.close();
}

int nLinhas(const string &nomeArquivo){
    ifstream arquivo(nomeArquivo);
    string linha;
    int cont = 0;

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo!" << endl;
        return 0;
    }

    while (getline(arquivo, linha)) {
        cont++;
    }

    arquivo.close();
    return cont;
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
        }
        else if(pm->pcb.estado == "Bloqueado"){
            cout << "Processo bloqueado após acesso de periferico ocupado.\n";
            recalcularQuantum(pm);
        }
        pthread_mutex_unlock(&pm->mutex);
    }
    return nullptr;
}

void* executarCpu(void* arg) {

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


// Atualize `carregarProcessos` para inicializar o semáforo
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
            processo.registradores = (int*)malloc(10 * sizeof(int));
            processo.caminhoArquivo = entry.path().string();
            processo.estado = "Pronto";
            processo.linhasArquivo = nLinhas(processo.caminhoArquivo);
            processo.idCpuAtual = -1;

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
    LogSaida("\nProcesso " + to_string(pm->pcb.id + 1) + " encerrado!" + 
            "\nDados finais: \nLinhas Processadas: " + to_string(pm->pcb.linhasProcessadasAtual) +  
            "\nTimestamp: " + to_string(pm->pcb.timestamp) + 
            "\nQuantum final: " + to_string(pm->pcb.quantum) + "\n");
}

// Atualizar timestamps
void atualizarTimestamps(std::vector<PaginaMemoria *> &filaPaginasMemoria, int quantumGasto, PaginaMemoria * pm) {

    pthread_mutex_lock(&filaMutex); // Protege a fila contra acesso simultâneo
    for (auto *pagina : filaPaginasMemoria) { // Itera sobre os ponteiros
        if((pm->pcb.idCpuAtual == pagina->pcb.idCpuAtual) || (pagina->pcb.idCpuAtual == -1)){
            pagina->pcb.timestamp += quantumGasto; // Atualiza o timestamp
        }
    }
    pthread_mutex_unlock(&filaMutex);
}
    