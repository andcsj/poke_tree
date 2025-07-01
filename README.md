# Sistema de Gerenciamento de Cartas Pokémon

Este projeto implementa um sistema completo para gerenciamento de cartas Pokémon, utilizando estruturas de dados avançadas como tabela hash, árvore AVL e árvore BST.

## Visão Geral

O sistema permite:
- Importar cartas da Pokémon TCG API
- Armazenar cartas em múltiplas estruturas eficientes
- Buscar cartas por ID ou nome
- Garantir balanceamento e desempenho com AVL e hash dinâmico

---

## Estruturas Principais

### Enumerações
- `JogoTipo`: Define o tipo de jogo (atualmente apenas `POKEMON`)

### Estruturas de Dados
- `Carta`: Armazena os dados de uma carta Pokémon:
  - `id`, `nome`, `jogo`, `poder`, `resistencia`, `tipos`, `rating`
- `NodeAVL`: Nó da árvore AVL (balanceada por nome)
- `NodeBST`: Nó da árvore BST (pode ser usada para ordenação alternativa)
- `tabelaHash`: Vetor de listas encadeadas com controle de colisão por encadeamento separado

---

## Classe Principal: GerenciadorCartas

### Atributos Privados
- `tabelaHash`: Vetor de listas de ponteiros compartilhados
- `tamanho`: Número atual de cartas
- `capacidade`: Tamanho atual da hash table
- `LIMITE_CARGA`: Fator de carga máximo antes de rehash (0.7)
- `raizAVL`: Raiz da árvore AVL

---

## Métodos Principais

### 1. Operações Internas
- `hash()`: Calcula o índice com double hashing
- `rehash()`: Duplica a capacidade da tabela hash e redistribui os elementos
- `altura()`: Calcula a altura de um nó AVL
- `rotacionarDireita()`, `rotacionarEsquerda()`: Rotacionam subárvores AVL
- `inserirAVL()`: Insere carta na árvore AVL com balanceamento
- `buscarPorId()`: Busca carta por ID na hash table
- `buscarPorNome()`: Busca na árvore AVL por nome

### 2. Importação de Dados (a partir de `libcurl`)
- `carregarDaAPI()`: (presumivelmente implementado em outro trecho do código) carrega cartas da API da Pokémon TCG

---

## Recursos Suportados

- Armazenamento eficiente via ponteiros inteligentes (`shared_ptr`, `unique_ptr`)
- Balanceamento automático da árvore AVL
- Hash table com tratamento robusto de colisões
- Estrutura modular para fácil extensão

---

## Estruturas de Dados Utilizadas

### 1. Tabela Hash
- Utiliza vetor de listas (`vector<list<shared_ptr<Carta>>>`)
- Tratamento de colisões com encadeamento separado
- Rehash automático baseado no fator de carga (>= 0.7)

### 2. Árvore AVL
- Balanceada automaticamente
- Baseada em nome de carta
- Inserção com rotações simples e duplas

### 3. Árvore BST
- Estrutura auxiliar com potencial para ordenação por outro critério (ex: rating)

---

## Desempenho

| Operação          | Complexidade esperada |
|-------------------|-----------------------|
| Inserção Hash     | O(1) médio |
| Rehash            | O(n) |
| Inserção AVL      | O(log n) |
| Busca por ID      | O(1) esperado |
| Busca por nome    | O(log n) (AVL) |

---

## Dependências

- `libcurl`: para requisições HTTP
- `nlohmann/json`: para parsing de JSON
- Compilador compatível com C++17 ou superior

---

## Como Compilar

```bash
g++ -lcurl -o poketree poketree.cpp
