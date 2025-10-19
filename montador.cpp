#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>
#include <regex>
using namespace std;

class Opcode
{
private:
    string nome;
    int codigo;
    int argumentos;
public:
    Opcode(const string &nome, int codigo, int argumentos)
        : nome(nome), codigo(codigo), argumentos(argumentos) {}
    string getNome() const { return nome; }
    int getCodigo() const { return codigo; }
    int getArgumentos() const { return argumentos; }
};

class Simbolo
{
private:
    string simbolo;
    int endereco;
    bool definido;
    vector<int> pendencias;
    vector<int> referencias; // Linhas onde o símbolo é referenciado (para erros)
public:
    Simbolo(const string &simbolo)
        : simbolo(simbolo), endereco(-1), definido(false) {}

    string getSimbolo() const { return simbolo; }

    int getEndereco() const { return endereco; }

    bool ehDefinido() const { return definido; }

    void definir(int endereco)
    {
        this->endereco = endereco;
        this->definido = true;
    }

    void adicionarPendencia(int pendencia, int linha)
    {
        pendencias.push_back(pendencia);
        referencias.push_back(linha);
    }

    const vector<int>& getPendencias() const
    {
        return pendencias;
    }

    const vector<int>& getReferencias() const
    {
        return referencias;
    }
};

struct CodigoTresEnderecos
{
    int endereco;
    int opcode;
    int operando1;
    int operando2;
};

// - Rotulo declarado duas vezes em lugares diferentes ✅
// - Dois rótulos na mesma linha
// - Rotulo não declarado ✅
// - Instrução com número de parâmetros errado ✅
// - Instução inexistente ✅
// - Erros léxicos (label  não pode começar por número e o único caracter especial que pode ter é o “_”). ✅
const map<string, string> erros = {
    {"rotulo_duplicado", "Rótulo declarado duas vezes em lugares diferentes"},
    {"dois_rotulos", "Dois rótulos na mesma linha"},
    {"rotulo_nao_declarado", "Rótulo não declarado"},
    {"parametros_errados", "Instrução com número de parâmetros errado"},
    {"instrucao_inexistente", "Instrução inexistente"},
    {"erro_lexico", "Erro léxico: rótulo inválido"}
};

class Montador
{
public:
    Montador(const string &inname, const string &outname)
        : inname(inname), outname(outname) {}
    bool processa(bool resolverPendencias = true)
    {
        this->resolverPendencias = resolverPendencias;
        this->abreArquivos();
        this->montagem();
        this->validaTabelaDeSimbolos();
        this->escreveCodigoObjeto();
        this->limpeza();
        return 0;
    }
private:
    string inname;
    string outname;
    ifstream infile;
    ofstream outfile;
    bool resolverPendencias;

    vector<int> codigoObjeto;
    #define PROXIMO_ENDERECO codigoObjeto.size()

    map<string, Simbolo> tabelaSimbolos;
    const map<string, Opcode> tabelaOpCodes = {
        {"ADD", Opcode("ADD", 1, 1)},
        {"SUB", Opcode("SUB", 2, 1)},
        {"MULT", Opcode("MULT", 3, 1)},
        {"DIV", Opcode("DIV", 4, 1)},
        {"JMP", Opcode("JMP", 5, 1)},
        {"JMPN", Opcode("JMPN", 6, 1)},
        {"JMPP", Opcode("JMPP", 7, 1)},
        {"JMPZ", Opcode("JMPZ", 8, 1)},
        {"COPY", Opcode("COPY", 9, 2)},
        {"LOAD", Opcode("LOAD", 10, 1)},
        {"STORE", Opcode("STORE", 11, 1)},
        {"INPUT", Opcode("INPUT", 12, 1)},
        {"OUTPUT", Opcode("OUTPUT", 13, 1)},
        {"STOP", Opcode("STOP", 14, 0)}
    };

    void abreArquivos()
    {
        string tempOutname = outname; // Permite a rechamada do método na mesma instância
        resolverPendencias ? tempOutname += ".o2" : tempOutname += ".o1";

        infile.open(inname);
        if (!infile.is_open())
        {
            cerr << "Erro ao abrir arquivo de entrada: " << inname << endl;
            return;
        }
        outfile.open(tempOutname);
        if (!outfile.is_open())
        {
            cerr << "Erro ao abrir arquivo de saída: " << tempOutname << endl;
            return;
        }
    }

    /*
        Fazer o algoritmo de passagem única com a lista de pendencias feita no próprio código. E mostrar o código inteiro com as listas de pendencias SEM corrigir as as pendencias na saída .o1. A saída final do código compilado e no arquivo .o2.

        A sáida deve ser em uma única linha SEM enter e com espaços, os espaços reservados com SPACE devem ser colocados como 0 (não xx) do tipo:
        5 9 8 12 2 12 3 12 1 12 14 0.

        Deve ser capaz de ignorar qualquer espaço, enter ou tabulação desnecessária. Deve ser capaz de aceitar enter depois de um rotulo do tipo:
        ROT:
        ADD N1

        O compilador deve aceitar maiúsculas e minúsculas. Diretivas SPACE e CONST com argumentos. Deve indicar erros, marcando a linha no arquivo .pre e o tipo (sintático, semântico ou léxico).
     */
    void montagem()
    {
        string linha;
        int numeroLinha = 0; // usado para adiconar erros na linha correta
        while (getline(infile, linha))
        {
            numeroLinha++;

            if (linha.empty() || linha[0] == ';') // Ignora linhas vazias e comentadas
                continue;
                
            string palavra;
            istringstream iss(linha);
            while (iss >> palavra)
            {
                // Remove espaços em branco
                palavra.erase(remove_if(palavra.begin(), palavra.end(), ::isspace), palavra.end());
                transform(palavra.begin(), palavra.end(), palavra.begin(), ::toupper);

                // se comentário, limpa a linha
                if (palavra[0] == ';')
                {
                    linha.clear();
                    break;
                }

                // É label?
                if (ehDefinicaoDeLabel(palavra))
                {
                    if (!regex_search(palavra, regex("[^a-zA-Z_]")))
                    {
                        adicionaErro(erros.at("erro_lexico"), numeroLinha);
                    }
                    
                    string rotulo = palavra.substr(0, palavra.size() - 1);
                    // Verifica se o rótulo já foi declarado e definido
                    if (tabelaSimbolos.find(rotulo) != tabelaSimbolos.end() && tabelaSimbolos.at(rotulo).ehDefinido())
                    {
                        adicionaErro(erros.at("rotulo_duplicado"), numeroLinha);
                    }
                    // Verifica se o rótulo já foi declarado mas não definido
                    else if (tabelaSimbolos.find(rotulo) != tabelaSimbolos.end() && !tabelaSimbolos.at(rotulo).ehDefinido())
                    {
                        Simbolo& simbolo = tabelaSimbolos.at(rotulo);
                        simbolo.definir(PROXIMO_ENDERECO);
                        if (resolverPendencias)
                        {
                            for (const unsigned int &pendencia : simbolo.getPendencias())
                            {
                                substituiCodigoObjeto(pendencia, simbolo.getEndereco());
                            }
                        }
                    }
                    else
                    {
                        Simbolo simbolo(rotulo);
                        simbolo.definir(PROXIMO_ENDERECO);
                        tabelaSimbolos.insert({rotulo, simbolo});
                    }
                }
                // Verifica o opcode
                else if (ehOpcode(palavra))
                {
                    // Adiciona no código objeto
                    adicionaCodigoObjeto(tabelaOpCodes.at(palavra).getCodigo());

                    // Lê os parâmetros
                    const Opcode &opcode = tabelaOpCodes.at(palavra);
                    string param;
                    for (int i = 0; i < opcode.getArgumentos(); i++)
                    {
                        string param;
                        if (iss >> param && !ehDefinicaoDeLabel(param) && !ehOpcode(param))
                        {
                            if (tabelaSimbolos.find(param) == tabelaSimbolos.end())
                            {
                                tabelaSimbolos.insert({param, Simbolo(param)});
                                tabelaSimbolos.at(param).adicionarPendencia(PROXIMO_ENDERECO, numeroLinha);
                                adicionaCodigoObjeto(-1);
                            } 
                            else if (!tabelaSimbolos.at(param).ehDefinido())
                            {
                                tabelaSimbolos.at(param).adicionarPendencia(PROXIMO_ENDERECO, numeroLinha);
                                adicionaCodigoObjeto(-1);
                            }
                            else
                            {
                                adicionaCodigoObjeto(tabelaSimbolos.at(param).getEndereco());
                            }
                        }
                        else
                        {
                            adicionaErro(erros.at("parametros_errados"), numeroLinha);
                        }
                    }
                }
                else if (ehDiretiva(palavra))
                {
                    if (palavra == "SPACE")
                    {
                        string tamanhoSpace;
                        if (iss >> tamanhoSpace)
                        {
                            int tamanho = stoi(tamanhoSpace);
                            for (int i = 0; i < tamanho; i++)
                            {
                                adicionaCodigoObjeto(0); // Espaço reservado inicializado com 0
                            }
                        }
                        else
                        {
                            adicionaErro(erros.at("parametros_errados"), numeroLinha);
                        }
                    }
                    else if (palavra == "CONST")
                    {
                        string valorStr;
                        if (iss >> valorStr)
                        {
                            int valor = stoi(valorStr);
                            adicionaCodigoObjeto(valor);
                        }
                        else
                        {
                            adicionaErro(erros.at("parametros_errados"), numeroLinha);
                        }
                    }
                }
                else
                {
                    adicionaErro(erros.at("instrucao_inexistente"), numeroLinha);
                }
            }
        }
        infile.close();
    }

    void validaTabelaDeSimbolos()
    {
        // Se algum simbolo estiver indefinido, adiciona erro nas linhas pendentes
        for (const auto &par : tabelaSimbolos)
        {
            const Simbolo &simbolo = par.second;
            if (!simbolo.ehDefinido())
            {
                for (const int &referencia : simbolo.getReferencias())
                {
                    adicionaErro(erros.at("rotulo_nao_declarado"), referencia);
                }
            }
        }
    }
    
    bool ehDefinicaoDeLabel(const string &palavra)
    {
        return !palavra.empty() && palavra.back() == ':';
    }

    bool ehOpcode(const string &palavra)
    {
        return tabelaOpCodes.find(palavra) != tabelaOpCodes.end();
    }

    bool ehDiretiva(const string &palavra)
    {
        return palavra == "SPACE" || palavra == "CONST";
    }

    void adicionaCodigoObjeto(int codigo)
    {
        codigoObjeto.push_back(codigo);
    }

    void substituiCodigoObjeto(unsigned int endereco, int codigo)
    {
        if (endereco < codigoObjeto.size())
        {
            codigoObjeto[endereco] = codigo;
        }
    }

    void adicionaErro(const string &mensagem, int linha)
    {
        // Escreve erro no fim da linha no infile
        infile.open(inname);
        if (!infile.is_open())
        {
            cerr << "Erro ao abrir arquivo de entrada para adicionar erro: " << inname << endl;
            return;
        }
        string linhaAtual;
        int numeroLinha = 0;
        vector<string> linhas;
        while (getline(infile, linhaAtual))
        {
            numeroLinha++;
            if (numeroLinha == linha)
            {
                linhaAtual += " ; Erro: " + mensagem; 
            }
            linhas.push_back(linhaAtual);
        }
        infile.close();

        // Reescreve o arquivo .pre com as linhas atualizadas
        ofstream tempFile("temp.pre");
        for (const auto &l : linhas)
        {
            tempFile << l << endl;
        }
        tempFile.close();
        remove(inname.c_str());
        rename("temp.pre", inname.c_str());
    }

    void escreveCodigoObjeto()
    {
        for (size_t i = 0; i < codigoObjeto.size(); i++)
        {
            outfile << codigoObjeto[i];
            if (i < codigoObjeto.size() - 1)
                outfile << " ";
        }
        outfile << endl;
        outfile.close();
    }

    void limpeza()
    {
        infile.close();
        outfile.close();
        tabelaSimbolos.clear();
        codigoObjeto.clear();
    }
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Uso: " << argv[0] << " arquivo.pre\n";
        return 1;
    }

    string inname = argv[1];

    // Cria outfile sem extensão. A gestão da extensão será feita pela classe Montador
    string outname = inname;
    if (outname.size() >= 4 && outname.substr(outname.size() - 4) == ".pre")
        outname = outname.substr(0, outname.size() - 4);
    else
    {
        cerr << "Aviso: arquivo de entrada não tem extensão .pre\n";
        return 1;
    }

    Montador montador(inname, outname);
    if (!montador.processa() && !montador.processa(false))
    {
        return 1;
    }
    return 0;
}
