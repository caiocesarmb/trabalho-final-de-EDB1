#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>
#include <iomanip>

using namespace std;

// HASH usado em todas as estratégias
size_t calcularHash(int chave, size_t tamanhoTabela) {
    const unsigned long long a = 2654435761ULL;
    const unsigned long long b = 40503ULL;
    const unsigned long long p = 4294967291ULL;
    
    unsigned long long valorHash = ((a * static_cast<unsigned long long>(chave) + b) % p);
    return valorHash % tamanhoTabela;
}


// Estrutura para armazenar chave-valor
struct Elemento {
    int chave;
    int valor;
    Elemento(int c, int v) : chave(c), valor(v) {}
};

// Tabela HASH - ESTÁTICA
class TabelaEstatica {
private:
    vector<list<Elemento>> tabela;
    size_t capacidade;
    size_t elementos;
    size_t colisoes;
    
public:
    TabelaEstatica(size_t tamanhoFixo = 10007) : capacidade(tamanhoFixo), elementos(0), colisoes(0) {
        tabela.resize(tamanhoFixo);
    }
    
    void inserir(int chave, int valor) {
        size_t indice = calcularHash(chave, capacidade);
        
        for (auto& elem : tabela[indice]) {
            if (elem.chave == chave) {
                elem.valor = valor;
                return;
            }
        }
        
        if (!tabela[indice].empty()) {
            colisoes++;
        }
        
        tabela[indice].emplace_back(chave, valor);
        elementos++;
    }
    
    bool buscar(int chave, int& valorEncontrado) {
        size_t indice = calcularHash(chave, capacidade);
        
        for (const auto& elem : tabela[indice]) {
            if (elem.chave == chave) {
                valorEncontrado = elem.valor;
                return true;
            }
        }
        return false;
    }
    
    bool remover(int chave) {
        size_t indice = calcularHash(chave, capacidade);
        
        auto it = find_if(tabela[indice].begin(), tabela[indice].end(),
                         [chave](const Elemento& e) { return e.chave == chave; });
        
        if (it != tabela[indice].end()) {
            tabela[indice].erase(it);
            elementos--;
            return true;
        }
        return false;
    }
    
    size_t obterColisoes() const { return colisoes; }
    size_t obterCapacidade() const { return capacidade; }
    size_t obterElementos() const { return elementos; }
    double obterFatorCarga() const { 
        return capacidade > 0 ? (double)elementos / capacidade : 0.0; 
    }
};

// Tabela hash - REDIMENSIONAMENTO POR DUPLICAÇÃO
class TabelaDuplicacao {
private:
    vector<list<Elemento>> tabela;
    size_t capacidade;
    size_t elementos;
    size_t colisoes;
    size_t redimensionamentos;
    double cargaMaxima;
    double cargaMinima;
    
    void redimensionar(size_t novaCapacidade) {
        vector<list<Elemento>> tabelaAntiga = tabela;
        
        capacidade = novaCapacidade;
        tabela.clear();
        tabela.resize(capacidade);
        elementos = 0;
        colisoes = 0;
        
        for (const auto& bucket : tabelaAntiga) {
            for (const auto& elem : bucket) {
                size_t novoIndice = calcularHash(elem.chave, capacidade);
                
                if (!tabela[novoIndice].empty()) {
                    colisoes++;
                }
                
                tabela[novoIndice].emplace_back(elem.chave, elem.valor);
                elementos++;
            }
        }
        
        redimensionamentos++;
    }
    
public:
    TabelaDuplicacao(size_t capacidadeInicial = 8, double maxCarga = 0.7, double minCarga = 0.2)
        : capacidade(capacidadeInicial), elementos(0), colisoes(0), 
          redimensionamentos(0), cargaMaxima(maxCarga), cargaMinima(minCarga) {
        tabela.resize(capacidadeInicial);
    }
    
    void inserir(int chave, int valor) {
        if (obterFatorCarga() >= cargaMaxima) {
            redimensionar(capacidade * 2);
        }
        
        size_t indice = calcularHash(chave, capacidade);
        
        for (auto& elem : tabela[indice]) {
            if (elem.chave == chave) {
                elem.valor = valor;
                return;
            }
        }
        
        if (!tabela[indice].empty()) {
            colisoes++;
        }
        
        tabela[indice].emplace_back(chave, valor);
        elementos++;
        
        if (capacidade > 8 && obterFatorCarga() <= cargaMinima) {
            redimensionar(capacidade / 2);
        }
    }
    
    bool buscar(int chave, int& valorEncontrado) {
        size_t indice = calcularHash(chave, capacidade);
        
        for (const auto& elem : tabela[indice]) {
            if (elem.chave == chave) {
                valorEncontrado = elem.valor;
                return true;
            }
        }
        return false;
    }
    
    bool remover(int chave) {
        size_t indice = calcularHash(chave, capacidade);
        
        auto it = find_if(tabela[indice].begin(), tabela[indice].end(),
                         [chave](const Elemento& e) { return e.chave == chave; });
        
        if (it != tabela[indice].end()) {
            tabela[indice].erase(it);
            elementos--;
            
            if (capacidade > 8 && obterFatorCarga() <= cargaMinima) {
                redimensionar(capacidade / 2);
            }
            
            return true;
        }
        return false;
    }
    
    size_t obterColisoes() const { return colisoes; }
    size_t obterCapacidade() const { return capacidade; }
    size_t obterElementos() const { return elementos; }
    size_t obterRedimensionamentos() const { return redimensionamentos; }
    double obterFatorCarga() const { 
        return capacidade > 0 ? (double)elementos / capacidade : 0.0; 
    }
};

// Tabela HASH - REDIMENSIONAMENTO INCREMENTAL
class TabelaIncremental {
private:
    vector<list<Elemento>> tabelaAtual;
    vector<list<Elemento>> tabelaNova;
    size_t capacidade;
    size_t elementos;
    size_t colisoes;
    size_t redimensionamentos;
    double cargaMaxima;
    size_t elementosPorPasso;
    size_t indiceAtual;
    bool emRedimensionamento;
    
public:
    TabelaIncremental(size_t capacidadeInicial = 8, double maxCarga = 0.7, size_t passo = 5)
        : capacidade(capacidadeInicial), elementos(0), colisoes(0), 
          redimensionamentos(0), cargaMaxima(maxCarga), elementosPorPasso(passo),
          indiceAtual(0), emRedimensionamento(false) {
        tabelaAtual.resize(capacidadeInicial);
    }
    
    void passoRedimensionamento() {
        if (!emRedimensionamento || indiceAtual >= tabelaAtual.size()) return;
        
        size_t movidos = 0;
        while (indiceAtual < tabelaAtual.size() && movidos < elementosPorPasso) {
            auto& bucket = tabelaAtual[indiceAtual];
            while (!bucket.empty() && movidos < elementosPorPasso) {
                auto& elem = bucket.front();
                size_t novoIndice = calcularHash(elem.chave, tabelaNova.size());
                
                tabelaNova[novoIndice].emplace_back(elem.chave, elem.valor);
                bucket.pop_front();
                movidos++;
            }
            
            if (bucket.empty()) {
                indiceAtual++;
            }
        }
        
        if (indiceAtual >= tabelaAtual.size()) {
            tabelaAtual = move(tabelaNova);
            capacidade = tabelaAtual.size();
            tabelaNova.clear();
            emRedimensionamento = false;
            redimensionamentos++;
        }
    }
    
    void inserir(int chave, int valor) {
        if (emRedimensionamento) {
            passoRedimensionamento();
        }
        
        if (!emRedimensionamento && obterFatorCarga() >= cargaMaxima) {
            tabelaNova.resize(capacidade * 2);
            emRedimensionamento = true;
            indiceAtual = 0;
            passoRedimensionamento();
        }
        
        if (emRedimensionamento) {
            size_t indice = calcularHash(chave, tabelaNova.size());
            
            for (auto& elem : tabelaNova[indice]) {
                if (elem.chave == chave) {
                    elem.valor = valor;
                    return;
                }
            }
            
            if (!tabelaNova[indice].empty()) {
                colisoes++;
            }
            
            tabelaNova[indice].emplace_back(chave, valor);
        } else {
            size_t indice = calcularHash(chave, capacidade);
            
            for (auto& elem : tabelaAtual[indice]) {
                if (elem.chave == chave) {
                    elem.valor = valor;
                    return;
                }
            }
            
            if (!tabelaAtual[indice].empty()) {
                colisoes++;
            }
            
            tabelaAtual[indice].emplace_back(chave, valor);
        }
        
        elementos++;
    }
    
    bool buscar(int chave, int& valorEncontrado) {
        if (emRedimensionamento) {
            size_t novoIndice = calcularHash(chave, tabelaNova.size());
            for (const auto& elem : tabelaNova[novoIndice]) {
                if (elem.chave == chave) {
                    valorEncontrado = elem.valor;
                    return true;
                }
            }
            
            size_t velhoIndice = calcularHash(chave, tabelaAtual.size());
            for (const auto& elem : tabelaAtual[velhoIndice]) {
                if (elem.chave == chave) {
                    valorEncontrado = elem.valor;
                    return true;
                }
            }
        } else {
            size_t indice = calcularHash(chave, capacidade);
            for (const auto& elem : tabelaAtual[indice]) {
                if (elem.chave == chave) {
                    valorEncontrado = elem.valor;
                    return true;
                }
            }
        }
        return false;
    }
    
    bool remover(int chave) {
        if (emRedimensionamento) {
            size_t novoIndice = calcularHash(chave, tabelaNova.size());
            auto it = find_if(tabelaNova[novoIndice].begin(), tabelaNova[novoIndice].end(),
                             [chave](const Elemento& e) { return e.chave == chave; });
            
            if (it != tabelaNova[novoIndice].end()) {
                tabelaNova[novoIndice].erase(it);
                elementos--;
                return true;
            }
            
            size_t velhoIndice = calcularHash(chave, tabelaAtual.size());
            it = find_if(tabelaAtual[velhoIndice].begin(), tabelaAtual[velhoIndice].end(),
                        [chave](const Elemento& e) { return e.chave == chave; });
            
            if (it != tabelaAtual[velhoIndice].end()) {
                tabelaAtual[velhoIndice].erase(it);
                elementos--;
                return true;
            }
        } else {
            size_t indice = calcularHash(chave, capacidade);
            auto it = find_if(tabelaAtual[indice].begin(), tabelaAtual[indice].end(),
                             [chave](const Elemento& e) { return e.chave == chave; });
            
            if (it != tabelaAtual[indice].end()) {
                tabelaAtual[indice].erase(it);
                elementos--;
                return true;
            }
        }
        return false;
    }
    
    size_t obterColisoes() const { return colisoes; }
    size_t obterCapacidade() const { return capacidade; }
    size_t obterElementos() const { return elementos; }
    size_t obterRedimensionamentos() const { return redimensionamentos; }
    double obterFatorCarga() const { 
        return capacidade > 0 ? (double)elementos / capacidade : 0.0; 
    }
};

// Tabela HASH - REDIMENSIONAMENTO ADAPTATIVO
class TabelaAdaptativa {
private:
    vector<list<Elemento>> tabela;
    size_t capacidade;
    size_t elementos;
    size_t colisoes;
    size_t redimensionamentos;
    double cargaMaxima;
    double cargaMinima;
    size_t limiteColisoes;
    size_t colisoesConsecutivas;
    
    void redimensionar(size_t novaCapacidade) {
        vector<list<Elemento>> tabelaAntiga = tabela;
        
        capacidade = novaCapacidade;
        tabela.clear();
        tabela.resize(capacidade);
        elementos = 0;
        colisoes = 0;
        colisoesConsecutivas = 0;
        
        for (const auto& bucket : tabelaAntiga) {
            for (const auto& elem : bucket) {
                size_t novoIndice = calcularHash(elem.chave, capacidade);
                
                if (!tabela[novoIndice].empty()) {
                    colisoes++;
                }
                
                tabela[novoIndice].emplace_back(elem.chave, elem.valor);
                elementos++;
            }
        }
        
        redimensionamentos++;
        
        if (colisoes > limiteColisoes * 2) {
            cargaMaxima = max(0.5, cargaMaxima - 0.05);
            limiteColisoes = max((size_t)10, limiteColisoes / 2);
        } else if (colisoes < limiteColisoes / 2) {
            cargaMaxima = min(0.9, cargaMaxima + 0.05);
            limiteColisoes *= 2;
        }
    }
    
public:
    TabelaAdaptativa(size_t capacidadeInicial = 8, double maxCarga = 0.7, double minCarga = 0.2)
        : capacidade(capacidadeInicial), elementos(0), colisoes(0), 
          redimensionamentos(0), cargaMaxima(maxCarga), cargaMinima(minCarga),
          limiteColisoes(100), colisoesConsecutivas(0) {
        tabela.resize(capacidadeInicial);
    }
    
    void inserir(int chave, int valor) {
        bool precisaRedimensionar = false;
        
        if (obterFatorCarga() >= cargaMaxima) {
            precisaRedimensionar = true;
        } else if (colisoes > limiteColisoes) {
            colisoesConsecutivas++;
            if (colisoesConsecutivas >= 3) {
                precisaRedimensionar = true;
                colisoesConsecutivas = 0;
            }
        } else {
            colisoesConsecutivas = 0;
        }
        
        if (precisaRedimensionar) {
            size_t novaCapacidade = capacidade * 2;
            if (colisoes > limiteColisoes * 3) {
                novaCapacidade = capacidade * 3;
            }
            redimensionar(novaCapacidade);
        }
        
        size_t indice = calcularHash(chave, capacidade);
        
        for (auto& elem : tabela[indice]) {
            if (elem.chave == chave) {
                elem.valor = valor;
                return;
            }
        }
        
        if (!tabela[indice].empty()) {
            colisoes++;
        }
        
        tabela[indice].emplace_back(chave, valor);
        elementos++;
        
        if (capacidade > 16 && obterFatorCarga() <= cargaMinima && 
            colisoes < limiteColisoes / 4) {
            redimensionar(capacidade / 2);
        }
    }
    
    bool buscar(int chave, int& valorEncontrado) {
        size_t indice = calcularHash(chave, capacidade);
        
        for (const auto& elem : tabela[indice]) {
            if (elem.chave == chave) {
                valorEncontrado = elem.valor;
                return true;
            }
        }
        return false;
    }
    
    bool remover(int chave) {
        size_t indice = calcularHash(chave, capacidade);
        
        auto it = find_if(tabela[indice].begin(), tabela[indice].end(),
                         [chave](const Elemento& e) { return e.chave == chave; });
        
        if (it != tabela[indice].end()) {
            tabela[indice].erase(it);
            elementos--;
            
            if (capacidade > 16 && obterFatorCarga() <= cargaMinima && 
                colisoes < limiteColisoes / 4) {
                redimensionar(capacidade / 2);
            }
            
            return true;
        }
        return false;
    }
    
    size_t obterColisoes() const { return colisoes; }
    size_t obterCapacidade() const { return capacidade; }
    size_t obterElementos() const { return elementos; }
    size_t obterRedimensionamentos() const { return redimensionamentos; }
    double obterFatorCarga() const { 
        return capacidade > 0 ? (double)elementos / capacidade : 0.0; 
    }
};

// Funções auxiliares para gerar os dados
vector<int> gerarNumerosSequenciais(int quantidade) {
    vector<int> numeros(quantidade);
    for (int i = 0; i < quantidade; i++) {
        numeros[i] = i;
    }
    return numeros;
}

vector<int> gerarNumerosAleatorios(int quantidade, int maximo = 1000000) {
    vector<int> numeros(quantidade);
    random_device rd;
    mt19937 gerador(rd());
    uniform_int_distribution<> distribuicao(0, maximo);
    
    for (int i = 0; i < quantidade; i++) {
        numeros[i] = distribuicao(gerador);
    }
    return numeros;
}


// TESTE 1: INSERÇÃO DE 1 MILHÃO DE ELEMENTOS
void executarTeste1() {
    cout << "=========================================================================" << endl
    << "TESTE 1: 1 MILHÃO DE INSERÇÕES SEQUENCIAIS" << endl
    << "=========================================================================" << endl << endl;
    
    const int TOTAL_ELEMENTOS = 1000000;
    vector<int> dados = gerarNumerosSequenciais(TOTAL_ELEMENTOS);
    
    cout << "Tamanho da amostra: " << TOTAL_ELEMENTOS << " elementos" << endl << endl;
    
    vector<pair<string, void*>> estrategias = {
        {"ESTÁTICA", new TabelaEstatica(10007)},
        {"DUPLICAÇÃO", new TabelaDuplicacao(8, 0.7, 0.2)},
        {"INCREMENTAL", new TabelaIncremental(8, 0.7, 5)},
        {"ADAPTATIVA", new TabelaAdaptativa(8, 0.7, 0.2)}
    };
    
    for (auto& [nome, tabelaPtr] : estrategias) {
        cout << "----------------------------------------" << endl
        << "ESTRATÉGIA: " << nome << endl
        << "----------------------------------------" << endl;
        
        auto inicio = chrono::high_resolution_clock::now();
        
        if (nome == "ESTÁTICA") {
            TabelaEstatica* tabela = static_cast<TabelaEstatica*>(tabelaPtr);
            for (int i = 0; i < TOTAL_ELEMENTOS; i++) {
                tabela->inserir(dados[i], dados[i] * 2);
            }
            
            auto fim = chrono::high_resolution_clock::now();
            auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
            
            cout << "Tempo total: " << duracao.count() << " ms" << endl;
            cout << "Tempo por operação: " << (duracao.count() * 1000000.0) / TOTAL_ELEMENTOS << " ns" << endl;
            cout << "Colisões: " << tabela->obterColisoes() << endl;
            cout << "Fator de carga final: " << tabela->obterFatorCarga() << endl;
            
            size_t memoriaKB = (tabela->obterCapacidade() * 16 + TOTAL_ELEMENTOS * 12) / 1024;
            cout << "Memória estimada: " << memoriaKB << " KB" << endl;
            
        } else if (nome == "DUPLICAÇÃO") {
            TabelaDuplicacao* tabela = static_cast<TabelaDuplicacao*>(tabelaPtr);
            for (int i = 0; i < TOTAL_ELEMENTOS; i++) {
                tabela->inserir(dados[i], dados[i] * 2);
            }
            
            auto fim = chrono::high_resolution_clock::now();
            auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
            
            cout << "Tempo total: " << duracao.count() << " ms" << endl;
            cout << "Tempo por operação: " << (duracao.count() * 1000000.0) / TOTAL_ELEMENTOS << " ns" << endl;
            cout << "Colisões: " << tabela->obterColisoes() << endl;
            cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            cout << "Fator de carga final: " << tabela->obterFatorCarga() << endl;
            
            size_t memoriaKB = (tabela->obterCapacidade() * 16 + TOTAL_ELEMENTOS * 12) / 1024;
            cout << "Memória estimada: " << memoriaKB << " KB" << endl;
            
        } else if (nome == "INCREMENTAL") {
            TabelaIncremental* tabela = static_cast<TabelaIncremental*>(tabelaPtr);
            for (int i = 0; i < TOTAL_ELEMENTOS; i++) {
                tabela->inserir(dados[i], dados[i] * 2);
            }
            
            auto fim = chrono::high_resolution_clock::now();
            auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
            
            cout << "Tempo total: " << duracao.count() << " ms" << endl;
            cout << "Tempo por operação: " << (duracao.count() * 1000000.0) / TOTAL_ELEMENTOS << " ns" << endl;
            cout << "Colisões: " << tabela->obterColisoes() << endl;
            cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            cout << "Fator de carga final: " << tabela->obterFatorCarga() << endl;
            
            size_t memoriaKB = (tabela->obterCapacidade() * 16 + TOTAL_ELEMENTOS * 12) / 1024;
            cout << "Memória estimada: " << memoriaKB << " KB" << endl;
            
        } else if (nome == "ADAPTATIVA") {
            TabelaAdaptativa* tabela = static_cast<TabelaAdaptativa*>(tabelaPtr);
            for (int i = 0; i < TOTAL_ELEMENTOS; i++) {
                tabela->inserir(dados[i], dados[i] * 2);
            }
            
            auto fim = chrono::high_resolution_clock::now();
            auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
            
            cout << "Tempo total: " << duracao.count() << " ms" << endl;
            cout << "Tempo por operação: " << (duracao.count() * 1000000.0) / TOTAL_ELEMENTOS << " ns" << endl;
            cout << "Colisões: " << tabela->obterColisoes() << endl;
            cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            cout << "Fator de carga final: " << tabela->obterFatorCarga() << endl;
            
            size_t memoriaKB = (tabela->obterCapacidade() * 16 + TOTAL_ELEMENTOS * 12) / 1024;
            cout << "Memória estimada: " << memoriaKB << " KB" << endl;
        }
        cout << endl;
    }
    
    for (auto& [nome, tabelaPtr] : estrategias) {
        if (nome == "ESTÁTICA") delete static_cast<TabelaEstatica*>(tabelaPtr);
        else if (nome == "DUPLICAÇÃO") delete static_cast<TabelaDuplicacao*>(tabelaPtr);
        else if (nome == "INCREMENTAL") delete static_cast<TabelaIncremental*>(tabelaPtr);
        else if (nome == "ADAPTATIVA") delete static_cast<TabelaAdaptativa*>(tabelaPtr);
    }
}

// TESTE 2: OPERAÇÕES MISTAS (INSERÇÃO + BUSCA)
void executarTeste2() {
    cout << endl << endl;
    cout << "=========================================================================" << endl;
    cout << "TESTE 2: 500.000 OPERAÇÕES MISTAS (50% INSERÇÃO, 50% BUSCA)" << endl;
    cout << "=========================================================================" << endl << endl;
    
    const int TOTAL_OPERACOES = 500000;
    
    random_device rd;
    mt19937 gerador(rd());
    uniform_int_distribution<> distChaves(0, 200000);
    uniform_real_distribution<> distOperacoes(0.0, 1.0);
    
    cout << "Tamanho da amostra: " << TOTAL_OPERACOES << " operações" << endl << endl;
    
    vector<pair<string, void*>> estrategias = {
        {"ESTÁTICA", new TabelaEstatica(10007)},
        {"DUPLICAÇÃO", new TabelaDuplicacao(8, 0.7, 0.2)},
        {"INCREMENTAL", new TabelaIncremental(8, 0.7, 5)},
        {"ADAPTATIVA", new TabelaAdaptativa(8, 0.7, 0.2)}
    };
    
    for (auto& [nome, tabelaPtr] : estrategias) {
        cout << "----------------------------------------" << endl
        << "ESTRATÉGIA: " << nome << endl
        << "----------------------------------------" << endl;
        
        int insercoes = 0;
        int buscas = 0;
        int buscasComSucesso = 0;
        
        auto inicio = chrono::high_resolution_clock::now();
        
        if (nome == "ESTÁTICA") {
            TabelaEstatica* tabela = static_cast<TabelaEstatica*>(tabelaPtr);
            
            for (int i = 0; i < 10000; i++) {
                tabela->inserir(i, i * 2);
            }
            
            for (int i = 0; i < TOTAL_OPERACOES; i++) {
                int chave = distChaves(gerador);
                
                if (distOperacoes(gerador) < 0.5) {
                    tabela->inserir(chave, chave * 3);
                    insercoes++;
                } else {
                    int valor;
                    if (tabela->buscar(chave, valor)) {
                        buscasComSucesso++;
                    }
                    buscas++;
                }
            }
            
        } else if (nome == "DUPLICAÇÃO") {
            TabelaDuplicacao* tabela = static_cast<TabelaDuplicacao*>(tabelaPtr);
            
            for (int i = 0; i < 10000; i++) {
                tabela->inserir(i, i * 2);
            }
            
            for (int i = 0; i < TOTAL_OPERACOES; i++) {
                int chave = distChaves(gerador);
                
                if (distOperacoes(gerador) < 0.5) {
                    tabela->inserir(chave, chave * 3);
                    insercoes++;
                } else {
                    int valor;
                    if (tabela->buscar(chave, valor)) {
                        buscasComSucesso++;
                    }
                    buscas++;
                }
            }
            
        } else if (nome == "INCREMENTAL") {
            TabelaIncremental* tabela = static_cast<TabelaIncremental*>(tabelaPtr);
            
            for (int i = 0; i < 10000; i++) {
                tabela->inserir(i, i * 2);
            }
            
            for (int i = 0; i < TOTAL_OPERACOES; i++) {
                int chave = distChaves(gerador);
                
                if (distOperacoes(gerador) < 0.5) {
                    tabela->inserir(chave, chave * 3);
                    insercoes++;
                } else {
                    int valor;
                    if (tabela->buscar(chave, valor)) {
                        buscasComSucesso++;
                    }
                    buscas++;
                }
            }
            
        } else if (nome == "ADAPTATIVA") {
            TabelaAdaptativa* tabela = static_cast<TabelaAdaptativa*>(tabelaPtr);
            
            for (int i = 0; i < 10000; i++) {
                tabela->inserir(i, i * 2);
            }
            
            for (int i = 0; i < TOTAL_OPERACOES; i++) {
                int chave = distChaves(gerador);
                
                if (distOperacoes(gerador) < 0.5) {
                    tabela->inserir(chave, chave * 3);
                    insercoes++;
                } else {
                    int valor;
                    if (tabela->buscar(chave, valor)) {
                        buscasComSucesso++;
                    }
                    buscas++;
                }
            }
        }
        
        auto fim = chrono::high_resolution_clock::now();
        auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
        
        cout << "Tempo total: " << duracao.count() << " ms" << endl;
        cout << "Inserções: " << insercoes << endl;
        cout << "Buscas: " << buscas << " (" << buscasComSucesso << " com sucesso)" << endl;
        cout << "Taxa de sucesso: " << (buscas > 0 ? (buscasComSucesso * 100.0 / buscas) : 0) << "%" << endl;
        cout << "Tempo por operação: " << (duracao.count() * 1000000.0) / TOTAL_OPERACOES << " ns" << endl;
        
        if (nome != "ESTÁTICA") {
            if (nome == "DUPLICAÇÃO") {
                TabelaDuplicacao* tabela = static_cast<TabelaDuplicacao*>(tabelaPtr);
                cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            } else if (nome == "INCREMENTAL") {
                TabelaIncremental* tabela = static_cast<TabelaIncremental*>(tabelaPtr);
                cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            } else if (nome == "ADAPTATIVA") {
                TabelaAdaptativa* tabela = static_cast<TabelaAdaptativa*>(tabelaPtr);
                cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            }
        }
        cout << endl;
    }
    
    for (auto& [nome, tabelaPtr] : estrategias) {
        if (nome == "ESTÁTICA") delete static_cast<TabelaEstatica*>(tabelaPtr);
        else if (nome == "DUPLICAÇÃO") delete static_cast<TabelaDuplicacao*>(tabelaPtr);
        else if (nome == "INCREMENTAL") delete static_cast<TabelaIncremental*>(tabelaPtr);
        else if (nome == "ADAPTATIVA") delete static_cast<TabelaAdaptativa*>(tabelaPtr);
    }
}

// TESTE 3: CARGA VARIÁVEL
void executarTeste3() {
    cout << endl << endl
     << "=========================================================================" << endl
     << "TESTE 3: CARGA VARIÁVEL COM FASES DIFERENTES" << endl
     << "=========================================================================" << endl << endl;
    
    cout << "Fases do teste:" << endl;
    cout << "1) Inserção de 100.000 elementos" << endl;
    cout << "2) Busca em 100.000 elementos" << endl;
    cout << "3) Remoção de 50.000 e inserção de 50.000" << endl << endl;
    
    vector<pair<string, void*>> estrategias = {
        {"ESTÁTICA", new TabelaEstatica(10007)},
        {"DUPLICAÇÃO", new TabelaDuplicacao(8, 0.7, 0.2)},
        {"INCREMENTAL", new TabelaIncremental(8, 0.7, 5)},
        {"ADAPTATIVA", new TabelaAdaptativa(8, 0.7, 0.2)}
    };
    
    for (auto& [nome, tabelaPtr] : estrategias) {
        cout << "----------------------------------------" << endl
        << "ESTRATÉGIA: " << nome << endl
        << "----------------------------------------" << endl;
        
        auto inicioTotal = chrono::high_resolution_clock::now();
        
        // FASE 1: Inserção
        cout << "FASE 1 - Inserção de 100.000 elementos:" << endl;
        auto inicio = chrono::high_resolution_clock::now();
        
        if (nome == "ESTÁTICA") {
            TabelaEstatica* tabela = static_cast<TabelaEstatica*>(tabelaPtr);
            for (int i = 0; i < 100000; i++) {
                tabela->inserir(i, i * 2);
            }
        } else if (nome == "DUPLICAÇÃO") {
            TabelaDuplicacao* tabela = static_cast<TabelaDuplicacao*>(tabelaPtr);
            for (int i = 0; i < 100000; i++) {
                tabela->inserir(i, i * 2);
            }
        } else if (nome == "INCREMENTAL") {
            TabelaIncremental* tabela = static_cast<TabelaIncremental*>(tabelaPtr);
            for (int i = 0; i < 100000; i++) {
                tabela->inserir(i, i * 2);
            }
        } else if (nome == "ADAPTATIVA") {
            TabelaAdaptativa* tabela = static_cast<TabelaAdaptativa*>(tabelaPtr);
            for (int i = 0; i < 100000; i++) {
                tabela->inserir(i, i * 2);
            }
        }
        
        auto fim = chrono::high_resolution_clock::now();
        auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
        
        if (nome == "ESTÁTICA") {
            TabelaEstatica* tabela = static_cast<TabelaEstatica*>(tabelaPtr);
            cout << "  Tempo: " << duracao.count() << " ms, Colisões: " << tabela->obterColisoes() << endl;
        } else if (nome == "DUPLICAÇÃO") {
            TabelaDuplicacao* tabela = static_cast<TabelaDuplicacao*>(tabelaPtr);
            cout << "  Tempo: " << duracao.count() << " ms, Colisões: " << tabela->obterColisoes() << endl;
        } else if (nome == "INCREMENTAL") {
            TabelaIncremental* tabela = static_cast<TabelaIncremental*>(tabelaPtr);
            cout << "  Tempo: " << duracao.count() << " ms, Colisões: " << tabela->obterColisoes() << endl;
        } else if (nome == "ADAPTATIVA") {
            TabelaAdaptativa* tabela = static_cast<TabelaAdaptativa*>(tabelaPtr);
            cout << "  Tempo: " << duracao.count() << " ms, Colisões: " << tabela->obterColisoes() << endl;
        }
        
        // FASE 2: Buscas
        cout << "FASE 2 - 100.000 buscas aleatórias:" << endl;
        inicio = chrono::high_resolution_clock::now();
        
        random_device rd;
        mt19937 gerador(rd());
        uniform_int_distribution<> dist(0, 99999);
        
        int acertos = 0;
        for (int i = 0; i < 100000; i++) {
            int chave = dist(gerador);
            int valor;
            
            if (nome == "ESTÁTICA") {
                TabelaEstatica* tabela = static_cast<TabelaEstatica*>(tabelaPtr);
                if (tabela->buscar(chave, valor)) acertos++;
            } else if (nome == "DUPLICAÇÃO") {
                TabelaDuplicacao* tabela = static_cast<TabelaDuplicacao*>(tabelaPtr);
                if (tabela->buscar(chave, valor)) acertos++;
            } else if (nome == "INCREMENTAL") {
                TabelaIncremental* tabela = static_cast<TabelaIncremental*>(tabelaPtr);
                if (tabela->buscar(chave, valor)) acertos++;
            } else if (nome == "ADAPTATIVA") {
                TabelaAdaptativa* tabela = static_cast<TabelaAdaptativa*>(tabelaPtr);
                if (tabela->buscar(chave, valor)) acertos++;
            }
        }
        
        fim = chrono::high_resolution_clock::now();
        duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
        cout << "  Tempo: " << duracao.count() << " ms, Acertos: " << acertos << "/100000" << endl;
        
        // FASE 3: Remoções e novas inserções
        cout << "FASE 3 - Remoção de 50.000 e inserção de 50.000:" << endl;
        inicio = chrono::high_resolution_clock::now();
        
        if (nome == "ESTÁTICA") {
            TabelaEstatica* tabela = static_cast<TabelaEstatica*>(tabelaPtr);
            for (int i = 0; i < 50000; i++) tabela->remover(i);
            for (int i = 100000; i < 150000; i++) tabela->inserir(i, i * 3);
        } else if (nome == "DUPLICAÇÃO") {
            TabelaDuplicacao* tabela = static_cast<TabelaDuplicacao*>(tabelaPtr);
            for (int i = 0; i < 50000; i++) tabela->remover(i);
            for (int i = 100000; i < 150000; i++) tabela->inserir(i, i * 3);
        } else if (nome == "INCREMENTAL") {
            TabelaIncremental* tabela = static_cast<TabelaIncremental*>(tabelaPtr);
            for (int i = 0; i < 50000; i++) tabela->remover(i);
            for (int i = 100000; i < 150000; i++) tabela->inserir(i, i * 3);
        } else if (nome == "ADAPTATIVA") {
            TabelaAdaptativa* tabela = static_cast<TabelaAdaptativa*>(tabelaPtr);
            for (int i = 0; i < 50000; i++) tabela->remover(i);
            for (int i = 100000; i < 150000; i++) tabela->inserir(i, i * 3);
        }
        
        fim = chrono::high_resolution_clock::now();
        duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
        cout << "  Tempo: " << duracao.count() << " ms" << endl;
        
        // RESULTADOS FINAIS
        auto fimTotal = chrono::high_resolution_clock::now();
        auto duracaoTotal = chrono::duration_cast<chrono::milliseconds>(fimTotal - inicioTotal);
        
        cout << endl << "RESULTADOS FINAIS:" << endl;
        cout << "Tempo total das 3 fases: " << duracaoTotal.count() << " ms" << endl;
        
        if (nome == "ESTÁTICA") {
            TabelaEstatica* tabela = static_cast<TabelaEstatica*>(tabelaPtr);
            cout << "Colisões totais: " << tabela->obterColisoes() << endl;
            cout << "Fator de carga final: " << tabela->obterFatorCarga() << endl;
        } else if (nome == "DUPLICAÇÃO") {
            TabelaDuplicacao* tabela = static_cast<TabelaDuplicacao*>(tabelaPtr);
            cout << "Colisões totais: " << tabela->obterColisoes() << endl;
            cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            cout << "Fator de carga final: " << tabela->obterFatorCarga() << endl;
        } else if (nome == "INCREMENTAL") {
            TabelaIncremental* tabela = static_cast<TabelaIncremental*>(tabelaPtr);
            cout << "Colisões totais: " << tabela->obterColisoes() << endl;
            cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            cout << "Fator de carga final: " << tabela->obterFatorCarga() << endl;
        } else if (nome == "ADAPTATIVA") {
            TabelaAdaptativa* tabela = static_cast<TabelaAdaptativa*>(tabelaPtr);
            cout << "Colisões totais: " << tabela->obterColisoes() << endl;
            cout << "Redimensionamentos: " << tabela->obterRedimensionamentos() << endl;
            cout << "Fator de carga final: " << tabela->obterFatorCarga() << endl;
        }
        cout << endl;
    }
    
    for (auto& [nome, tabelaPtr] : estrategias) {
        if (nome == "ESTÁTICA") delete static_cast<TabelaEstatica*>(tabelaPtr);
        else if (nome == "DUPLICAÇÃO") delete static_cast<TabelaDuplicacao*>(tabelaPtr);
        else if (nome == "INCREMENTAL") delete static_cast<TabelaIncremental*>(tabelaPtr);
        else if (nome == "ADAPTATIVA") delete static_cast<TabelaAdaptativa*>(tabelaPtr);
    }
}

// PROGRAMA PRINCIPAL
int main() {
    cout << "==================================================================" << endl
    << "EXPERIMENTO: DIMENSIONAMENTO DINÂMICO EM TABELAS HASH" << endl
    << "Comparação de 4 Estratégias" << endl
    << "==================================================================" << endl << endl;
    
    cout << "Pressione ENTER para iniciar o Teste 1" << endl;
    cin.get();
    
    executarTeste1();
    
    cout << endl << "Pressione ENTER para iniciar o Teste 2" << endl;
    cin.get();
    
    executarTeste2();
    
    cout << endl << "Pressione ENTER para iniciar o Teste 3" << endl;
    cin.get();
    
    executarTeste3();
    
    cout << "==================================================================" << endl
    << "EXPERIMENTO CONCLUÍDO!" << endl
    << "==================================================================" << endl;
    
    return 0;
}