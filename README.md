# Teste Comparativo Experimental de Dimensionamento Dinâmico em Tabelas Hash

## 📋 Descrição

Este projeto implementa e compara quatro estratégias diferentes de dimensionamento dinâmico para tabelas hash:

1. **Tabela Hash Estática** - Tabela com tamanho fixo
2. **Tabela Hash com Redimensionamento por duplicação** - Dobra o tamanho da tabela
3. **Tabela Hash com Redimensionamento Incremental** - Aumenta o tamanho incrementalmente
4. **Tabela Hash com Redimensionamento Adaptativo** - Ajusta o tamanho de forma adaptativa com base na ocupação

## 🎯 Objetivo

Realizar testes comparativos experimentais para avaliar o desempenho das diferentes estratégias de dimensionamento dinâmico em operações de inserção, busca e remoção.

## 🏗️ Estrutura do Projeto

```
├── Testes_hash_table.cpp    # Código principal com implementação de todas as estratégias
├── output/                   # Diretório para saída executável
│   └── Testes_hash_table    # Executável compilado
└── README.md                # Este arquivo
```

## 📚 Componentes Principais

### Classes Implementadas

#### 1. **TabelaEstatica**
- Tabela hash com tamanho fixo
- Usa encadeamento (chaining) para resolver colisões
- Método de hash: multiplicação pelo número áureo

#### 2. **TabelaDuplicacao**
- Tabela hash com redimensionamento por duplicação (doubling)
- Dobra a capacidade quando o fator de carga ultrapassa o limite máximo
- Reduz pela metade quando o fator de carga fica abaixo do limite mínimo
- Oferece custo amortizado O(1) nas inserções e é adequada para aplicações reais

#### 3. **TabelaIncremental**
- Tabela hash com redimensionamento incremental
- Aumenta capacidade de forma gradual (incrementa um valor fixo)
- Realiza rehash quando o fator de carga ultrapassa um limite

#### 4. **TabelaAdaptativa**
- Tabela hash com redimensionamento adaptativo
- Ajusta o tamanho dinamicamente baseado na ocupação
- Implementa política de dobro/redução de capacidade

### Estruturas de Dados

- **Elemento**: Estrutura chave-valor com suporte a duplicatas de chaves

## 🔧 Métodos Disponíveis

Todas as classes implementam:
- `inserir(chave, valor)` - Insere um elemento na tabela
- `buscar(chave)` - Busca um elemento pela chave
- `remover(chave)` - Remove um elemento da tabela
- `obterEstatisticas()` - Retorna estatísticas de desempenho

## 🚀 Como Compilar

```bash
g++ -std=c++17 -O2 Testes_hash_table.cpp -o output/Testes_hash_table
```

## ▶️ Como Executar

```bash
./output/Testes_hash_table
```

## 📊 Métrica de Desempenho

O programa coleta as seguintes métricas para cada tabela:
- **Tempo de execução** (inserções, buscas, remoções)
- **Número de colisões**
- **Fator de carga** (relação entre elementos e capacidade)
- **Eficiência espacial**

## 🔗 Função Hash Utilizada

```cpp
size_t calcularHash(int chave, size_t tamanhoTabela) {
    const unsigned long long a = 2654435761ULL;  // φ - 1 * 2^32
    const unsigned long long b = 40503ULL;
    const unsigned long long p = 4294967291ULL;  // Número primo
    
    unsigned long long valorHash = ((a * chave + b) % p);
    return valorHash % tamanhoTabela;
}
```

A função usa o método de multiplicação com a proporção áurea para garantir boa distribuição dos valores de hash.

## 💡 Insights do Projeto

Este projeto permite experimentação com:
- Trade-offs entre fragmentação de memória e tempo de acesso
- Impacto do fator de carga no desempenho
- Comparação de diferentes políticas de redimensionamento
- Análise empírica de colisões em diferentes cenários

## 🛠️ Requisitos

- Compilador C++ com suporte a C++17
- Bibliotecas padrão do C++ (`<vector>`, `<list>`, `<chrono>`, etc.)

## 📝 Notas

- O projeto usa encadeamento (chaining) para resolução de colisões
- Cada estratégia mantém contadores de colisões e estatísticas
- Os testes medem tempo de execução com `chrono::high_resolution_clock`

## ✅ Status

Código compilável e funcional com as três estratégias de dimensionamento dinâmico implementadas.

---

**Autores**:
- Caio César de Morais Batista
- Herick Bruno Araújo Dantas de Lima
- Murillo Valdeger da Costa

**Data**: Dezembro de 2025
