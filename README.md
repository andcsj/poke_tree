# Sistema de Gerenciamento de Cartas Pokémon

Este projeto implementa um sistema completo para gerenciamento de cartas Pokémon, utilizando estruturas de dados avançadas como tabela hash, árvore AVL e árvore BST.

## Visão Geral

O sistema permite:
- Importar cartas de diferentes fontes (API, CSV ou manualmente)
- Realizar buscas complexas
- Exibir estatísticas
- Gerenciar e exportar sua coleção de cartas

---

## Estruturas Principais

### Enumerações
- `JogoTipo`: Define o tipo de jogo (atualmente apenas `POKEMON`)

### Estruturas de Dados
- `Carta`: Armazena os dados de uma carta Pokémon:
  - `id`, `nome`, `jogo`, `poder`, `resistencia`, `tipos`, `rating`
- `NodeAVL`: Nó da árvore AVL (balanceada por `id`)
- `NodeBST`: Nó da árvore BST (utilizada para exibição por `rating`)

---

## Classe Principal: GerenciadorCartas

### Atributos Privados
- `tabelaHash`: Vetor de listas encadeadas para armazenamento das cartas
- `tamanho`: Número atual de cartas
- `capacidade`: Tamanho da tabela hash
- `LIMITE_CARGA`: Fator de carga máximo antes de rehash (0.7)
- `raizAVL`: Ponteiro para a raiz da árvore AVL

---

## Métodos Principais

### 1. Operações Básicas
- `adicionarCarta()`: Adiciona uma nova carta
- `cartaExiste()`: Verifica existência por ID
- `buscarPorId()`: Busca rápida usando hash
- `buscarPorNome()`: Busca linear por substring no nome
- `buscarPorRating()`: Busca intervalo de ratings via AVL

### 2. Importação de Dados
- `carregarDaAPI()`: Importa cartas da Pokémon TCG API
- `inserirPorCSV()`: Lê e importa cartas de um arquivo `.csv`
- `inserirManual()`: Inserção manual via terminal

### 3. Visualização
- `exibirArvoreAVL()`: Mostra estrutura AVL
- `exibirTopCartas()`: Exibe cartas com maior `rating`
- `exibirArvoreAVLCompleta()`: Lista todas as cartas ordenadas por `id`
- `montarBSTPorRating()`: Cria e exibe BST temporária por `rating`

### 4. Gerenciamento de Memória
- `removerAVL()`: Remove nó AVL com balanceamento
- `excluirPorId()`: Remove uma carta específica
- `limparSeguro()`: Limpa todas as estruturas
- `destruirArvore()`: Reseta o sistema

### 5. Estatísticas
- `exibirEstatisticas()`: Mostra métricas de desempenho
- `contarNosAVL()`: Retorna número de cartas na AVL

### 6. Exportação
- `exportarParaCSV()`: Salva todas as cartas em `.csv`

---

## Métodos Auxiliares

- Funções de hash e rehash
- Operações de rotação e balanceamento AVL
- Funções de leitura segura de entrada (`lerInteiro`, `lerFloat`)
- Callback de requisição HTTP via `libcurl`

---

## Menu Interativo

O programa oferece 15 opções via terminal:

1. Carregar da API  
2. Carregar de CSV  
3. Inserção manual  
4. Exibir estrutura AVL  
5. Exibir top cartas  
6. Listar todas as cartas  
7. Buscar por ID  
8. Buscar por nome  
9. Buscar por rating  
10. Verificar quantidade  
11. Montar BST por rating  
12. Excluir carta  
13. Destruir árvore  
14. Exibir estatísticas  
15. Exportar para CSV  
16. Sair  

---

## Estruturas de Dados Utilizadas

### 1. Tabela Hash
- Encadeamento separado
- Função hash com multiplicação + double hashing
- Rehash automático ao atingir 70% de carga

### 2. Árvore AVL
- Balanceada automaticamente
- Ordenada por ID
- Suporte a buscas por intervalo de ratings

### 3. Árvore BST
- Construída temporariamente para exibição
- Ordenada por `rating`
- Destruída após uso

---

## Desempenho

| Operação          | Complexidade |
|-------------------|--------------|
| Inserção          | O(1) (hash) + O(log n) (AVL) |
| Busca por ID      | O(1) esperado |
| Busca por nome    | O(n) |
| Busca por rating  | O(n) (percurso in-order na AVL) |
| Rehash            | O(n) |

---

## Dependências

- `libcurl`: para requisições HTTP  
- `nlohmann/json`: para manipulação de JSON  
- Compilador compatível com C++11 ou superior

---
