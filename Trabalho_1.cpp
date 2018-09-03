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
        string aux, registro;
        int tamanho_registro;

        for (int j = 0; j < 4; ++j) {
            getline(cin, aux);
            registro.append(aux);
            registro.push_back(SEPARADOR_CAMPO);
        }

        registro.push_back(SEPARADOR_REGISTRO);
        
        tamanho_registro = registro.length();

        if (tamanho_registro + bytes_bloco > 510) {
            inteiro_para_little_endian_de_2_bytes(bytes_bloco, buffer_bloco, 510);

            arquivo_blocos.write(buffer_bloco, 512);

            bytes_bloco = 0;
        }

        for (int j = 0; j < tamanho_registro; ++j)
            buffer_bloco[bytes_bloco + j] = registro[j];

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
        stringstream buffer;
        
        while (!encontrou && arquivo_blocos.read(bloco, 512)) {
            encontrou = false;

            stringstream stream_bloco(remover_fragmentacao_externa(bloco));
            string registro;

            while (!encontrou && getline(stream_bloco, registro, SEPARADOR_REGISTRO)) {
                stringstream stream_registro(registro);
                string campo;
                int k = 0;

                while (getline(stream_registro, campo, SEPARADOR_CAMPO) && (encontrou || (!encontrou && campo == RA_alvo))) {
                    if (!encontrou)
                        encontrou = true;

                    buffer << campo;
                    
                    if (++k < 4)
                        buffer << ':';
                }
            }
        }

        if (!encontrou)
            cout << '*' << endl;
        else 
            cout << buffer.str() << endl;

        arquivo_blocos.clear();
        arquivo_blocos.seekg(0, fstream::beg);

        encontrou = false;
    }

    arquivo_blocos.close();

    return true;
}

void inteiro_para_little_endian_de_2_bytes(int x, char *saida, int posicao_inicio) {
    saida[posicao_inicio] = x % 256;
    saida[posicao_inicio + 1] = x / 256;
}

int little_endian_de_2_bytes_para_inteiro(char *entrada, int posicao_inicio) {
    return static_cast<unsigned char>(entrada[posicao_inicio + 1]) * 256 + static_cast<unsigned char>(entrada[posicao_inicio]);
}

string remover_fragmentacao_externa(char *bloco) {
    int tamanho_valido = little_endian_de_2_bytes_para_inteiro(bloco, 510);
    string aux(bloco);

    return aux.substr(0, tamanho_valido);
}
