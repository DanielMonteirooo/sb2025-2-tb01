#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>
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

class Pendencia
{
private:
    int endereco;
    int linha;
    int offset;
public:
    Pendencia(int endereco, int linha, int offset)
        : endereco(endereco), linha(linha), offset(offset) {}
    int getEndereco() const { return endereco; }
    int getLinha() const { return linha; }
    int getOffset() const { return offset; }
};

class Simbolo
{
private:
    string simbolo;
    int endereco;
    bool definido;
    vector<Pendencia> pendencias;
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

    void adicionarPendencia(int endereco, int linha, int offset = 0)
    {
        Pendencia p(endereco, linha, offset);
        pendencias.push_back(p);
        referencias.push_back(linha);
    }

    const vector<Pendencia>& getPendencias() const
    {
        return pendencias;
    }

    const vector<int>& getReferencias() const
    {
        return referencias;
    }
};

// - Rotulo declarado duas vezes em lugares diferentes ✅
// - Dois rótulos na mesma linha ✅
// - Rotulo não declarado ✅
// - Instrução com número de parâmetros errado ✅
// - Instução inexistente ✅
// - Erros léxicos (label  não pode começar por número e o único caracter especial que pode ter é o “_”). ✅
const map<string, string> erros = {
    {"rotulo_duplicado", "Erro semântico: rótulo declarado duas vezes em lugares diferentes"},
    {"dois_rotulos", "Erro sintático: dois rótulos na mesma linha"},
    {"rotulo_nao_declarado", "Erro semântico: rótulo não declarado"},
    {"parametros_errados", "Erro sintático: instrução com número de parâmetros errado"},
    {"instrucao_inexistente", "Erro sintático: instrução inexistente"},
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
        if (!errosExibidos)
        {
            this->adicionaErros();
        }
        this->limpeza();
        return 0;
    }
private:
    string inname;
    string outname;
    ifstream infile;
    ofstream outfile;
    map<int, string> errosLinha;
    bool errosExibidos = false;
    bool resolverPendencias;
    int numeroLinha = 0;
    int contadorLabelsNaLinha = 0;

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
        numeroLinha = 0; // usado para adiconar erros na linha correta
        contadorLabelsNaLinha = 0;
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
                if (ehDefinicaoDeLabel(palavra)) tratarLabel(palavra);
                // Verifica o opcode
                else if (ehOpcode(palavra)) tratarOpcode(palavra, iss);
                // Verifica diretiva
                else if (ehDiretiva(palavra)) tratarDiretiva(palavra, iss);
                else
                {
                    contadorLabelsNaLinha = 0;
                    errosLinha.insert({numeroLinha, erros.at("instrucao_inexistente")});
                }
            }
        }
        infile.close();
    }

    void tratarLabel(const string &palavra)
    {
        contadorLabelsNaLinha++;

        if (contadorLabelsNaLinha > 1)
        {
            errosLinha.insert({numeroLinha, erros.at("dois_rotulos")});
        }

        string rotulo = palavra.substr(0, palavra.size() - 1);
        
        if (rotulo.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != string::npos)
        {
            errosLinha.insert({numeroLinha, erros.at("erro_lexico")});
        }

        // Verifica se o rótulo já foi declarado e definido
        if (tabelaSimbolos.find(rotulo) != tabelaSimbolos.end() && tabelaSimbolos.at(rotulo).ehDefinido())
        {
            errosLinha.insert({numeroLinha, erros.at("rotulo_duplicado")});
        }

        // Verifica se o rótulo já foi declarado mas não definido
        else if (tabelaSimbolos.find(rotulo) != tabelaSimbolos.end() && !tabelaSimbolos.at(rotulo).ehDefinido())
        {
            Simbolo& simbolo = tabelaSimbolos.at(rotulo);
            simbolo.definir(PROXIMO_ENDERECO);
            if (resolverPendencias)
            {
                for (const Pendencia &pendencia : simbolo.getPendencias())
                {
                    substituiCodigoObjeto(pendencia.getEndereco(), simbolo.getEndereco() + pendencia.getOffset());
                }
            }
        }

        // Define novo rótulo
        else
        {
            Simbolo simbolo(rotulo);
            simbolo.definir(PROXIMO_ENDERECO);
            tabelaSimbolos.insert({rotulo, simbolo});
        }
    }

    void tratarOpcode(const string &palavra, istringstream &iss)
    {
        contadorLabelsNaLinha = 0;
                    
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
                // Verifica se há offset
                size_t maisPos = param.find('+');
                string rotulo = (maisPos != string::npos) ? param.substr(0, maisPos) : param;
                int offset = 0;
                if (maisPos != string::npos)
                {
                    string offsetStr = param.substr(maisPos + 1);
                    try {
                        offset = stoi(offsetStr);
                    } catch (const invalid_argument &e) {
                        errosLinha.insert({numeroLinha, erros.at("parametros_errados")});
                        continue;
                    }
                }

                // Verifica se o símbolo não existe na tabela de símbolos (adiciona como pendência)
                if (tabelaSimbolos.find(rotulo) == tabelaSimbolos.end())
                {
                    Simbolo simbolo(rotulo);
                    simbolo.adicionarPendencia(PROXIMO_ENDERECO, numeroLinha, offset);
                    tabelaSimbolos.insert({rotulo, simbolo});
                    adicionaCodigoObjeto(-1);
                }
                // Verifica se o símbolo já foi declarado mas não definido (adiciona como pendência)
                else if (!tabelaSimbolos.at(rotulo).ehDefinido())
                {
                    tabelaSimbolos.at(rotulo).adicionarPendencia(PROXIMO_ENDERECO, numeroLinha, offset);
                    adicionaCodigoObjeto(-1);
                }
                // Símbolo já definido
                else
                {
                    adicionaCodigoObjeto(tabelaSimbolos.at(rotulo).getEndereco() + offset);
                }
            }
            else
            {
                errosLinha.insert({numeroLinha, erros.at("parametros_errados")});
            }
        }
    }

    void tratarDiretiva(const string &palavra, istringstream &iss)
    {
        contadorLabelsNaLinha = 0;
                    
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
                adicionaCodigoObjeto(0);
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
                errosLinha.insert({numeroLinha, erros.at("parametros_errados")});
            }
        }
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
                    errosLinha.insert({referencia, erros.at("rotulo_nao_declarado")});
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

    void adicionaErros()
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
            try {
                if (errosLinha.find(numeroLinha) != errosLinha.end())
                {
                    linhaAtual += " ; " + errosLinha.at(numeroLinha);
                }
                linhas.push_back(linhaAtual);
            } catch (const out_of_range &e) {
                linhas.push_back(linhaAtual);
            }
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
        errosExibidos = true;
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
