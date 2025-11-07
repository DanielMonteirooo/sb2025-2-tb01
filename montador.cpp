#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>
#include <stdexcept>

using namespace std;

// --- Estruturas de Dados ---

struct OpcodeInfo {
    int codigo;
    int operandos;
    int tamanho;
};

struct SimboloInfo {
    string nome;
    int endereco;
    bool definido;
    int lista_pendencias;
    vector<int> linhas_referencia;

    SimboloInfo(string n = "") : nome(n), endereco(-1), definido(false), lista_pendencias(-1) {}
};

// --- Tabela de Instruções ---
const map<string, OpcodeInfo> TabelaOpcodes = {
    {"ADD",    {1, 1, 2}},
    {"SUB",    {2, 1, 2}},
    {"MULT",   {3, 1, 2}},
    {"DIV",    {4, 1, 2}},
    {"JMP",    {5, 1, 2}},
    {"JMPN",   {6, 1, 2}},
    {"JMPP",   {7, 1, 2}},
    {"JMPZ",   {8, 1, 2}},
    {"COPY",   {9, 2, 3}},
    {"LOAD",   {10, 1, 2}},
    {"STORE",  {11, 1, 2}},
    {"INPUT",  {12, 1, 2}},
    {"OUTPUT", {13, 1, 2}},
    {"STOP",   {14, 0, 1}}
};

// --- Classe Montador ---

class Montador {
public:
    Montador(const string &inname_base);
    bool montar();

private:
    string inname_pre;
    string outname_o1;
    string outname_o2;
    vector<int> codigoObjeto;
    map<string, SimboloInfo> tabelaSimbolos;
    map<int, string> errosPorLinha;
    map<int, int> pendenciaOffsets;
    bool has_errors;

    string normalizarToken(string s);
    bool ehLabelValido(const string& label);
    bool parseOperando(const string& op, string& label, int& offset);
    void adicionarErro(int linha, const string& tipo_erro, const string& detalhe = "");

    bool passagemUnica();
    void processarLabel(const string& label, int LC, int numeroLinha);
    void processarOperando(const string& operando, int enderecoOperando, int numeroLinha);
    void resolverPendenciasFinais();
    void verificarSimbolosNaoDefinidos();
    void escreverArquivoO1();
    void escreverArquivoO2();
    void reportarErros();
};

// --- Implementações ---

Montador::Montador(const string &inname_base) : 
    inname_pre(inname_base + ".pre"),
    outname_o1(inname_base + ".o1"),
    outname_o2(inname_base + ".o2"),
    has_errors(false) {}

bool Montador::montar() {
    if (!passagemUnica()) {
        cerr << "Erro durante a passagem única." << endl;
        reportarErros();
        return false;
    }

    escreverArquivoO1();
    resolverPendenciasFinais();
    verificarSimbolosNaoDefinidos();

    if (codigoObjeto.empty() && !has_errors) {
        adicionarErro(0, "SINTATICO", "Nenhum código gerado (arquivo vazio).");
    }

    if (!has_errors) {
        escreverArquivoO2();
    } else {
        cerr << "Erros encontrados durante a montagem. Arquivo .o2 não gerado." << endl;
        remove(outname_o2.c_str());
    }

    reportarErros();
    return !has_errors;
}

// --- Funções Auxiliares ---

string Montador::normalizarToken(string s) {
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

bool Montador::ehLabelValido(const string& label) {
    if (label.empty() || isdigit(label[0])) return false;
    for (char c : label)
        if (!isalnum(c) && c != '_') return false;
    if (TabelaOpcodes.count(label) || label == "CONST" || label == "SPACE")
        return false;
    return true;
}

bool Montador::parseOperando(const string& op, string& label, int& offset) {
    size_t posPlus = op.find('+');
    if (posPlus == string::npos) {
        label = op;
        offset = 0;
        return true;
    }

    label = op.substr(0, posPlus);
    string offset_str = op.substr(posPlus + 1);
    if (label.empty() || offset_str.empty()) return false;

    for (char c : offset_str)
        if (!isdigit(c)) return false;
    offset = stoi(offset_str);
    return true;
}

void Montador::adicionarErro(int linha, const string& tipo_erro, const string& detalhe) {
    if (errosPorLinha.find(linha) == errosPorLinha.end()) {
        string msg = "ERRO " + tipo_erro + ": " + (detalhe.empty() ? "Erro não especificado." : detalhe);
        errosPorLinha[linha] = msg;
        has_errors = true;
    }
}

// --- Passagem Única ---

bool Montador::passagemUnica() {
    ifstream infile(inname_pre);
    if (!infile.is_open()) {
        cerr << "Erro ao abrir arquivo pré-processado: " << inname_pre << endl;
        return false;
    }

    string linha;
    int numeroLinha = 0;
    int LC = 0;

    while (getline(infile, linha)) {
        numeroLinha++;
        size_t posComentario = linha.find(';');
        if (posComentario != string::npos)
            linha = linha.substr(0, posComentario);

        transform(linha.begin(), linha.end(), linha.begin(), ::toupper);
        stringstream ss(linha);
        string token;
        int labelsNestaLinha = 0;

        while (ss >> token) {
            if (token[0] == ';') break;

            if (token.back() == ':') {
                labelsNestaLinha++;
                if (labelsNestaLinha > 1)
                    adicionarErro(numeroLinha, "SINTATICO", "Dois rótulos na mesma linha.");
                processarLabel(token.substr(0, token.size() - 1), LC, numeroLinha);
            } 
            else if (TabelaOpcodes.count(token)) {
                const auto& opcodeInfo = TabelaOpcodes.at(token);
                codigoObjeto.push_back(opcodeInfo.codigo);

                vector<string> operandos;
                string opToken;
                for(int i = 0; i < opcodeInfo.operandos; ++i) {
                    if (!(ss >> opToken)) {
                        adicionarErro(numeroLinha, "SINTATICO", "Número de operandos inválido para " + token + ".");
                        break;
                    }
                    opToken.erase(remove(opToken.begin(), opToken.end(), ','), opToken.end());
                    operandos.push_back(opToken);
                }

                for (size_t i = 0; i < operandos.size(); ++i)
                    processarOperando(operandos[i], LC + i + 1, numeroLinha);

                LC += opcodeInfo.tamanho;
                break;
            } 
            else if (token == "CONST") {
                string valor;
                if (!(ss >> valor))
                    adicionarErro(numeroLinha, "SINTATICO", "CONST requer 1 operando numérico.");
                else {
                    try {
                        codigoObjeto.push_back(stoi(valor));
                        LC++;
                    } catch (...) {
                        adicionarErro(numeroLinha, "SINTATICO", "Valor inválido para CONST.");
                    }
                }
                break;
            } 
            else if (token == "SPACE") {
                string valor;
                int tamanho = 1;
                if (ss >> valor) {
                    try {
                        tamanho = stoi(valor);
                        if (tamanho < 0) throw runtime_error("negativo");
                    } catch (...) {
                        adicionarErro(numeroLinha, "SINTATICO", "Argumento inválido para SPACE.");
                        tamanho = 0;
                    }
                }
                for (int i = 0; i < tamanho; ++i)
                    codigoObjeto.push_back(0);
                LC += tamanho;
                break;
            } 
            else {
                adicionarErro(numeroLinha, "SINTATICO", "Instrução desconhecida: " + token);
                break;
            }
        }
    }
    infile.close();
    return true;
}

// --- Manipulação de Símbolos e Pendências ---

void Montador::processarLabel(const string& label, int LC, int numeroLinha) {
    if (!ehLabelValido(label)) {
        adicionarErro(numeroLinha, "LEXICO", "Rótulo inválido: " + label);
        return;
    }

    if (tabelaSimbolos.count(label)) {
        auto& simbolo = tabelaSimbolos.at(label);
        if (simbolo.definido)
            adicionarErro(numeroLinha, "SEMANTICO", "Símbolo redefinido: " + label);
        else {
            simbolo.definido = true;
            simbolo.endereco = LC;
        }
    } else {
        SimboloInfo novo(label);
        novo.definido = true;
        novo.endereco = LC;
        tabelaSimbolos.insert({label, novo});
    }
}

void Montador::processarOperando(const string& operando, int enderecoOperando, int numeroLinha) {
    string labelName;
    int offset;
    if (!parseOperando(operando, labelName, offset)) {
        adicionarErro(numeroLinha, "SINTATICO", "Operando inválido: " + operando);
        codigoObjeto.push_back(-999);
        return;
    }
    if (!ehLabelValido(labelName)) {
        adicionarErro(numeroLinha, "LEXICO", "Rótulo inválido: " + labelName);
        codigoObjeto.push_back(-999);
        return;
    }

    if (tabelaSimbolos.count(labelName)) {
        auto& simbolo = tabelaSimbolos.at(labelName);
        if (simbolo.definido)
            codigoObjeto.push_back(simbolo.endereco + offset);
        else {
            codigoObjeto.push_back(simbolo.lista_pendencias);
            simbolo.lista_pendencias = enderecoOperando;
            simbolo.linhas_referencia.push_back(numeroLinha);
            if (offset) pendenciaOffsets[enderecoOperando] = offset;
        }
    } else {
        SimboloInfo novo(labelName);
        novo.lista_pendencias = enderecoOperando;
        novo.linhas_referencia.push_back(numeroLinha);
        tabelaSimbolos.insert({labelName, novo});
        codigoObjeto.push_back(-1);
        if (offset) pendenciaOffsets[enderecoOperando] = offset;
    }
}

void Montador::resolverPendenciasFinais() {
    for (auto const& [nome, simbolo] : tabelaSimbolos) {
        if (!simbolo.definido) continue;
        int endereco_pendente = simbolo.lista_pendencias;
        while (endereco_pendente != -1) {
            int proximo = codigoObjeto[endereco_pendente];
            int offset = pendenciaOffsets.count(endereco_pendente) ? pendenciaOffsets[endereco_pendente] : 0;
            codigoObjeto[endereco_pendente] = simbolo.endereco + offset;
            endereco_pendente = proximo;
        }
    }
}

void Montador::verificarSimbolosNaoDefinidos() {
    for (auto const& [nome, simbolo] : tabelaSimbolos) {
        if (!simbolo.definido)
            for (int linha : simbolo.linhas_referencia)
                adicionarErro(linha, "SEMANTICO", "Símbolo não definido: " + nome);
    }
}

// --- Escrita e Relatórios ---

void Montador::escreverArquivoO1() {
    ofstream out(outname_o1);
    for (size_t i = 0; i < codigoObjeto.size(); ++i)
        out << codigoObjeto[i] << (i == codigoObjeto.size() - 1 ? "" : " ");
    out << endl;
}

void Montador::escreverArquivoO2() {
    ofstream out(outname_o2);
    for (size_t i = 0; i < codigoObjeto.size(); ++i)
        out << codigoObjeto[i] << (i == codigoObjeto.size() - 1 ? "" : " ");
    out << endl;
}

void Montador::reportarErros() {
    if (errosPorLinha.empty()) return;

    ifstream in(inname_pre);
    vector<string> linhas;
    string l;
    while (getline(in, l)) linhas.push_back(l);
    in.close();

    for (const auto& [n, msg] : errosPorLinha) {
        if (n > 0 && n <= (int)linhas.size())
            linhas[n - 1] += " ; " + msg;
        else if (n == 0)
            linhas.push_back("; " + msg);
    }

    ofstream out(inname_pre);
    for (const string& linha : linhas) out << linha << endl;
    cout << "Erros adicionados ao arquivo " << inname_pre << endl;
}

// --- Main ---

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <nome_base_arquivo>\n";
        return 1;
    }

    Montador montador(argv[1]);
    if (montador.montar()) {
        cout << "Montagem concluída com sucesso.\n";
        return 0;
    } else {
        cout << "Montagem concluída com erros.\n";
        return 1;
    }
}
