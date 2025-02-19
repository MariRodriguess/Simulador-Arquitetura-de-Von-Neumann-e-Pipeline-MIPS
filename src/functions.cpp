#include "functions.hpp"
vector<PaginaMemoria*> filaPaginasMemoria; // Usando ponteiros
vector<PaginaVirtual> paginasVirtuais;
pthread_mutex_t filaMutex = PTHREAD_MUTEX_INITIALIZER; // Inicializando o mutex

struct Args {
    PaginaMemoria* pm;// Ponteiro para a fila
};

unordered_map<char, int> mapaInstrucoes = {{'=',2}, {'+', 5}, {'-', 5}, {'*', 5}, {'/', 5}, {'$',3}, {'?', 3}};

bool similaridade = false;


// ===== Execução Processos

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

double jaccardSimilarity(const unordered_set<string>& set1, const unordered_set<string>& set2) {
    unordered_set<string> intersectionSet;
    unordered_set<string> unionSet = set1;

    for (const auto& token : set2) {
        if (set1.count(token)) {
            intersectionSet.insert(token);
        }
        unionSet.insert(token);
    }

    if (unionSet.empty()) return 0.0;

    return static_cast<double>(intersectionSet.size()) / unionSet.size();
}

// Função para processar um arquivo e transformar suas linhas em um conjunto de tokens
unordered_set<string> processarArquivo(const string& caminho) {
    ifstream file(caminho);
    unordered_set<string> tokens;
    string line;

    if (!file.is_open()) {
        cerr << "Erro ao abrir arquivo: " << caminho << endl;
        return tokens;
    }

    while (getline(file, line)) {
        size_t pos = 0;
        while ((pos = line.find(' ')) != string::npos) {
            tokens.insert(line.substr(0, pos));
            line.erase(0, pos + 1);
        }
        if (!line.empty()) tokens.insert(line);
    }

    file.close();
    return tokens;
}

// Função para encontrar a melhor posição para inserir o processo na fila
size_t encontrarPosicaoInsercao(const unordered_set<string>& novoProcessoTokens) {
    double maxSimilaridade = -1.0;
    size_t melhorPosicao = filaPaginasMemoria.size(); // Se não for similar, insere no final

    for (size_t i = 0; i < filaPaginasMemoria.size(); ++i) {
        unordered_set<string> tokensExistente = processarArquivo(filaPaginasMemoria[i]->pcb.caminhoArquivo);
        double similaridade = jaccardSimilarity(novoProcessoTokens, tokensExistente);

        if (similaridade > maxSimilaridade) {
            maxSimilaridade = similaridade;
            melhorPosicao = i;
        }
    }

    return melhorPosicao;
}

void carregarProcessos(const string& diretorio) {
    namespace fs = filesystem;
    int idProcesso = 0;
    PaginaVirtual pagina;

    for (const auto& entry : fs::directory_iterator(diretorio)) {
        if (entry.is_regular_file() && entry.path().extension() == ".data") {
            PCB processo;
            processo.id = idProcesso++;
            processo.prioridade = rand() % 10;
            processo.quantum = (rand() % 6) + 1;
            // processo.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50
            processo.quantumOriginal = processo.quantum;
            processo.registradores = (int*)malloc(10 * sizeof(int));
            processo.caminhoArquivo = entry.path().string();
            processo.estado = "Pronto";
            auto resultado = nLinhas(processo.caminhoArquivo);
            processo.linhasArquivo = resultado.first;
            processo.quantumNecessario = resultado.second;
            processo.idCpuAtual = -1;
            processo.numBilhetes = 1;
            processo.recebeuRecurso = false;
            processo.timestamp = 0;

            // Inicializa o semáforo
            sem_init(&processo.semaforo, 0, 1);

            auto* pm = new PaginaMemoria();
            pm->pcb = processo;
            pm->base = filaPaginasMemoria.size();
            pm->limite = pm->base + 1;
            
            pthread_mutex_init(&pm->mutex, nullptr);
            pthread_cond_init(&pm->cond, nullptr);

            if (pthread_create(&pm->th_processo, nullptr, executarProcesso, pm) != 0) {
                cout << "Erro ao criar a thread para o processo " << processo.id << endl;
                delete pm;
                continue;
            }
            //Se similaridade, colocar na fila de acordo com o critério
            if(!similaridade){

                filaPaginasMemoria.push_back(pm);
                // cout << "Pm: " << pm->base << " e " << pm->limite << endl;

                if (SJF_map){
                    // Cria uma página virtual para o processo
                    pagina.enderecoVirtual = transfDecimalBinario(processo.id); // Endereço virtual (ID em binário)
                    pagina.quantumNecessario = processo.quantumNecessario;     // Quantum necessário
                    paginasVirtuais.push_back(pagina);
                }
            }
            else{
                // Obtém tokens do novo processo
                unordered_set<string> novoProcessoTokens = processarArquivo(processo.caminhoArquivo);

                // Encontra a posição ideal na fila com base na similaridade
                size_t posicao = encontrarPosicaoInsercao(novoProcessoTokens);
                filaPaginasMemoria.insert(filaPaginasMemoria.begin() + posicao, pm);
            }
            if (!SJF_map){
                cout << "\nCarregando processo " << processo.id + 1 << " do arquivo: " << processo.caminhoArquivo << endl;
            }else{
                cout << "Processo " << processo.id+1 << " carregado. Endereco virtual: " << pagina.enderecoVirtual << endl;
            }

            sleep(2);
        }
    }
}

void LerInstrucoesDoArquivo(const string &nomeArquivo, int *registradores, PaginaMemoria *pm) {

    ifstream arquivo(nomeArquivo);
    string linha;
    int linhaAtual = 1;
    tempoGasto[pm->pcb.idCpuAtual - 1] = CLOCK[pm->pcb.idCpuAtual - 1];

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo!" << endl;
        return;
    }

    while (getline(arquivo, linha)) {

        UnidadeControle(registradores, linha, linhaAtual, pm);

        linhaAtual++;
        if(pm->pcb.estado == "Bloqueado"){
            
            // Atualizar timestamps
            atualizarTimestamps(filaPaginasMemoria, tempoGasto[pm->pcb.idCpuAtual - 1],pm);
            sleep(1);
            pm->pcb.linhasProcessadasAnt = pm->pcb.linhasProcessadasAtual;
            pm->pcb.linhasProcessadasAtual = 0;
            pm->pcb.estado = "Bloqueado";

            cout << "\nProcesso " << pm->pcb.id << " bloqueado! - Periférico em uso\n";
            LogSaida("\nProcesso " + to_string(pm->pcb.id) + " bloqueado! - Periférico em uso\n");
            break;
        }

        else if(pm->pcb.linhasProcessadasAtual == pm->pcb.linhasArquivo){

            // Atualizar estado do processo
            pm->pcb.estado = "Finalizado";
            
            // Atualizar timestamps
            atualizarTimestamps(filaPaginasMemoria, tempoGasto[pm->pcb.idCpuAtual - 1], pm);
            sleep(1);

            pthread_mutex_lock(&filaMutex);
            imprimirDados(pm);
            pthread_mutex_unlock(&filaMutex);
            sleep(0.5);

            //Liberar recursos associados
            for(int i = 0; i < (int)pm->pcb.recursos.size(); i++){
                perifericos[pm->pcb.recursos[i]] = true;
                pm->pcb.recursos.erase(pm->pcb.recursos.begin() + i);
            }
            
            // Remover o processo finalizado da fila
            pthread_mutex_lock(&filaMutex);
            
            if(Loteria || FCFS){
                
                // Localizar o processo na fila
                auto it = find(filaPaginasMemoria.begin(), filaPaginasMemoria.end(), pm);

                if (it != filaPaginasMemoria.end()) {
                    filaPaginasMemoria.erase(it); // Remove o processo
                }        
            }
            /*
            else if((!SJF) && (!RoundRobin)){
                    filaPaginasMemoria.erase(filaPaginasMemoria.begin());
                }
            }
            */

            cout << "Processo " << pm->pcb.id + 1<< " encerrado!" << endl;
            pthread_mutex_unlock(&filaMutex);

        }
        else if (pm->pcb.quantum == 0){

            pm->pcb.linhasProcessadasAnt = pm->pcb.linhasProcessadasAtual;
            pm->pcb.linhasProcessadasAtual = 0;
            pm->pcb.estado = "Pronto";

            //Liberar recursos associados
            for(int i = 0; i < (int)pm->pcb.recursos.size(); i++){
                perifericos[pm->pcb.recursos[i]] = true;
                pm->pcb.recursos.erase(pm->pcb.recursos.begin() + i);
            }

            cout << "\nQuantum insuficiente!";
            LogSaida("Processo " + to_string(pm->pcb.id+1) + " -> Quantum esgotado!");
            
            // Atualizar timestamps
            atualizarTimestamps(filaPaginasMemoria, tempoGasto[pm->pcb.idCpuAtual - 1], pm);
            sleep(1);

            break;
        }  
    }

    arquivo.close();
}

void* executarProcesso(void* arg) {

    int auxCPU=0;

    sleep(2);

    auto* pm = static_cast<PaginaMemoria*>(arg);

    while (true) {

        pthread_mutex_lock(&pm->mutex); // Bloqueia até a CPU sinalizar para iniciar a execução

        // Sai do loop se o estado for "Finalizado"
        if (pm->pcb.estado == "Finalizado") {
            pthread_mutex_unlock(&pm->mutex);
            pm->pcb.idCpuAtual = -1;
            contProcessos[auxCPU] -= 1;
            break; // Finaliza a thread
        }

        // Aguarda liberação da CPU
        // cout << "Processo " << pm->pcb.id + 1 << " aguardando liberacao da CPU..." << endl;

        pthread_cond_wait(&pm->cond, &pm->mutex);

        // Coloca a thread em execução
        pm->pcb.estado = "Executando";
        pthread_mutex_unlock(&pm->mutex);
        // cout << "Processo " << pm->pcb.id + 1 << " indo rodar com a CPU " << pm->pcb.idCpuAtual << endl;
        sleep(0.5);
        LerInstrucoesDoArquivo(pm->pcb.caminhoArquivo, pm->pcb.registradores, pm);
        pthread_mutex_lock(&pm->mutex);

        auxCPU = pm->pcb.idCpuAtual-1;
        
        if (pm->pcb.estado == "Pronto"){

            recalcularQuantum(pm);
            pm->pcb.quantumOriginal = pm->pcb.quantum;
            if(Loteria){
                incrementarBilhetesNaoAtendidos(filaPaginasMemoria); // Aumentar as chances para processos não atendidos a cada execução
            }
            if(RoundRobin){
                pm->pcb.idCpuAtual = -1;
                filaPaginasMemoria.push_back(pm);
            }else{
                pm->pcb.idCpuAtual = -1;
            }
            contProcessos[auxCPU] -= 1;
            sleep(0.6);   
        }
        else if(pm->pcb.estado == "Bloqueado"){

            cout << "Processo bloqueado após acesso de periferico ocupado.\n";
            recalcularQuantum(pm);
            pm->pcb.quantumOriginal = pm->pcb.quantum;
            if(Loteria){
                incrementarBilhetesNaoAtendidos(filaPaginasMemoria); // Aumentar as chances para processos não atendidos a cada execução
            }
            if(SJF || RoundRobin){
                pm->pcb.idCpuAtual = -1;
                filaPaginasMemoria.push_back(pm);
                if (!RoundRobin){
                    ordenarFila_SJF();
                }
            }else{
                pm->pcb.idCpuAtual = -1;
                sleep(0.6);
            }  
            contProcessos[auxCPU] -= 1;
        }
        else if (pm->pcb.estado == "Finalizado"){

            if(Loteria){
                incrementarBilhetesNaoAtendidos(filaPaginasMemoria); // Aumentar as chances para processos não atendidos a cada execução
            }
        }
        
        pthread_mutex_unlock(&pm->mutex);
    }
    return nullptr;
}

void atualizarTimestamps(vector<PaginaMemoria *> &filaPaginasMemoria, int quantumGasto, PaginaMemoria * pm) {


    pthread_mutex_lock(&filaMutex); // Protege a fila contra acesso simultâneo

    for (auto *pagina : filaPaginasMemoria) { 
        if((pm->pcb.idCpuAtual == pagina->pcb.idCpuAtual) || (pagina->pcb.idCpuAtual == -1)){
            pagina->pcb.timestamp += (CLOCK[pm->pcb.idCpuAtual-1] - quantumGasto); // Atualiza o timestamp
        }

        //cout << "\nProcessoAtual: " << pm->pcb.id+1 << " - IdCpuAtual: " << pm->pcb.idCpuAtual << " - Adicionando: " << (CLOCK[pm->pcb.idCpuAtual-1] - quantumGasto) << "(" << CLOCK[pm->pcb.idCpuAtual-1] << "-" << quantumGasto <<  ") - " <<
        //" - ProcessoAtualizando: " << pagina->pcb.id+1 << " - IdCpuAtualizando: " << pagina->pcb.idCpuAtual << " - LinhasProcessadas " << pm->pcb.linhasProcessadasAtual;

    }
    if(SJF || RoundRobin){
        pm->pcb.timestamp += (CLOCK[pm->pcb.idCpuAtual-1] - quantumGasto);
    }

    pthread_mutex_unlock(&filaMutex);
}

void recalcularQuantum (PaginaMemoria *pm){
    
    sleep(0.5);
    pm->pcb.quantum = (rand() % 31) + 20;  // Gera um número entre 20 e 50 (inclusive)
    //pm->pcb.quantum = (rand() % 6) + 1;
}

// ===== Escalonadores

void* executarCpu_FCFS(void* arg) {

    CPU* cpu = static_cast<CPU*>(arg); // CPU específica

    cout << "\nCPU " << cpu->id << " inicializada.\n";

    while (true) {

        PaginaMemoria* pm = nullptr;

        // Tenta pegar um processo da fila
        pthread_mutex_lock(&filaMutex);

        if (filaPaginasMemoria.empty()) {
            pthread_mutex_unlock(&filaMutex);
            break; // Sai do loop se a fila estiver vazia
        }

        // Verifica se a CPU está livre
        if (!cpu->ocupada) {
            for (auto pagina : filaPaginasMemoria) { 
                //cout << "\nCpu " << cpu->id << " analisando Processo " << pagina->pcb.id+1 << " com IdCPU " << pagina->pcb.idCpuAtual << " e Quantum " << pagina->pcb.quantum;
                if (pagina->pcb.idCpuAtual == -1) {
                    pm = pagina;
                    pm->pcb.idCpuAtual = cpu->id;
                    cpu->ocupada = true; // Marca a CPU como ocupada
                    pm->pcb.quantum = 2147483647; // Para que o processo não sofra preempção
                    pm->pcb.quantum = 100;
                    break;
                }
            }
        }

        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == cpu->id){
                
                //cout << "Cpu " << cpu->id << " pegando Processo " << pm->pcb.id+1 << "." << endl;
                
                // Executa o processo
                sleep(0.6);
                pthread_mutex_lock(&pm->mutex);
                sleep(0.6);
                pthread_cond_signal(&pm->cond); // Libera o processo para execução
                sleep(0.6);
                pthread_mutex_unlock(&pm->mutex);

                while(pm->pcb.idCpuAtual == cpu->id){
                    sleep(0.1);
                }
                sleep(0.1);

                pthread_mutex_lock(&filaMutex);
                cpu->ocupada = false; // Libera a CPU
                // cout << "Cpu " << cpu->id << " liberada." << endl;
                pthread_mutex_unlock(&filaMutex);
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

        if (!cpu->ocupada) {

            if (filaPaginasMemoria.size() == 1){
                pm = filaPaginasMemoria.front();
                pm->pcb.idCpuAtual = cpu->id;
                pm->pcb.quantum = 2147483647; // Para que o processo não sofra preempção por quantum
                pm->pcb.recebeuRecurso = true;
            }
            else{
                // Calcular o total de bilhetes
                int totalBilhetes = 0;
                for (auto pagina : filaPaginasMemoria) { 
                    totalBilhetes += pagina->pcb.numBilhetes;
                }   

                // Sortear um bilhete
                int bilheteExecucao = rand() % totalBilhetes; // Gera um número entre 0 e totalBilhetes-1.
                // cout << "\nBilhete: " << bilheteExecucao << endl;
                int aux = 0;
                for (auto& pagina : filaPaginasMemoria) {
                    aux += pagina->pcb.numBilhetes;
                    if ((aux > bilheteExecucao) && (pagina->pcb.idCpuAtual == -1)) {
                        pm = pagina;
                        pm->pcb.idCpuAtual = cpu->id;
                        pm->pcb.quantum = 2147483647; // Para que o processo não sofra preempção por quantum
                        pm->pcb.recebeuRecurso = true;
                        break;
                        
                    }
                }
            }
        }

        pthread_mutex_unlock(&filaMutex);

        // Processo 1 com 3 bilhetes → [0, 1, 2]
        // Processo 2 com 5 bilhetes → [3, 4, 5, 6, 7]
        // Processo 3 com 2 bilhetes → [8, 9] etc
        //
        // Se bilheteExecucao=4, pertence ao processo 2 [3, 4, 5, 6, 7] porque 4 < 8.
        // Se bilheteExecucao=9, pertence ao processo 3 [8, 9] porque 9 < 10.
       
        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == cpu->id){
                
                //cout << "Cpu " << cpu->id << " pegando Processo " << pm->pcb.id+1 << "." << endl;

                // Executa o processo
                sleep(0.6);
                pthread_mutex_lock(&pm->mutex);
                sleep(0.6);
                pthread_cond_signal(&pm->cond); // Libera o processo para execução
                pthread_mutex_unlock(&pm->mutex);

                while(pm->pcb.idCpuAtual == cpu->id){
                    sleep(0.1);
                }
                sleep(0.1);

                pthread_mutex_lock(&filaMutex);
                cpu->ocupada = false; // Libera a CPU
                // cout << "Cpu " << cpu->id << " liberada." << endl;
                pthread_mutex_unlock(&filaMutex);
            }
        } else {
            sleep(1); // Espera por novos processos
        }
        
    }
    return nullptr;
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

        // Verifica se a CPU está livre
        if (!cpu->ocupada) {
            pm = filaPaginasMemoria.front(); 
            filaPaginasMemoria.erase(filaPaginasMemoria.begin());
            pm->pcb.quantum = 2147483647; // Para que o processo não sofra preempção
            pm->pcb.idCpuAtual = cpu->id;
        }

        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == cpu->id){
                
                //cout << "Cpu " << cpu->id << " pegando Processo " << pm->pcb.id+1 << "." << endl;
                
                // Executa o processo
                sleep(0.6);
                pthread_mutex_lock(&pm->mutex);
                sleep(0.6);
                pthread_cond_signal(&pm->cond); // Libera o processo para execução
                pthread_mutex_unlock(&pm->mutex);

                while(pm->pcb.idCpuAtual == cpu->id){
                    sleep(0.1);
                }
                sleep(0.1);

                pthread_mutex_lock(&filaMutex);
                cpu->ocupada = false; // Libera a CPU
                // cout << "Cpu " << cpu->id << " liberada." << endl;
                pthread_mutex_unlock(&filaMutex);
            }
        } else {
            sleep(1); // Espera por novos processos
        }
        
    }
    return nullptr;
}

void* executarCpu_RoundRobin(void* arg) {
    
    CPU* cpu = static_cast<CPU*>(arg); // CPU específica

    while (true) {
        PaginaMemoria* pm = nullptr;

        // Tenta pegar um processo da fila
        pthread_mutex_lock(&filaMutex);

        if (filaPaginasMemoria.empty()) {
            pthread_mutex_unlock(&filaMutex);
            break; // Sai do loop se a fila estiver vazia
        }

        // Verifica se a CPU está livre
        if (!cpu->ocupada) {
            for (auto it = filaPaginasMemoria.begin(); it != filaPaginasMemoria.end(); ++it) {
                if ((*it)->pcb.idCpuAtual == -1) {
                    pm = *it;
                    pm->pcb.idCpuAtual = cpu->id;
                    pm->pcb.quantum = 40;
                    cpu->ocupada = true; // Marca a CPU como ocupada
                    filaPaginasMemoria.erase(it); // Remove da fila circular
                    break;
                }
            }
        }

        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == cpu->id){
                
                //cout << "Cpu " << cpu->id << " pegando Processo " << pm->pcb.id+1 << "." << endl;

                // Executa o processo
                contProcessos[pm->pcb.idCpuAtual - 1] += 1;

                sleep(0.6);
                pthread_mutex_lock(&pm->mutex);
                sleep(0.6);
                pthread_cond_signal(&pm->cond); // Libera o processo para execução
                pthread_mutex_unlock(&pm->mutex);

                while((pm->pcb.idCpuAtual == cpu->id) || (contProcessos[cpu->id-1] != 0)){
                    sleep(0.1);
                }
                sleep(0.1);

                pthread_mutex_lock(&filaMutex);
                cpu->ocupada = false; // Libera a CPU
                // cout << "Cpu " << cpu->id << " liberada." << endl;
                pthread_mutex_unlock(&filaMutex);
            }
        } else {
            sleep(1); // Espera por novos processos
        }
    }
    return nullptr;
}

bool compararQuantum(const PaginaVirtual& a, const PaginaVirtual& b) {
    return a.quantumNecessario < b.quantumNecessario;
}

void ordenarPaginasVirtuais() {
    sort(paginasVirtuais.begin(), paginasVirtuais.end(), compararQuantum);
}

void* executarCpu_SJF_mapeamento(void* arg) {

    CPU* cpu = static_cast<CPU*>(arg);

    ordenarPaginasVirtuais(); // Ordena as páginas virtuais pelo quantum

    while (true) {

        PaginaMemoria* pm = nullptr;

        pthread_mutex_lock(&filaMutex);

        if (paginasVirtuais.empty()) {
            pthread_mutex_unlock(&filaMutex);
            break; // Sai do loop se não houver páginas virtuais
        }

        if (!cpu->ocupada) {

            // Pega a primeira página virtual da lista ordenada
            PaginaVirtual pagina = paginasVirtuais.front();
            paginasVirtuais.erase(paginasVirtuais.begin());

            // Converte o endereço virtual (binário) de volta para decimal
            int idProcesso = transBinarioDecimal(pagina.enderecoVirtual);

            // Procura o processo na filaPaginasMemoria usando o ID convertido
            for (auto it = filaPaginasMemoria.begin(); it != filaPaginasMemoria.end(); ++it) {
                if ((*it)->pcb.id == idProcesso) {
                    pm = *it;
                    filaPaginasMemoria.erase(it);
                    break;
                }
            }

            if (pm) {
                pm->pcb.quantum = 2147483647; // Para que o processo não sofra preempção
                pm->pcb.idCpuAtual = cpu->id;
            }
        }

        pthread_mutex_unlock(&filaMutex);

        if (pm != nullptr) {
            if ((pm->pcb.estado == "Pronto" || pm->pcb.estado == "Bloqueado") && pm->pcb.idCpuAtual == cpu->id) {
                sleep(0.6);
                pthread_mutex_lock(&pm->mutex);
                sleep(0.6);
                pthread_cond_signal(&pm->cond);
                pthread_mutex_unlock(&pm->mutex);

                while (pm->pcb.idCpuAtual == cpu->id) {
                    sleep(0.1);
                }
                sleep(0.1);

                pthread_mutex_lock(&filaMutex);
                cpu->ocupada = false;
                pthread_mutex_unlock(&filaMutex);
            }
        } else {
            sleep(1); // Espera por novos processos
        }
    }
    return nullptr;
}

// ===== Funções auxiliares

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

void imprimirDados (PaginaMemoria *pm){
    if (FCFS || SJF || SJF_map){
        LogSaida("\nProcesso " + to_string(pm->pcb.id + 1) + " encerrado!" + 
                "\nDados finais: \nLinhas Processadas: " + to_string(pm->pcb.linhasProcessadasAtual) +  
                "\nPrioridade: " + to_string(pm->pcb.prioridade) + 
                "\nQuantum necessário para executar: " + to_string(pm->pcb.quantumNecessario) +
                "\nTimestamp: " + to_string(pm->pcb.timestamp) + 
                "\nEstado: " + pm->pcb.estado + "\n");
    }
    else if (Loteria){
        LogSaida("\nProcesso " + to_string(pm->pcb.id + 1) + " encerrado!" + 
                "\nDados finais: \nLinhas Processadas: " + to_string(pm->pcb.linhasProcessadasAtual) +  
                "\nPrioridade: " + to_string(pm->pcb.prioridade) + 
                "\nQuantum necessário para executar: " + to_string(pm->pcb.quantumNecessario) +
                "\nTimestamp: " + to_string(pm->pcb.timestamp) + 
                "\nNúmeros de bilhetes acumulados: " + to_string(pm->pcb.numBilhetes) + 
                "\nEstado: " + pm->pcb.estado +"\n");
    }else{
        LogSaida("\nProcesso " + to_string(pm->pcb.id + 1) + " encerrado!" + 
                "\nDados finais: \nLinhas Processadas: " + to_string(pm->pcb.linhasProcessadasAtual) +  
                "\nPrioridade: " + to_string(pm->pcb.prioridade) + 
                "\nQuantum necessário para executar: " + to_string(pm->pcb.quantumNecessario) +
                "\nTimestamp: " + to_string(pm->pcb.timestamp) + 
                "\nQuantum final: " + to_string(pm->pcb.quantum) + 
                "\nEstado: " + pm->pcb.estado +"\n");
    }
    
}