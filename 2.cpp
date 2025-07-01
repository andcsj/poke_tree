#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

enum class JogoTipo { POKEMON };

struct Carta {
    string id;
    string nome;
    int poder;
    int resistencia;
    float rating;

    Carta(string id, string nome, int poder, int resistencia, float rating)
        : id(id), nome(nome), poder(poder), resistencia(resistencia), rating(rating) {}

    void imprimir() const {
        cout << fixed << setprecision(2);
        cout << "ID: " << id << ", Nome: " << nome 
             << ", Poder: " << poder << ", Resistência: " << resistencia 
             << ", Rating: " << rating << endl;
    }
};

vector<shared_ptr<Carta>> cartas;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void inserirCarta(shared_ptr<Carta> carta) {
    cartas.push_back(move(carta));
}

void carregarDoCSV(const string& caminho, int quantidade = -1) {
    ifstream file(caminho);
    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo CSV." << endl;
        return;
    }

    string linha;
    getline(file, linha); // Cabeçalho
    int contador = 0;
    
    while (getline(file, linha) && (quantidade == -1 || contador < quantidade)) {
        istringstream ss(linha);
        string id, nome, poderStr, resistenciaStr, ratingStr;

        getline(ss, id, ',');
        getline(ss, nome, ',');
        getline(ss, poderStr, ',');
        getline(ss, resistenciaStr, ',');
        getline(ss, ratingStr, ',');

        int poder = stoi(poderStr);
        int resistencia = stoi(resistenciaStr);
        float rating = stof(ratingStr);

        auto carta = make_shared<Carta>(id, nome, poder, resistencia, rating);
        inserirCarta(carta);
        contador++;
    }
    cout << "Foram carregadas " << contador << " cartas do CSV.\n";
}

void carregarDaAPI(int quantidade = 1000) {
    CURL* curl = curl_easy_init();
    string readBuffer;
    if (curl) {
        // Calcula quantas páginas precisamos para obter a quantidade desejada
        // (a API tem um limite máximo de 250 cartas por página)
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
                
                auto carta = make_shared<Carta>(id, nome, poder, resistencia, rating);
                inserirCarta(carta);
                cartasImportadas++;
            }
        }
        curl_easy_cleanup(curl);
        cout << "Foram carregadas " << cartasImportadas << " cartas da API.\n";
    }
}

void menuQuantidade(const string& origem) {
    int opcao;
    cout << "\nQuantas cartas deseja importar?\n";
    cout << "1. 1000 cartas\n";
    cout << "2. 5000 cartas\n";
    cout << "3. 10000 cartas\n";
    cout << "Escolha: ";
    cin >> opcao;

    int quantidade;
    switch (opcao) {
        case 1: quantidade = 1000; break;
        case 2: quantidade = 5000; break;
        case 3: quantidade = 10000; break;
        default: 
            cout << "Opção inválida. Usando 1000 cartas.\n";
            quantidade = 1000;
    }

    if (origem == "API") {
        carregarDaAPI(quantidade);
    } else {
        carregarDoCSV("pokemons.csv", quantidade);
    }
}

void menuPrincipal() {
    int opcao;
    do {
        cout << "\n==== MENU PRINCIPAL ====\n";
        cout << "Cartas importadas: " << cartas.size() << "\n\n";
        cout << "1. Importar cartas da API\n";
        cout << "2. Importar cartas do CSV\n";
        cout << "3. Imprimir todas as cartas\n";
        cout << "4. Buscar carta por ID\n";
        cout << "5. Inserir carta manualmente\n";
        cout << "6. Remover carta por ID\n";
        cout << "0. Sair\n";
        cout << "Escolha: ";
        cin >> opcao;

        switch (opcao) {
            case 1:
                menuQuantidade("API");
                break;
            case 2:
                menuQuantidade("CSV");
                break;
            case 3:
                for (const auto& carta : cartas) {
                    carta->imprimir();
                }
                break;
            case 4: {
                string id;
                cout << "ID da carta: ";
                cin >> id;
                bool encontrada = false;
                for (const auto& carta : cartas) {
                    if (carta->id == id) {
                        carta->imprimir();
                        encontrada = true;
                        break;
                    }
                }
                if (!encontrada) {
                    cout << "Carta não encontrada.\n";
                }
                break;
            }
            case 5: {
                string id, nome;
                int poder, resistencia;
                float rating;
                
                cout << "ID: "; cin >> id;
                cout << "Nome: "; cin.ignore(); getline(cin, nome);
                cout << "Poder: "; cin >> poder;
                cout << "Resistência: "; cin >> resistencia;
                cout << "Rating: "; cin >> rating;
                
                auto carta = make_shared<Carta>(id, nome, poder, resistencia, rating);
                inserirCarta(carta);
                cout << "Carta adicionada.\n";
                break;
            }
            case 6: {
                string id;
                cout << "ID da carta a remover: ";
                cin >> id;
                auto it = remove_if(cartas.begin(), cartas.end(), 
                    [&](const shared_ptr<Carta>& c) { return c->id == id; });
                if (it != cartas.end()) {
                    cartas.erase(it, cartas.end());
                    cout << "Carta removida.\n";
                } else {
                    cout << "Carta não encontrada.\n";
                }
                break;
            }
            case 0:
                cout << "Saindo...\n";
                break;
            default:
                cout << "Opção inválida!\n";
        }
    } while (opcao != 0);
}

int main() {
    menuPrincipal();
    return 0;
}
