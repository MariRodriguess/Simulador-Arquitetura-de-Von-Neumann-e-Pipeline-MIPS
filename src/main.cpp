#include "cpu.hpp"
#include "functions.hpp"

int PC = 0;
int CLOCK = 0;
bool perifericos[NUM_PERIFERICOS] = {false};
vector<int> principal;
queue<PCB> filaProcessos;
pthread_mutex_t filaLock = PTHREAD_MUTEX_INITIALIZER;

int main() {

    string diretorio = "data"; // Pasta contendo os arquivos .data

	// Carregar processos da pasta
    carregarProcessos(diretorio, filaProcessos);

    CPU cpu;
    //cpu.inicializar();    // Inicializa registradores e componentes
    cpu.executar();       // Executa as instruções do arquivo
    cpu.exibirEstado();   // Exibe o estado final da CPU

    return 0;
}