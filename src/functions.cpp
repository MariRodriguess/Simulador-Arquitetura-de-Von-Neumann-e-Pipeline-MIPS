#include "functions.hpp"

// vector<PaginaMemoria> filaPaginasMemoria;

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
            
            // Atualizar timestamps
            atualizarTimestamps(filaPaginasMemoria, tempoGasto[pm->pcb.idCpuAtual - 1],pm);
            pm->pcb.linhasProcessadasAnt = pm->pcb.linhasProcessadasAtual;
            pm->pcb.linhasProcessadasAtual = 0;
            pm->pcb.estado = "Bloqueado";
            pm->pcb.idCpuAtual = -1;
            break;
        }

        else if(pm->pcb.linhasProcessadasAtual == pm->pcb.linhasArquivo){

            cout << "\n\nProcesso " << pm->pcb.id + 1<< " encerrado! CPU: " << pm->pcb.idCpuAtual << endl;


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

/*
void* executarCpu() {

    int cont = 0;
    while (true) {

        PaginaMemoria* pm = nullptr;

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

            if (pm->pcb.estado == "Pronto") {

                // Sinaliza que o processo pode começar
                pthread_mutex_lock(&pm->mutex);

                // pm->pcb.estado = "Em execução";
                cout << "\n ======= \nCPU esta ativando o processo " << pm->pcb.id << endl;

                sleep(1);
                pthread_cond_signal(&pm->cond); // Libera o processo
                sleep(1);
                pthread_mutex_unlock(&pm->mutex);
                sleep(1);
            }
            else if(pm->pcb.estado == "Bloqueado"){
                // Sinaliza que o processo pode começar
                pthread_mutex_lock(&pm->mutex);

                // pm->pcb.estado = "Em execução";
                cout << "\n ======= \nCPU esta tentando ativar o processo " << pm->pcb.id << ", previamente bloqueado.\n";

                sleep(1);
                pthread_cond_signal(&pm->cond); // Libera o processo
                sleep(1);
                pthread_mutex_unlock(&pm->mutex);
                sleep(1);
            }

            if(filaPaginasMemoria.size() == 0){
                break;
            }

        } else {
            cout << "Fila vazia, aguardando novos processos..." << endl;
            sleep(1);
        }
    }
    return nullptr;
}
*/

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

        // Após liberação
        cout << "Processo " << pm->pcb.id << " em execucao por " << pm->pcb.quantum << " unidades de tempo.";

        // Coloca a thread em execução
        pm->pcb.estado = "Em execução";
        pthread_mutex_unlock(&pm->mutex);
        LerInstrucoesDoArquivo(pm->pcb.caminhoArquivo, pm->pcb.registradores, pm);
        pthread_mutex_lock(&pm->mutex);

        cout << "\nSaindo da execucao com o estado: " << pm->pcb.estado << endl;
        
        if (pm->pcb.estado == "Pronto"){
            cout << "Com " << pm->pcb.linhasProcessadasAnt << " linha(s) processada(s) ate o momento." << endl;
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
       
        // Procura um processo não atribuído a nenhuma CPU
        /*for (auto* pagina : filaPaginasMemoria) {
            if ((pagina->pcb.estado == "Pronto" || pagina->pcb.estado == "Bloqueado") && pagina->pcb.idCpuAtual == -1) {
                pm = pagina;
                pagina->pcb.idCpuAtual = cpu->id; // Marca que este processo está sendo processado por esta CPU
                break;
            }
        }*/

        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == -1){
                // Executa o processo
                
                cout << "\nCPU " << cpu->id << " esta ativando o processo " << pm->pcb.id + 1 << endl;
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
            // processo.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50
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
            cout << "\nCarregando processo " << processo.id + 1<< " do arquivo: " << processo.caminhoArquivo << endl;

            sleep(3);
        }
    }
}

/*
void carregarProcessos(const string &diretorio) {
    
    namespace fs = filesystem;
    int idProcesso = 0;

    for (const auto &entry : fs::directory_iterator(diretorio)) {
        if (entry.is_regular_file() && entry.path().extension() == ".data") {
            nArquivos++;
            PCB processo;
            processo.id = idProcesso;
            idProcesso++;
            processo.prioridade = rand() % 10;
            // processo.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50
            processo.quantum = (rand() % 6) + 1; 
            processo.registradores = (int *)malloc(10 * sizeof(int)); // Alocando os registradores do processo
            processo.caminhoArquivo = entry.path().string();
            processo.estado = "Pronto";
            processo.linhasArquivo = nLinhas(processo.caminhoArquivo);

            // Aloca a página de memória dinamicamente
            auto* pm = new PaginaMemoria();
            pm->pcb = processo;

            // Inicializa mutex e variável de condição
            pthread_mutex_init(&pm->mutex, nullptr);
            pthread_cond_init(&pm->cond, nullptr);

            // Cria a thread do processo
            if (pthread_create(&pm->th_processo, nullptr, executarProcesso, pm) != 0) {
                cout << "Erro ao criar a thread para o processo " << processo.id << endl;
                delete pm; // Libera memória se a thread não puder ser criada
                continue;
            }

            // Adiciona a página de memória à fila
            //filaPaginasMemoria.push_back(*pm);
            filaPaginasMemoria.push_back(pm);

            cout << "\nCarregado processo " << processo.id << " do arquivo: " << processo.caminhoArquivo << endl;

            sleep(3);
        }
    }
}
*/

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
    
    // pm->pcb.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50 (inclusive)
    pm->pcb.quantum = (rand() % 6) + 1;
}

void imprimirDados (PaginaMemoria *pm){
    cout << "\nDados finais: " << endl;
    cout << "Linhas Processadas: " << pm->pcb.linhasProcessadasAtual << endl;
    cout << "Timestamp: " << pm->pcb.timestamp << endl;
    cout << "Quantum final: " << pm->pcb.quantum;
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
    