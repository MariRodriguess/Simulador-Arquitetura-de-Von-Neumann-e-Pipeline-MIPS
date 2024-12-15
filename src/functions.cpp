#include "unidadeControle.hpp"
#include "functions.hpp"

// Função para inicializar um processo com quantum e timestamp
Processo criarProcesso(int quantumInicial, int idProcesso)
{
    Processo p;
    p.quantum = quantumInicial;
    p.timestamp = CLOCK; // Timestamp inicial é o valor atual do CLOCK
    p.id = idProcesso;
    return p;
}

void LerInstrucoesDoArquivo(const string &nomeArquivo, int *registradores) {
    ifstream arquivo(nomeArquivo);
    string linha;

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo!" << endl;
        return;
    }

    while (getline(arquivo, linha)) {
        UnidadeControle(registradores, linha);
        //cout << "Clock: " << CLOCK << endl;
    }

    arquivo.close();
}

void LogSaida(const string &mensagem) {
    ofstream logFile("log_output.txt", ios::app); // Abre o arquivo no modo append
    time_t agora = time(0);                       // Pega o tempo atual
    char *dt = ctime(&agora);                     // Converte para string legível

    // Remove o '\n' ao final da string de tempo
    string tempoAtual(dt);
    tempoAtual.pop_back();

    // Mensagem formatada
    cout << "[" << tempoAtual << "] " << mensagem << endl; // Exibe no console
    if (logFile.is_open()) {
        logFile << "[" << tempoAtual << "] " << mensagem << endl; // Escreve no arquivo
        logFile.close();
    }
}
