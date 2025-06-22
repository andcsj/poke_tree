#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <queue>
#include <functional>
#include <unordered_set>

using namespace std;

enum class JogoTipo { POKEMON };

struct Carta {
    int id;
    string nome;
    JogoTipo jogo;
    int poder;
    int resistencia;
    vector<string> tipos;
    float rating;

    Carta(int i, string n, JogoTipo j, int p, int r, vector<string> t, float rt)
        : id(i), nome(n), jogo(j), poder(p), resistencia(r), tipos(t), rating(rt) {}
};

class GerenciadorCartas {
private:
    struct NodeAVL {
        shared_ptr<Carta> carta;
        unique_ptr<NodeAVL> esquerda;
        unique_ptr<NodeAVL> direita;
        int altura;
        NodeAVL(shared_ptr<Carta> c) : carta(c), altura(1) {}
    };

    struct NodeBST {
        shared_ptr<Carta> carta;
        unique_ptr<NodeBST> esquerda;
        unique_ptr<NodeBST> direita;
        NodeBST(shared_ptr<Carta> c) : carta(c) {}
    };

    vector<list<shared_ptr<Carta>>> tabelaHash;
    int tamanho;
    int capacidade;
    const float LIMITE_CARGA = 0.7f;
    unique_ptr<NodeAVL> raizAVL;

    size_t hash(int id, int tentativa = 0, int cap = -1) const {
        if (cap == -1) cap = capacidade;
        const size_t primo = 2654435761;
        size_t h = id * primo;
        return (h + tentativa * (h >> 16)) % cap;
    }

    void rehash() {
        int novaCapacidade = capacidade * 2;
        vector<list<shared_ptr<Carta>>> novaTabela(novaCapacidade);
        
        for (auto& bucket : tabelaHash) {
            for (auto& carta : bucket) {
                size_t indice = hash(carta->id, 0, novaCapacidade);
                novaTabela[indice].push_back(carta);
            }
        }
        
        tabelaHash = move(novaTabela);
        capacidade = novaCapacidade;
    }

    int altura(const unique_ptr<NodeAVL>& node) const {
        return node ? node->altura : 0;
    }

    unique_ptr<NodeAVL> rotacionarDireita(unique_ptr<NodeAVL> y) {
        auto x = move(y->esquerda);
        y->esquerda = move(x->direita);
        x->direita = move(y);

        x->direita->altura = max(altura(x->direita->esquerda), altura(x->direita->direita)) + 1;
        x->altura = max(altura(x->esquerda), altura(x->direita)) + 1;
        return x;
    }

    unique_ptr<NodeAVL> rotacionarEsquerda(unique_ptr<NodeAVL> x) {
        auto y = move(x->direita);
        x->direita = move(y->esquerda);
        y->esquerda = move(x);

        y->esquerda->altura = max(altura(y->esquerda->esquerda), altura(y->esquerda->direita)) + 1;
        y->altura = max(altura(y->esquerda), altura(y->direita)) + 1;
        return y;
    }

    unique_ptr<NodeAVL> inserirAVL(unique_ptr<NodeAVL> node, shared_ptr<Carta> carta) {
        if (!node) return make_unique<NodeAVL>(carta);

        if (carta->id < node->carta->id)
            node->esquerda = inserirAVL(move(node->esquerda), carta);
        else if (carta->id > node->carta->id)
            node->direita = inserirAVL(move(node->direita), carta);
        else
            return node;

        node->altura = 1 + max(altura(node->esquerda), altura(node->direita));
        int balance = altura(node->esquerda) - altura(node->direita);

        if (balance > 1 && carta->id < node->esquerda->carta->id)
            return rotacionarDireita(move(node));
        if (balance < -1 && carta->id > node->direita->carta->id)
            return rotacionarEsquerda(move(node));
        if (balance > 1 && carta->id > node->esquerda->carta->id) {
            node->esquerda = rotacionarEsquerda(move(node->esquerda));
            return rotacionarDireita(move(node));
        }
        if (balance < -1 && carta->id < node->direita->carta->id) {
            node->direita = rotacionarDireita(move(node->direita));
            return rotacionarEsquerda(move(node));
        }
        return node;
    }

    void imprimirBST(const unique_ptr<NodeBST>& node, const string& prefix = "", bool isLeft = true) const {
        if (!node) return;
        if (node->direita)
            imprimirBST(node->direita, prefix + (isLeft ? "│   " : "    "), false);
        cout << prefix << (isLeft ? "└── " : "┌── ") << node->carta->rating << "\n";
        if (node->esquerda)
            imprimirBST(node->esquerda, prefix + (isLeft ? "    " : "│   "), true);
    }

    void inserirBST(unique_ptr<NodeBST>& raiz, shared_ptr<Carta> carta, float rating, bool maiorQue) {
        if ((maiorQue && carta->rating <= rating) || (!maiorQue && carta->rating >= rating)) return;
        
        if (!raiz) {
            raiz = make_unique<NodeBST>(carta);
        } else {
            if (carta->rating < raiz->carta->rating) {
                inserirBST(raiz->esquerda, carta, rating, maiorQue);
            } else {
                inserirBST(raiz->direita, carta, rating, maiorQue);
            }
        }
    }

    void montarBSTPorRating(float rating, bool maiorQue) {
        unique_ptr<NodeBST> raizBST;
        for (auto& bucket : tabelaHash) {
            for (auto& carta : bucket) {
                inserirBST(raizBST, carta, rating, maiorQue);
            }
        }
        cout << "\nBST com cartas com rating " << (maiorQue ? ">" : "<") << " " << rating << ":\n";
        imprimirBST(raizBST);
    }

    int contarNosAVL(const unique_ptr<NodeAVL>& node) const {
        if (!node) return 0;
        return 1 + contarNosAVL(node->esquerda) + contarNosAVL(node->direita);
    }

public:
    GerenciadorCartas(int cap = 100) : tamanho(0), capacidade(max(2, cap)) {
        tabelaHash.resize(capacidade);
    }

    void inserirManual(int id, string nome, int poder, int resistencia, vector<string> tipos, float rating) {
        auto carta = make_shared<Carta>(id, nome, JogoTipo::POKEMON, poder, resistencia, tipos, rating);
        adicionarCarta(carta);
    }

    void inserirPorCSV(const string& caminho, int limite) {
        ifstream arquivo(caminho);
        if (!arquivo.is_open()) {
            cerr << "Erro ao abrir arquivo: " << caminho << endl;
            return;
        }

        string linha;
        int contador = 0;
        bool cabecalho = true;

        while (getline(arquivo, linha) && contador < limite) {
            if (cabecalho) {
                cabecalho = false;
                continue;
            }

            stringstream ss(linha);
            string campo;
            vector<string> campos;

            while (getline(ss, campo, ',')) {
                campos.push_back(campo);
            }

            if (campos.size() >= 5) {
                try {
                    int id = stoi(campos[0]);
                    string nome = campos[1];
                    int poder = stoi(campos[2]);
                    int resistencia = stoi(campos[3]);
                    float rating = stof(campos[4]);
                    vector<string> tipos = {"Pokémon"};

                    inserirManual(id, nome, poder, resistencia, tipos, rating);
                    contador++;
                } catch (const exception& e) {
                    cerr << "Erro na linha " << (contador+1) << ": " << e.what() << endl;
                }
            }
        }
        cout << "Total de cartas carregadas: " << contador << endl;
    }

    void adicionarCarta(shared_ptr<Carta> carta) {
        float fatorCarga = static_cast<float>(tamanho + 1) / capacidade;
        if (fatorCarga >= LIMITE_CARGA) rehash();

        size_t indice = hash(carta->id);
        for (auto& c : tabelaHash[indice]) {
            if (c->id == carta->id) {
                return; // ID duplicado
            }
        }

        tabelaHash[indice].push_back(carta);
        tamanho++;
        raizAVL = inserirAVL(move(raizAVL), carta);
    }

    void exibirArvoreAVL() const {
        cout << "\n=== Estrutura da Árvore AVL (por ID) ===\n";
        function<void(const unique_ptr<NodeAVL>&, string, bool)> printAVL = 
            [&](const unique_ptr<NodeAVL>& node, const string& prefix, bool isLeft) {
                if (!node) return;
                if (node->direita)
                    printAVL(node->direita, prefix + (isLeft ? "│   " : "    "), false);
                cout << prefix << (isLeft ? "└── " : "┌── ") << node->carta->rating << "\n";
                if (node->esquerda)
                    printAVL(node->esquerda, prefix + (isLeft ? "    " : "│   "), true);
            };
        printAVL(raizAVL, "", true);
        cout << "\nTotal de cartas: " << tamanho << "\n";
    }

    void exibirArvoreAVLCompleta() const {
        cout << "\n=== Todos os IDs e Ratings ===\n";
        function<void(const unique_ptr<NodeAVL>&)> inOrder = 
            [&](const unique_ptr<NodeAVL>& node) {
                if (!node) return;
                inOrder(node->esquerda);
                cout << "ID: " << node->carta->id << " | Rating: " << node->carta->rating << "\n";
                inOrder(node->direita);
            };
        inOrder(raizAVL);
    }

    void exibirTopCartas(int limite = 20) const {
        cout << "\n=== Top " << limite << " Cartas (Maior Rating) ===\n";
        vector<shared_ptr<Carta>> cartas;

        function<void(const unique_ptr<NodeAVL>&)> coletar = 
            [&](const unique_ptr<NodeAVL>& node) {
                if (!node) return;
                coletar(node->direita);
                cartas.push_back(node->carta);
                coletar(node->esquerda);
            };
        coletar(raizAVL);

        // Ordena por rating decrescente
        sort(cartas.begin(), cartas.end(), 
            [](const auto& a, const auto& b) { return a->rating > b->rating; });

        for (int i = 0; i < min(limite, (int)cartas.size()); i++) {
            cout << i+1 << ". ID: " << cartas[i]->id 
                 << " | Rating: " << cartas[i]->rating << "\n";
        }
    }

    int contarNosAVL() const {
        return contarNosAVL(raizAVL);
    }

    void montarBSTDoUsuario() {
        float rating;
        int opcao;
        cout << "Digite o rating base: ";
        cin >> rating;
        cout << "Deseja cartas com rating (1) MAIOR ou (2) MENOR que " << rating << "? ";
        cin >> opcao;
        montarBSTPorRating(rating, opcao == 1);
    }

    void excluirPorId(int id) {
        cout << "Exclusão ainda não implementada.\n";
    }

    void destruirArvore() {
        raizAVL.reset();
        cout << "Árvore destruída.\n";
    }
};

int main() {
    GerenciadorCartas gerenciador;
    int opcao;
    
    do {
        cout << "\n=== MENU PRINCIPAL ===\n";
        cout << "1. Exibir Estrutura da Árvore AVL\n";
        cout << "2. Exibir Top 20 Cartas por Rating\n";
        cout << "3. Verificar Quantidade de Cartas\n";
        cout << "4. Inserir Carta Manualmente\n";
        cout << "5. Carregar Cartas de Arquivo CSV\n";
        cout << "6. Montar BST por Rating\n";
        cout << "7. Excluir Carta por ID\n";
        cout << "8. Destruir Árvore AVL\n";
        cout << "9. Exibir Estatísticas\n";
        cout << "10. Exibir Todas as Cartas\n";
        cout << "0. Sair\n";
        cout << "Escolha: ";
        cin >> opcao;
        cin.ignore();

        switch(opcao) {
            case 1: {
                gerenciador.exibirArvoreAVL();
                break;
            }
            case 2: {
                gerenciador.exibirTopCartas();
                break;
            }
            case 3: {
                int total = gerenciador.contarNosAVL();
                cout << "\nTotal de cartas na estrutura: " << total << endl;
                if (total == 10000) {
                    cout << "✓ Todos os 10.000 registros foram carregados!\n";
                }
                break;
            }
            case 4: {
                int id, poder, resistencia;
                float rating;
                string nome;
                
                cout << "\n--- Inserção Manual ---\n";
                cout << "ID: "; cin >> id;
                cout << "Nome: "; cin.ignore(); getline(cin, nome);
                cout << "Poder: "; cin >> poder;
                cout << "Resistência: "; cin >> resistencia;
                cout << "Rating (0-10): "; cin >> rating;
                
                gerenciador.inserirManual(id, nome, poder, resistencia, {"Pokémon"}, rating);
                cout << "Carta adicionada com sucesso!\n";
                break;
            }
            case 5: {
                string caminho = "./pokemons.csv";
                int limite;
                
                cout << "\n--- Carregar CSV ---\n";
                cout << "Quantidade a importar (500/1000/5000/10000): ";
                cin >> limite;
                
                auto inicio = chrono::high_resolution_clock::now();
                gerenciador.inserirPorCSV(caminho, limite);
                auto fim = chrono::high_resolution_clock::now();
                
                auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
                cout << "Tempo de carregamento: " << duracao.count() << "ms\n";
                break;
            }
            case 6: {
                gerenciador.montarBSTDoUsuario();
                break;
            }
            case 7: {
                int id;
                cout << "\n--- Exclusão ---\n";
                cout << "ID da carta a remover: "; cin >> id;
                gerenciador.excluirPorId(id);
                break;
            }
            case 8: {
                gerenciador.destruirArvore();
                cout << "Árvore AVL reinicializada.\n";
                break;
            }
            case 9: {
                cout << "\n=== ESTATÍSTICAS ===\n";
                cout << "Total de cartas: " << gerenciador.contarNosAVL() << endl;
                cout << "Memória aproximada: " << (gerenciador.contarNosAVL() * 128)/1024 << " KB\n";
                break;
            }

            case 10: {  // Opção para exibir todos os IDs
                gerenciador.exibirArvoreAVLCompleta();
                break;
            }
            case 0: {
                cout << "Encerrando o programa...\n";
                break;
            }
            default: {
                cout << "Opção inválida!\n";
            }
        }
    } while (opcao != 0);

    return 0;
}