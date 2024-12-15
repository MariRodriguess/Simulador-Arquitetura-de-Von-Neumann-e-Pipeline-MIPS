#include "cpu.hpp"

int PC = 0;
int CLOCK = 0;
bool perifericos[NUM_PERIFERICOS] = {false};
vector<int> principal;

int main() {
    string nomeArquivo = "data/input.data";

    CPU cpu(nomeArquivo); // Instancia a CPU
    cpu.inicializar();    // Inicializa registradores e componentes
    cpu.executar();       // Executa as instruções do arquivo
    cpu.exibirEstado();   // Exibe o estado final da CPU

    return 0;
}

