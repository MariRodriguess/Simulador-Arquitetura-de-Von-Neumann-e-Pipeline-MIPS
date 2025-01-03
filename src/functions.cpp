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
    tempoGasto = 0;

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo!" << endl;
        return;
    }

    while (getline(arquivo, linha)) {

        UnidadeControle(registradores, linha, linhaAtual, pm);

        pm->pcb.linhasProcessadas++;
        linhaAtual++;

        if(pm->pcb.linhasProcessadas == pm->pcb.linhasArquivo){

            cout << "Processo " << pm->pcb.id << " encerrado!\n";

            // Atualizar estado do processo
            pm->pcb.estado = "Finalizado";

            /*
            // Atualizar timestamps
            pthread_mutex_lock(&filaMutex);
            for (auto pagina : filaPaginasMemoria) {
                if (pagina->pcb.id != pm->pcb.id) { // Não atualiza o processo atual
                    pagina->pcb.timestamp += pm->pcb.quantum; // Incrementa o timestamp
                }
            }
            pthread_mutex_unlock(&filaMutex);
            */
            

            imprimirDados(pm);

            // Remover o processo finalizado da fila
            pthread_mutex_lock(&filaMutex);
            filaPaginasMemoria.erase(filaPaginasMemoria.begin());
            pthread_mutex_unlock(&filaMutex);
    
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
        pm = filaPaginasMemoria[indicePm];
        pthread_mutex_unlock(&filaMutex);
            

        if (pm != nullptr) {

            // Sinaliza que o processo pode começar
            pthread_mutex_lock(&pm->mutex);
            if (pm->pcb.estado != "Finalizado") {

                pm->pcb.estado = "Executando";
                cout << "\nCPU esta ativando o processo " << pm->pcb.id << endl;
                sleep(1);
                pthread_cond_signal(&pm->cond); // Libera o processo
                sleep(1);
                pthread_mutex_unlock(&pm->mutex);
                sleep(1);
            } else {
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

void* executarProcesso(void* arg) {

    sleep(2);
    auto* pm = static_cast<PaginaMemoria*>(arg);

    pthread_mutex_lock(&pm->mutex); // Bloqueia até a CPU sinalizar para iniciar a execução
    cout << "Processo " << pm->pcb.id << " aguardando liberacao da CPU..." << endl;

    // Espera até receber o sinal da CPU
    pthread_cond_wait(&pm->cond, &pm->mutex);

    // Após liberação
    cout << "Processo " << pm->pcb.id << " em execucao por " << pm->pcb.quantum << " unidades de tempo." << endl;

    // Execução
    LerInstrucoesDoArquivo(pm->pcb.caminhoArquivo, pm->pcb.registradores, pm);
    
    // Atualizar timestamps
    atualizarTimestamps(filaPaginasMemoria, tempoGasto, pm->pcb.id);

    /*
    // Simula a mudança de estado do processo
    pm->pcb.estado = "Finalizado";
    cout << "Processo " << pm->pcb.id << " finalizado." << endl;
    */

    pthread_mutex_unlock(&pm->mutex);
    return nullptr;
}

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
            processo.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50
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

            cout << "\nCarregado processo " << processo.id 
                 << " do arquivo: " << processo.caminhoArquivo << endl;

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
        logFile << "[" << tempoAtual << "] " << mensagem << endl; // Escreve no arquivo
        logFile.close();
    }
}

void recalcularQuantum (PaginaMemoria *pm){
    
    pm->pcb.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50 (inclusive)

}

void imprimirDados (PaginaMemoria *pm){
    cout << "Dados finais: " << endl;
    cout << "Linhas Processadas: " << pm->pcb.linhasProcessadas << endl;
    cout << "Timestamp: " << pm->pcb.timestamp << endl;
    cout << "Quantum final: " << pm->pcb.quantum << "\n\n";
}

// Atualizar timestamps
void atualizarTimestamps(std::vector<PaginaMemoria *> &filaPaginasMemoria, int quantumGasto, int idAtual) {
    pthread_mutex_lock(&filaMutex); // Protege a fila contra acesso simultâneo
    for (auto *pagina : filaPaginasMemoria) { // Itera sobre os ponteiros
        if (pagina->pcb.id != idAtual) { // Ignora o processo atual
            pagina->pcb.timestamp += quantumGasto; // Atualiza o timestamp
        }
    }
    pthread_mutex_unlock(&filaMutex);
}
    