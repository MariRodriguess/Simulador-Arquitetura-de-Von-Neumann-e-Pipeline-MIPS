#include "cache.hpp"

deque<string> lruQueue;

void adicionarAoCache(string hashInstrucao, int resultado) {
    if (cacheInstrucoes.find(hashInstrucao) == cacheInstrucoes.end()) {
        if ((int)lruQueue.size() >= CACHE_SIZE) {
            string hashRemover = lruQueue.front();
            lruQueue.pop_front();
            cacheInstrucoes.erase(hashRemover);
        }
        lruQueue.push_back(hashInstrucao);
    }
    cacheInstrucoes[hashInstrucao] = resultado;
}

bool verificarCache(string hashInstrucao) {
    if (cacheInstrucoes.find(hashInstrucao) != cacheInstrucoes.end()) {
        return true;
    }
    return false;
}