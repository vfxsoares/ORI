#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

#define SEPARADOR_REGISTRO static_cast<char>(10)
#define SEPARADOR_CAMPO static_cast<char>(13)

using namespace std;

bool buscar(const string &nome_arquivo);
bool escrever(const string &nome_arquivo, int num_registros);
int little_endian_de_2_bytes_para_inteiro(char *entrada, int posicao_inicio);
void inteiro_para_little_endian_de_2_bytes(int x, char *saida, int posicao_inicio);
string dados_para_registro(string dados[4]);
string remover_fragmentacao_externa(char *bloco);

int main() {
    int num_registros;
    string nome_arquivo;

    cin >> nome_arquivo >> num_registros;
    cin.ignore();

    nome_arquivo.append(".dat");

    if (escrever(nome_arquivo, num_registros))
        buscar(nome_arquivo);

    return 0;
}

bool escrever(const string &nome_arquivo, int num_registros) {
    int bytes_bloco = 0;
    char buffer_bloco[512];
    fstream arquivo_blocos;

    arquivo_blocos.open(nome_arquivo.c_str(), ofstream::out | ofstream::binary);

    if (!arquivo_blocos.is_open()) {
        cerr << "Impossivel abrir " << nome_arquivo << " para escrita" << endl;
        return false;
    }

    for (int i = 0; i < num_registros; ++i) {
        string dados[4], registro;
        int tamanho_registro;

        for (int j = 0; j < 4; ++j)
            getline(cin, dados[j]);

        registro = dados_para_registro(dados);
        tamanho_registro = registro.length();

        if (tamanho_registro + bytes_bloco > 510) {
            inteiro_para_little_endian_de_2_bytes(bytes_bloco, buffer_bloco, 510);

            arquivo_blocos.write(buffer_bloco, 512);

            bytes_bloco = 0;
        }

        for (int j = 0, k = 0; j < tamanho_registro; ++j, ++k)
            buffer_bloco[bytes_bloco + j] = registro[k];

        bytes_bloco += tamanho_registro;
    }

    inteiro_para_little_endian_de_2_bytes(bytes_bloco, buffer_bloco, 510);

    arquivo_blocos.write(buffer_bloco, 512);

    arquivo_blocos.close();

    return true;
}

bool buscar(const string &nome_arquivo) {
    bool encontrou = false;
    char bloco[512];
    string RA_alvo;
    fstream arquivo_blocos;

    arquivo_blocos.open(nome_arquivo.c_str(), ofstream::in | ofstream::binary);

    if (!arquivo_blocos.is_open()) {
        cerr << "Impossivel abrir " << nome_arquivo << " para leitura" << endl;
        return false;
    }

    while (cin >> RA_alvo && RA_alvo != "0") {
        while (!encontrou && arquivo_blocos.read(bloco, 512)) {
            encontrou = false;

            stringstream aux(remover_fragmentacao_externa(bloco));
            string token;

            while (!encontrou && getline(aux, token, SEPARADOR_REGISTRO)) {
                stringstream aux2(token), buffer;
                string token2;
                int k = 0;

                while (getline(aux2, token2, SEPARADOR_CAMPO)) {
                    if (!encontrou) {
                        if (token2 != RA_alvo)
                            break;
                        else
                            encontrou = true;
                    }

                    buffer << token2;
                    if (++k < 4)
                        buffer << ':';
                }

                cout << buffer.str();
            }
        }

        if (!encontrou)
            cout << '*';

        cout << endl;

        arquivo_blocos.clear();
        arquivo_blocos.seekg(0, fstream::beg);

        encontrou = false;
    }

    arquivo_blocos.close();

    return true;
}

string dados_para_registro(string dados[4]) {
    stringstream construtor_de_registro;

    for (int i = 0; i < 4; ++i)
        construtor_de_registro << dados[i] << SEPARADOR_CAMPO;

    construtor_de_registro << SEPARADOR_REGISTRO;

    return construtor_de_registro.str();
}

void inteiro_para_little_endian_de_2_bytes(int x, char *saida, int posicao_inicio) {
    stringstream aux;
    string k;

    aux << setfill('0') << setw(4) << hex << x;

    k = aux.str();

    saida[posicao_inicio] = strtol(k.substr(2, 2).c_str(), NULL, 16);
    saida[posicao_inicio + 1] = strtol(k.substr(0, 2).c_str(), NULL, 16);
}

int little_endian_de_2_bytes_para_inteiro(char *entrada, int posicao_inicio) {
    return static_cast<unsigned char>(entrada[posicao_inicio + 1]) * 256 + static_cast<unsigned char>(entrada[posicao_inicio]);
}

string remover_fragmentacao_externa(char *bloco) {
    int tamanho_valido = little_endian_de_2_bytes_para_inteiro(bloco, 510);
    string aux(bloco);

    return aux.substr(0, tamanho_valido);
}
