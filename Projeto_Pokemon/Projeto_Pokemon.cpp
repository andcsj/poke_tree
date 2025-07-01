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
#include <iomanip>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

enum class JogoTipo { POKEMON };

struct Carta {
    string id;
    string nome;
    JogoTipo jogo;
    int poder;
    int resistencia;
    vector<string> tipos;
    float rating;

    Carta(string i, string n, JogoTipo j, int p, int r, vector<string> t, float rt)
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

    size_t hash(const string& id, int tentativa = 0, int cap = -1) const {
        if (cap == -1) cap = capacidade;
        
        size_t h = 0;
        for (char c : id) {
            h = h * 31 + c;
        }
        
        size_t h2 = 1 + (h % (capacidade - 1));
        
        return (h + tentativa * h2) % capacidade;
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
        cout << prefix << (isLeft ? "└── " : "┌── ") << node->carta->nome << " (ID: " << node->carta->id << ", Rating: " << node->carta->rating << ")\n";
        if (node->esquerda)
            imprimirBST(node->esquerda, prefix + (isLeft ? "    " : "│   "), true);
    }

    void inserirBST(unique_ptr<NodeBST>& raiz, shared_ptr<Carta> carta) {
      if (!raiz) {
          raiz = make_unique<NodeBST>(carta);
      } else {
          if (carta->rating < raiz->carta->rating) {
              inserirBST(raiz->esquerda, carta);
          } else {
              inserirBST(raiz->direita, carta);
          }
      }
    }

    void montarBSTPorRating(float rating, bool maiorQue) {
      unique_ptr<NodeBST> raizBST;
      vector<shared_ptr<Carta>> cartasFiltradas;
      
      // 1. Fase de filtragem - percorre toda a tabela hash uma vez
      for (auto& bucket : tabelaHash) {
          for (auto& carta : bucket) {
              if ((maiorQue && carta->rating > rating) || (!maiorQue && carta->rating < rating)) {
                  cartasFiltradas.push_back(carta);
              }
          }
      }
      
      // 2. Fase de construção da BST
      for (auto& carta : cartasFiltradas) {
          inserirBST(raizBST, carta);
      }
      
      // 3. Exibição
      cout << "\nBST com " << cartasFiltradas.size() << " cartas com rating " 
           << (maiorQue ? ">" : "<") << " " << rating << ":\n";
      imprimirBST(raizBST);
    }

    int contarNosAVL(const unique_ptr<NodeAVL>& node) const {
        if (!node) return 0;
        return 1 + contarNosAVL(node->esquerda) + contarNosAVL(node->direita);
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    void carregarDaAPI(int quantidade) {
        CURL* curl = curl_easy_init();
        string readBuffer;
        if (curl) {
            int cartasPorPagina = min(250, quantidade);
            int paginasNecessarias = ceil((double)quantidade / cartasPorPagina);
            int cartasImportadas = 0;

            for (int pagina = 1; pagina <= paginasNecessarias && cartasImportadas < quantidade; pagina++) {
                string url = "https://api.pokemontcg.io/v2/cards?page=" + to_string(pagina) + 
                             "&pageSize=" + to_string(cartasPorPagina);
                
                readBuffer.clear();
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                
                struct curl_slist* headers = NULL;
                headers = curl_slist_append(headers, "X-Api-Key: 0712203e-2e15-4dfe-a64b-c78f73aeecbc");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
                
                CURLcode res = curl_easy_perform(curl);
                curl_slist_free_all(headers);

                if (res != CURLE_OK) {
                    cerr << "Erro ao buscar dados da API (página " << pagina << ")." << endl;
                    continue;
                }

                auto j = json::parse(readBuffer);
                for (const auto& item : j["data"]) {
                    if (cartasImportadas >= quantidade) break;

                    string id = item["id"];
                    string nome = item["name"];
                    
                    int poder = 0;
                    if (item.contains("hp") && item["hp"].is_string()) {
                        try {
                            poder = stoi(string(item["hp"]));
                        } catch (...) {
                            poder = 0;
                        }
                    }
                    
                    int resistencia = 0;
                    float rating = static_cast<float>(rand() % 1000) / 100.0f;
                    
                    auto carta = std::make_shared<Carta>(id, nome, JogoTipo::POKEMON, poder, resistencia, std::vector<std::string>{"Pokémon"}, rating);
                    adicionarCarta(carta);
                    cartasImportadas++;
                }
            }
            curl_easy_cleanup(curl);
            cout << "Foram carregadas " << cartasImportadas << " cartas da API.\n";
        }
    }

public:
    GerenciadorCartas(int cap = 100) : tamanho(0), capacidade(max(2, cap)) {
        tabelaHash.resize(capacidade);
    }

    // 1. Melhor verificação de duplicatas na inserção manual
    bool cartaExiste(const string& id) const {
        size_t indice = hash(id);
        for (const auto& c : tabelaHash[indice]) {
            if (c->id == id) return true;
        }
        return false;
    }

    int lerInteiro(const string& mensagem) {
        int valor;
        while (true) {
            cout << mensagem;
            cin >> valor;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Entrada inválida. Por favor, digite um número inteiro.\n";
            } else {
                cin.ignore();
                return valor;
            }
        }
    }

    float lerFloat(const string& mensagem) {
        float valor;
        while (true) {
            cout << mensagem;
            cin >> valor;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Entrada inválida. Por favor, digite um número.\n";
            } else {
                cin.ignore();
                return valor;
            }
        }
    }

    void inserirManual(const string& id, string nome, int poder, int resistencia, vector<string> tipos, float rating) {
        if (cartaExiste(id)) {
            cout << "Erro: Carta com ID " << id << " já existe!\n";
            return;
        }
        auto carta = std::make_shared<Carta>(id, nome, JogoTipo::POKEMON, poder, resistencia, tipos, rating);
        adicionarCarta(carta);
        cout << "Carta adicionada com sucesso!\n";
    }

    // 2. Melhor tratamento de erros no CSV
    void inserirPorCSV(const string& caminho, int limite) {
        ifstream arquivo(caminho);
        if (!arquivo.is_open()) {
            cerr << "Erro ao abrir arquivo: " << caminho << endl;
            return;
        }

        string linha;
        int contador = 0;
        int erros = 0;
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

            if (campos.size() < 5) {
                erros++;
                cerr << "Erro na linha " << (contador+erros+1) << ": Número insuficiente de campos (" << campos.size() << ")\n";
                continue;
            }

            try {
                string id = campos[0];
                if (cartaExiste(id)) {
                    cout << "Aviso: Carta com ID " << id << " já existe, pulando...\n";
                    continue;
                }

                string nome = campos[1];
                int poder = stoi(campos[2]);
                int resistencia = stoi(campos[3]);
                float rating = stof(campos[4]);
                vector<string> tipos = {"Pokémon"};

                if (rating < 0 || rating > 10) {
                    throw out_of_range("Rating deve estar entre 0 e 10");
                }

                inserirManual(id, nome, poder, resistencia, tipos, rating);
                contador++;
            } catch (const exception& e) {
                erros++;
                cerr << "Erro na linha " << (contador+erros+1) << ": " << e.what() << endl;
            }
        }
        cout << "Total de cartas carregadas: " << contador << endl;
        if (erros > 0) {
            cout << "Erros encontrados: " << erros << endl;
        }
    }

    void adicionarCarta(shared_ptr<Carta> carta) {
        float fatorCarga = static_cast<float>(tamanho + 1) / capacidade;
        if (fatorCarga >= LIMITE_CARGA) rehash();

        size_t indice = hash(carta->id);
        tabelaHash[indice].push_back(carta);
        tamanho++;
        raizAVL = inserirAVL(move(raizAVL), carta);
    }

    // 3. Métodos de busca
    shared_ptr<Carta> buscarPorId(const string& id) const {
        size_t indice = hash(id);
        for (const auto& c : tabelaHash[indice]) {
            if (c->id == id) return c;
        }
        return nullptr;
    }

    vector<shared_ptr<Carta>> buscarPorNome(const string& nome) const {
        vector<shared_ptr<Carta>> resultados;
        for (const auto& bucket : tabelaHash) {
            for (const auto& c : bucket) {
                if (c->nome.find(nome) != string::npos) {
                    resultados.push_back(c);
                }
            }
        }
        return resultados;
    }

    vector<shared_ptr<Carta>> buscarPorRating(float min, float max) const {
        vector<shared_ptr<Carta>> resultados;
        function<void(const unique_ptr<NodeAVL>&)> inOrder = [&](const unique_ptr<NodeAVL>& node) {
            if (!node) return;
            inOrder(node->esquerda);
            if (node->carta->rating >= min && node->carta->rating <= max) {
                resultados.push_back(node->carta);
            }
            inOrder(node->direita);
        };
        inOrder(raizAVL);
        return resultados;
    }

    void exibirResultadosBusca(const vector<shared_ptr<Carta>>& resultados) const {
        if (resultados.empty()) {
            cout << "Nenhuma carta encontrada.\n";
            return;
        }
        cout << "\n=== RESULTADOS DA BUSCA ===\n";
        for (const auto& c : resultados) {
            cout << "ID: " << c->id << " | Nome: " << c->nome 
                 << " | Poder: " << c->poder << " | Rating: " << c->rating << "\n";
        }
        cout << "Total encontrado: " << resultados.size() << "\n";
    }

    void exibirArvoreAVL() const {
        cout << "\n=== Estrutura da Árvore AVL (por ID) ===\n";
        function<void(const unique_ptr<NodeAVL>&, string, bool)> printAVL = 
            [&](const unique_ptr<NodeAVL>& node, const string& prefix, bool isLeft) {
                if (!node) return;
                if (node->direita)
                    printAVL(node->direita, prefix + (isLeft ? "│   " : "    "), false);
                cout << prefix << (isLeft ? "└── " : "┌── ") << node->carta->nome << " (ID: " << node->carta->id << ", Rating: " << node->carta->rating << ")\n";
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
                cout << "ID: " << node->carta->id << " | Nome: " << node->carta->nome << " | Rating: " << node->carta->rating << "\n";
                inOrder(node->direita);
            };
        inOrder(raizAVL);
    }

    void exibirTopCartas(int limite = 20) const {
        cout << "\n=== Top " << limite << " Cartas (Maior Rating) ===\n";
        
        priority_queue<
            shared_ptr<Carta>,
            vector<shared_ptr<Carta>>,
            function<bool(shared_ptr<Carta>, shared_ptr<Carta>)>
        > heap([](const auto& a, const auto& b) { return a->rating > b->rating; });
    
        function<void(const unique_ptr<NodeAVL>&)> coletar = [&](const unique_ptr<NodeAVL>& node) {
            if (!node) return;
            coletar(node->esquerda);
            heap.push(node->carta);
            if (heap.size() > limite) heap.pop();
            coletar(node->direita);
        };
        
        coletar(raizAVL);
    
        vector<shared_ptr<Carta>> topCartas;
        while (!heap.empty()) {
            topCartas.push_back(heap.top());
            heap.pop();
        }
        
        reverse(topCartas.begin(), topCartas.end());
    
        for (size_t i = 0; i < topCartas.size(); i++) {
            cout << i+1 << ". ID: " << topCartas[i]->id 
                 << " | Nome: " << topCartas[i]->nome
                 << " | Rating: " << topCartas[i]->rating << "\n";
        }
    }

    int contarNosAVL() const {
        return contarNosAVL(raizAVL);
    }

    void montarBSTDoUsuario() {
        float rating = lerFloat("Digite o rating base: ");
        int opcao;
        while (true) {
            cout << "Deseja cartas com rating (1) MAIOR ou (2) MENOR que " << rating << "? ";
            cin >> opcao;
            if (opcao == 1 || opcao == 2) break;
            cout << "Opção inválida. Digite 1 ou 2.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        montarBSTPorRating(rating, opcao == 1);
    }

    unique_ptr<NodeAVL> removerAVL(unique_ptr<NodeAVL> node, const string& id) {
        if (!node) return nullptr;
    
        if (id < node->carta->id) {
            node->esquerda = removerAVL(move(node->esquerda), id);
        } else if (id > node->carta->id) {
            node->direita = removerAVL(move(node->direita), id);
        } else {
            if (!node->esquerda || !node->direita) {
                node = move(node->esquerda ? node->esquerda : node->direita);
            } else {
                auto sucessor = node->direita.get();
                while (sucessor->esquerda) sucessor = sucessor->esquerda.get();
                node->carta = sucessor->carta;
                node->direita = removerAVL(move(node->direita), sucessor->carta->id);
            }
        }
    
        if (!node) return nullptr;
    
        node->altura = 1 + max(altura(node->esquerda), altura(node->direita));
        int balance = altura(node->esquerda) - altura(node->direita);
    
        if (balance > 1) {
            if (altura(node->esquerda->esquerda) >= altura(node->esquerda->direita)) {
                return rotacionarDireita(move(node));
            } else {
                node->esquerda = rotacionarEsquerda(move(node->esquerda));
                return rotacionarDireita(move(node));
            }
        }
        if (balance < -1) {
            if (altura(node->direita->direita) >= altura(node->direita->esquerda)) {
                return rotacionarEsquerda(move(node));
            } else {
                node->direita = rotacionarDireita(move(node->direita));
                return rotacionarEsquerda(move(node));
            }
        }
    
        return node;
    }

    // 4. Limpeza segura
    void limparSeguro() {
        // Limpa a tabela hash
        for (auto& bucket : tabelaHash) {
            bucket.clear();
        }
        tabelaHash.clear();
        
        // Limpa a árvore AVL
        raizAVL.reset();
        
        // Redefine o tamanho e capacidade
        tamanho = 0;
        capacidade = 100;
        tabelaHash.resize(capacidade);
        
        cout << "Todos os dados foram removidos de forma segura.\n";
    }

    void excluirPorId(const string& id) {
        size_t indice = hash(id);
        auto& bucket = tabelaHash[indice];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if ((*it)->id == id) {
                bucket.erase(it);
                tamanho--;
                raizAVL = removerAVL(move(raizAVL), id);
                cout << "Carta com ID " << id << " removida com sucesso!\n";
                return;
            }
        }
        cout << "Carta com ID " << id << " não encontrada.\n";
    }
    
    void destruirArvore() {
        limparSeguro();
        cout << "Todas as cartas foram removidas e a árvore foi reinicializada.\n";
    }
    
    void exibirEstatisticas() const {
        cout << "\n=== ESTATÍSTICAS AVANÇADAS ===\n";
        
        int total = contarNosAVL();
        cout << "Total de cartas: " << total << endl;
        cout << "Memória aproximada: " << (total * 128)/1024 << " KB\n";
        
        int colisoes = 0;
        int bucketMax = 0;
        int bucketsVazios = 0;
        
        for (const auto& bucket : tabelaHash) {
            if (bucket.empty()) bucketsVazios++;
            if (bucket.size() > 1) colisoes += bucket.size() - 1;
            if (bucket.size() > bucketMax) bucketMax = bucket.size();
        }
        
        cout << "\n=== TABELA HASH ===\n";
        cout << "Capacidade: " << capacidade << endl;
        cout << "Fator de carga: " << fixed << setprecision(2) 
             << (float)tamanho/capacidade << endl;
        cout << "Colisões totais: " << colisoes << endl;
        cout << "Buckets vazios: " << bucketsVazios << " (" 
             << (bucketsVazios*100.0f/capacidade) << "%)" << endl;
        cout << "Maior bucket: " << bucketMax << " elementos" << endl;
        
        if (raizAVL) {
            cout << "\n=== ÁRVORE AVL ===\n";
            cout << "Altura da árvore: " << altura(raizAVL) << endl;
            cout << "Altura teórica mínima: " << floor(log2(total)) + 1 << endl;
        }
    }

    void carregarDaAPIMenu() {
        int opcao;
        cout << "\nQuantas cartas deseja importar?\n";
        cout << "1. 1000 cartas\n";
        cout << "2. 5000 cartas\n";
        cout << "3. 10000 cartas\n";
        cout << "Escolha: ";
        
        while (true) {
            cin >> opcao;
            if (opcao >= 1 && opcao <= 3) break;
            cout << "Opção inválida. Digite 1, 2 ou 3: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        int quantidade;
        switch (opcao) {
            case 1: quantidade = 1000; break;
            case 2: quantidade = 5000; break;
            case 3: quantidade = 10000; break;
        }

        auto inicio = chrono::high_resolution_clock::now();
        carregarDaAPI(quantidade);
        auto fim = chrono::high_resolution_clock::now();
        
        auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
        cout << "Tempo de carregamento: " << duracao.count() << "ms\n";
    }

    void exportarParaCSV() {
        string caminhoArquivo = "cartas_exportadas.csv"; 
        ofstream arquivo(caminhoArquivo); 

        if (!arquivo.is_open()) {
            cerr << "Erro ao criar arquivo: " << caminhoArquivo << endl;
            return;
        }

        arquivo << "id,nome,poder,resistencia,rating,tipos\n";

        function<void(const unique_ptr<NodeAVL>&)> escreverCartas = 
            [&](const unique_ptr<NodeAVL>& node) {
                if (!node) return;
                escreverCartas(node->esquerda);
                
                arquivo << node->carta->id << ","
                        << node->carta->nome << ","
                        << node->carta->poder << ","
                        << node->carta->resistencia << ","
                        << node->carta->rating << ",\"";
                
                for (size_t i = 0; i < node->carta->tipos.size(); ++i) {
                    if (i != 0) arquivo << "|";
                    arquivo << node->carta->tipos[i];
                }
                arquivo << "\"\n";
                
                escreverCartas(node->direita);
            };

        escreverCartas(raizAVL);
        arquivo.close();
        cout << "Cartas exportadas com sucesso para: " << caminhoArquivo << endl;
    }
};

int main() {
    GerenciadorCartas gerenciador;
    int opcao;
    
    do {
        cout << "\n=== MENU PRINCIPAL ===\n";
        cout << "=== IMPORTAR CARTAS ===\n";
        cout << "1. Carregar Cartas da API\n";
        cout << "2. Carregar Cartas de Arquivo CSV\n";
        cout << "3. Inserir Carta Manualmente\n";
        cout << "\n=== EXIBIR CARTAS ===\n";
        cout << "4. Exibir Estrutura da Árvore AVL\n";
        cout << "5. Exibir Top 20 Cartas por Rating\n";
        cout << "6. Exibir Todas as Cartas\n";
        cout << "\n=== BUSCAR CARTAS ===\n";
        cout << "7. Buscar por ID\n";
        cout << "8. Buscar por Nome\n";
        cout << "9. Buscar por Rating\n";
        cout << "\n=== OUTRAS OPÇÕES ===\n";
        cout << "10. Verificar Quantidade de Cartas\n";
        cout << "11. Montar BST por Rating\n";
        cout << "12. Excluir Carta por ID\n";
        cout << "13. Destruir Árvore AVL\n";
        cout << "14. Exibir Estatísticas\n";
        cout << "15. Exportar cartas para CSV\n";
        cout << "0. Sair\n";
        cout << "Escolha: ";
        
        while (true) {
            cin >> opcao;
            if (opcao >= 0 && opcao <= 15) break;
            cout << "Opção inválida. Digite um número entre 0 e 15: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore();

        switch(opcao) {
            case 1: {
                gerenciador.carregarDaAPIMenu();
                break;
            }
            case 2: {
                string caminho = "./pokemons.csv";
                int limite = gerenciador.lerInteiro("\n--- Carregar CSV ---\nQuantidade a importar (500/1000/5000/10000): ");
                
                auto inicio = chrono::high_resolution_clock::now();
                gerenciador.inserirPorCSV(caminho, limite);
                auto fim = chrono::high_resolution_clock::now();
                
                auto duracao = chrono::duration_cast<chrono::milliseconds>(fim - inicio);
                cout << "Tempo de carregamento: " << duracao.count() << "ms\n";
                break;
            }
            case 3: {
                cout << "\n--- Inserção Manual ---\n";
                string id;
                cout << "ID: ";
                getline(cin, id);
                
                string nome;
                cout << "Nome: ";
                getline(cin, nome);
                
                int poder = gerenciador.lerInteiro("Poder: ");
                int resistencia = gerenciador.lerInteiro("Resistência: ");
                float rating = gerenciador.lerFloat("Rating (0-10): ");
                
                gerenciador.inserirManual(id, nome, poder, resistencia, {"Pokémon"}, rating);
                break;
            }
            case 4: {
                gerenciador.exibirArvoreAVL();
                break;
            }
            case 5: {
                gerenciador.exibirTopCartas();
                break;
            }
            case 6: {
                gerenciador.exibirArvoreAVLCompleta();
                break;
            }
            case 7: {
                string id;
                cout << "\n--- Busca por ID ---\nDigite o ID: ";
                getline(cin, id);
                auto carta = gerenciador.buscarPorId(id);
                if (carta) {
                    cout << "\nCarta encontrada:\n";
                    cout << "ID: " << carta->id << "\n";
                    cout << "Nome: " << carta->nome << "\n";
                    cout << "Poder: " << carta->poder << "\n";
                    cout << "Rating: " << carta->rating << "\n";
                } else {
                    cout << "Carta não encontrada.\n";
                }
                break;
            }
            case 8: {
                string nome;
                cout << "\n--- Busca por Nome ---\nDigite o nome (ou parte): ";
                getline(cin, nome);
                auto resultados = gerenciador.buscarPorNome(nome);
                gerenciador.exibirResultadosBusca(resultados);
                break;
            }
            case 9: {
                cout << "\n--- Busca por Rating ---\n";
                float min = gerenciador.lerFloat("Rating mínimo: ");
                float max = gerenciador.lerFloat("Rating máximo: ");
                auto resultados = gerenciador.buscarPorRating(min, max);
                gerenciador.exibirResultadosBusca(resultados);
                break;
            }
            case 10: {
                int total = gerenciador.contarNosAVL();
                cout << "\nTotal de cartas na estrutura: " << total << endl;
                if (total == 10000) {
                    cout << "✓ Todos os 10.000 registros foram carregados!\n";
                }
                break;
            }
            case 11: {
                gerenciador.montarBSTDoUsuario();
                break;
            }
            case 12: {
                string id;
                cout << "\n--- Exclusão ---\nID da carta a remover: ";
                getline(cin, id);
                gerenciador.excluirPorId(id);
                break;
            }
            case 13: {
                gerenciador.destruirArvore();
                cout << "Árvore AVL reinicializada.\n";
                break;
            }
            case 14: {
                gerenciador.exibirEstatisticas();
                break;
            }
            case 15: {
                gerenciador.exportarParaCSV();
                break;
            }
            case 0: {
                cout << "Encerrando o programa...\n";
                break;
            }
        }
    } while (opcao != 0);

    return 0;
}
