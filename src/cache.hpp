#ifndef CACHE_HPP
#define CACHE_HPP

#include "include.hpp"

extern unordered_map<string, int> cacheInstrucoes;
extern deque<string> lruQueue;
extern const int CACHE_SIZE;

void adicionarAoCache(string hashInstrucao, int resultado);
bool verificarCache(string hashInstrucao);

#endif