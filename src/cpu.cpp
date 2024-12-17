#include "cpu.hpp"
#include "functions.hpp"

pthread_mutex_t lockPipeline = PTHREAD_MUTEX_INITIALIZER; // Inicializando o mutex

CPU::CPU() {
    cout << "Inicializando CPU..." << endl;

    for (int i = 0; i < NUM_NUCLEOS; i++) {
        int *reg = (int *)malloc(32 * sizeof(int)); // Registradores por núcleo
        for (int j = 0; j < 32; j++) {
            reg[j] = 0; // Inicializa registradores com zero
        }
        nucleos.push_back(reg);
        nucleosLivres.push_back(true); // Todos os núcleos começam livres
    }
}

CPU::~CPU() {
    
    free(registradores);

    nucleosLivres.clear();
    nucleosLivres.shrink_to_fit(); 

    nucleos.clear();
    nucleos.shrink_to_fit(); 
    
}

/*
void CPU::executar() {

    cout << "Iniciando execucao das instrucoes em " << NUM_NUCLEOS << " nucleos..." << endl;

    //vector<pthread_t> threads={NULL};  // Array de threads   
    vector<pthread_t> threads(NUM_NUCLEOS); // Array de threads 
  
    NucleoArgs args[NUM_NUCLEOS];   // Argumentos para os núcleos

    
    while (!filaProcessos.empty()){
        for (int i = 0; i < NUM_NUCLEOS; i++) {
            string nomeArquivo = filaProcessos.front().caminhoArquivo;
            filaProcessos.pop();
            args[i] = {nucleos[i], i, nomeArquivo}; // Define argumentos

            pthread_mutex_lock(&lockPipeline);  // Garante que apenas um processo acesse o pipeline
            pthread_create(&threads[i], nullptr, executarNucleo, &args[i]);
            pthread_mutex_unlock(&lockPipeline);  // Libera o acesso ao pipeline
        } 

    }

    cout << "Execucao finalizada." << endl;
}

void * executarNucleo(void *arg) {
    // Argumentos recebidos pela thread
    auto *dados = static_cast<NucleoArgs*>(arg);

    int *registradores = dados->registradores;
    int idNucleo = dados->idNucleo;
    string nomeArquivo = dados->nomeArquivo;

    cout << "Nucleo " << idNucleo << " iniciado." << endl;
    // Processa instruções para este núcleo
    LerInstrucoesDoArquivo(nomeArquivo, registradores);

    cout << "Nucleo " << idNucleo << " finalizado." << endl;
    return nullptr;
}
*/

void CPU::executar() {
    cout << "Iniciando exucacao das intrucoes em " << NUM_NUCLEOS << " nucleos..." << endl;

    vector<pthread_t> threads(NUM_NUCLEOS); // Array de threads

    while (!filaProcessos.empty()) {
        for (int i = 0; i < NUM_NUCLEOS && !filaProcessos.empty(); i++) {
            if (nucleosLivres[i]) { // Verifica se o núcleo está livre
                string nomeArquivo = filaProcessos.front().caminhoArquivo;
                filaProcessos.pop();

                nucleosLivres[i] = false; // Marca o núcleo como ocupado
                NucleoArgs *arg = new NucleoArgs{nucleos[i], i, nomeArquivo};
                cout << "Criando thread para nucleo " << i << " com arquivo " << nomeArquivo << endl;

                pthread_create(&threads[i], nullptr, executarNucleo, arg);
            }
        }

        // Aguarda conclusão das threads
        for (int i = 0; i < NUM_NUCLEOS; i++) {
            if (!nucleosLivres[i]) {
                pthread_join(threads[i], nullptr);
                nucleosLivres[i] = true; // Marca o núcleo como livre
            }
        }
    }

    cout << "Execucao finalizada." << endl;
}

void *executarNucleo(void *arg) {
    auto *dados = static_cast<NucleoArgs *>(arg);

    int *registradores = dados->registradores;
    int idNucleo = dados->idNucleo;
    string nomeArquivo = dados->nomeArquivo;

    cout << "Nucleo " << idNucleo << " iniciado com arquivo " << nomeArquivo << "." << endl;

    // Trava o mutex antes de acessar seções críticas
    pthread_mutex_lock(&lockPipeline);
    
    // Processa instruções para este núcleo
    LerInstrucoesDoArquivo(nomeArquivo, registradores);
    
    // Libera o mutex ao terminar as seções críticas
    pthread_mutex_unlock(&lockPipeline);

    cout << "Nucleo " << idNucleo << " finalizado." << endl;

    delete dados; // Libera memória alocada para os argumentos
    return nullptr;
}



void CPU::exibirEstado() {
    cout << "Estado Final da CPU:" << endl;
    for (int i = 0; i < NUM_NUCLEOS; i++) {
        cout << "Nucleo " << i << ": ";
        for (int j = 0; j < 32; j++) {
            cout << nucleos[i][j] << " ";
        }
        cout << endl;
    }
}
