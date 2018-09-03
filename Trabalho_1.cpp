#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

// Line feed
#define SEPARADOR_REGISTRO static_cast<char>(10) 

// Carriage return
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
    // Ignorar \n
    cin.ignore();

    // Adicionar .dat no nome do arquivo
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

    // Caso não seja possível abrir o arquivo
    if (!arquivo_blocos.is_open()) {
        cerr << "Impossivel abrir " << nome_arquivo << " para escrita" << endl;
        return false;
    }

    for (int i = 0; i < num_registros; ++i) {
        string aux, registro;
        int tamanho_registro;
        // Campos : RA,Nome,Sigla,Ingresso
            for (int j = 0; j < 4; ++j) {
            getline(cin, aux);
            registro.append(aux);
            // Adicionar o separador no final do campo
            registro.push_back(SEPARADOR_CAMPO);
        }
        // Adicionar o separador no final do registro
        registro.push_back(SEPARADOR_REGISTRO);

        tamanho_registro = registro.length();

        if (tamanho_registro + bytes_bloco > 510) {

            // Insere o tamanho do bloco nos ultimos 2 bytes do bloco
            inteiro_para_little_endian_de_2_bytes(bytes_bloco, buffer_bloco, 510);

            // Escreve o bloco no arquivo
            arquivo_blocos.write(buffer_bloco, 512);

            // Reinicia o bloco
            bytes_bloco = 0;
        }

        for (int j = 0; j < tamanho_registro; ++j)
            buffer_bloco[bytes_bloco + j] = registro[j];
        // Adicionar o tamanho do registro ao tamanho do bloco
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
    // Caso não seja possível abrir o arquivo
    if (!arquivo_blocos.is_open()) {
        cerr << "Impossivel abrir " << nome_arquivo << " para leitura" << endl;
        return false;
    }

    while (cin >> RA_alvo && RA_alvo != "0") {
        stringstream buffer;
        // Lendo o bloco
        while (!encontrou && arquivo_blocos.read(bloco, 512)) {
            encontrou = false;

            stringstream stream_bloco(remover_fragmentacao_externa(bloco));
            string registro;
            // Lendo o registro
            while (!encontrou && getline(stream_bloco, registro, SEPARADOR_REGISTRO)) {
                stringstream stream_registro(registro);
                string campo;
                int k = 0;
                // Lendo o campo
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
        // clear e seekg juntos servem para voltar para o ínicio do arquivo
        arquivo_blocos.clear();
        arquivo_blocos.seekg(0, fstream::beg);

        encontrou = false;
    }

    arquivo_blocos.close();

    return true;
}
// Os valores que eu consigo representar em um byte vai de 0 a 255, por isso o byte mais significativo é dividido por 256 e o menos significativo é o resto

void inteiro_para_little_endian_de_2_bytes(int x, char *saida, int posicao_inicio) {
    saida[posicao_inicio] = x % 256;
    saida[posicao_inicio + 1] = x / 256;
}

int little_endian_de_2_bytes_para_inteiro(char *entrada, int posicao_inicio) {
    // static_cast<unsigned char> serve para transformar um char com sinal em sem sinal
    return static_cast<unsigned char>(entrada[posicao_inicio + 1]) * 256 + static_cast<unsigned char>(entrada[posicao_inicio]);
}

string remover_fragmentacao_externa(char *bloco) {
    int tamanho_valido = little_endian_de_2_bytes_para_inteiro(bloco, 510);
    string aux(bloco);

    return aux.substr(0, tamanho_valido);
}
