#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

// Nomes de macros
class NomeDeMacro
{
private:
    string nome;
    int quantidadeArgumentos;
    int linhaDefinicao;

public:
    NomeDeMacro() : nome(""), quantidadeArgumentos(0), linhaDefinicao(-1) {}
    NomeDeMacro(const string &nome, int quantidadeArgumentos, int linhaDefinicao)
        : nome(nome), quantidadeArgumentos(quantidadeArgumentos), linhaDefinicao(linhaDefinicao) {}

    string getNome() const { return nome; }
    int getQuantidadeArgumentos() const { return quantidadeArgumentos; }
    int getLinhaDefinicao() const { return linhaDefinicao; }
};

// Tabela de definições de macros
class DefinicaoDeMacro
{
private:
    vector<string> definicao;

public:
    DefinicaoDeMacro(const string &nome, const vector<string> &definicao)
        : definicao(definicao) {}

    vector<string> getDefinicao() const { return definicao; }
};

class Preprocessador
{
public:
    Preprocessador(const string &inname, const string &outname)
        : inname(inname), outname(outname) {}

    bool processa()
    {
        this->passagemZero();
        return 0;
    }

private:
    string inname;
    string outname;
    ifstream infile;
    ofstream outfile;
    map<int, NomeDeMacro> tabelaNomesDeMacros;
    map<int, DefinicaoDeMacro> tabelaDefinicoesDeMacros;

    /*
        Passagem 0:
        - Ler proxima linha do código fonte
        - Se é MACRO, ler a definição da macro e armazenar na tabela de definições de macros
        - Se é outra diretiva de pré-processamento, processar
        - Se é o nome de uma macro, expandir a definição da tabela de definições de macros, substituindo os argumentos pelos valores passados e escrevendo um novo codigo fonte
        - Caso contrário, escrever a linha no novo código fonte
        - Se eof, terminar
    */
    void passagemZero()
    {
        infile.open(inname);
        if (!infile.is_open())
        {
            cerr << "Erro ao abrir arquivo de entrada: " << inname << endl;
            return;
        }

        outfile.open(outname);
        if (!outfile.is_open())
        {
            cerr << "Erro ao abrir arquivo de saída: " << outname << endl;
            return;
        }

        string linha;
        int numeroLinha = 0;
        // Lê linha por linha do arquivo de entrada
        while (getline(infile, linha))
        {
            numeroLinha++;
            istringstream iss(linha);

            // Verifica se a linha tem label
            bool temLabel = false;
            if (linha.find(':') != string::npos)
            {
                temLabel = true;
            }

            string label;
            string opcode;
            
            if (temLabel)
                iss >> label >> opcode;
            else
                iss >> opcode;
            
            label.erase(remove_if(label.begin(), label.end(), ::isspace), label.end());
            label.erase(remove(label.begin(), label.end(), ':'), label.end());
            opcode.erase(remove_if(opcode.begin(), opcode.end(), ::isspace), opcode.end());
            
            bool macroDefinida = false;
            for (const auto &par : tabelaNomesDeMacros)
            {
                if (par.second.getNome() == opcode)
                {
                    macroDefinida = true;
                    break;
                }
            }

            // Se a linha começa com MACRO, guardar a definição da macro
            if (label == "MACRO")
            {
                // Falta dar um nome à macro
                cout << "Alerta: Nome da macro esperado na linha " << numeroLinha << endl;
                continue;
            }
            else if (opcode == "MACRO")
            {
                armazenarDefinicaoMacro(iss, label);
            }
            // Se é o nome de uma macro, expandir a definição da tabela de definições de macros
            else if (tabelaNomesDeMacros.size() > 0 && macroDefinida)
            {
                expandirMacro(iss, opcode);
            }
            // Caso contrário, escrever a linha no novo código fonte
            else
            {
                outfile << linha << endl;
            }
        }
    }

    void armazenarDefinicaoMacro(istringstream &iss, string nomeMacro)
    {
        int linhaTabelaDeDefinicao = tabelaDefinicoesDeMacros.size();

        map<string, string> argumentos;
        string arg;
        while (iss >> arg)
        {
            arg.erase(remove_if(arg.begin(), arg.end(), ::isspace), arg.end());
            arg.erase(remove(arg.begin(), arg.end(), ','), arg.end());
            
            if (arg[0] == ';')
                break; // Ignora comentários

            string originalArg = arg;

            arg = "#" + to_string(argumentos.size() + 1); // Renomeia para #1, #2, ...
            argumentos.emplace(originalArg, arg);
        }

        int quantidadeArgumentos = argumentos.size();
        NomeDeMacro nomeDeMacro(nomeMacro, quantidadeArgumentos, linhaTabelaDeDefinicao);
        tabelaNomesDeMacros.emplace(linhaTabelaDeDefinicao, nomeDeMacro);

        vector<string> definicao;
        string linha;
        // Lê a definição da macro até encontrar ENDMACRO
        while (getline(infile, linha))
        {
            istringstream issLinha(linha);
            string palavra;
            issLinha >> palavra;
            
            if (palavra[0] == ';')
                continue; // Ignora comentários

            if (palavra == "ENDMACRO")
            {
                break;
            }

            // Substitui os argumentos pelos nomes renomeados
            for (const auto &par : argumentos)
            {
                size_t pos = linha.find(par.first);
                while (pos != string::npos)
                {
                    linha.replace(pos, par.first.length(), par.second);
                    pos = linha.find(par.first, pos + par.second.length());
                }
            }

            definicao.push_back(linha);
        }

        DefinicaoDeMacro definicaoDeMacro(nomeMacro, definicao);
        tabelaDefinicoesDeMacros.emplace(linhaTabelaDeDefinicao, definicaoDeMacro);
    }

    void expandirMacro(istringstream &iss, const string &nomeMacro)
    {
        // Encontra nome de macro que corresponde ao opcode
        
        NomeDeMacro nomeDeMacro;
        for (const auto &par : tabelaNomesDeMacros)
        {
            if (par.second.getNome() == nomeMacro)
            {
                nomeDeMacro = par.second;
                break;
            }
        }

        if (nomeDeMacro.getLinhaDefinicao() == -1)
        {
            cout << "Alerta: Macro " << nomeMacro << " não definida." << endl;
            return;
        }
        DefinicaoDeMacro definicaoDeMacro = tabelaDefinicoesDeMacros.at(nomeDeMacro.getLinhaDefinicao());
        int quantidadeArgumentos = nomeDeMacro.getQuantidadeArgumentos();
        map<string, string> argumentosPassados;
        string arg;
        
        while (iss >> arg)
        {
            arg.erase(remove_if(arg.begin(), arg.end(), ::isspace), arg.end());
            arg.erase(remove(arg.begin(), arg.end(), ','), arg.end());

            if (arg[0] == ';')
                break; // Ignora comentários

            argumentosPassados.emplace("#" + to_string(argumentosPassados.size() + 1), arg);
        }
        
        if (argumentosPassados.size() != quantidadeArgumentos)
        {
            cout << "Alerta: Quantidade de argumentos incorreta para a macro " << nomeMacro << endl;
            return;
        }

        vector<string> definicao = definicaoDeMacro.getDefinicao();
        for (const string &linha : definicao)
        {
            string linhaExpandida = linha;

            for (int i = 0; i < quantidadeArgumentos; i++)
            {
                string argNome = "#" + to_string(i + 1);
                size_t pos = linhaExpandida.find(argNome);
                while (pos != string::npos)
                {
                    linhaExpandida.replace(pos, argNome.length(), argumentosPassados[argNome]);
                    pos = linhaExpandida.find(argNome, pos + argumentosPassados[argNome].length());
                }
            }

            outfile << linhaExpandida << endl;
        }
    }
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Uso: " << argv[0] << " arquivo.asm\n";
        return 1;
    }

    string inname = argv[1];
    // Troca extensão .asm por .pre
    string outname = inname;
    if (outname.size() >= 4 && outname.substr(outname.size() - 4) == ".asm")
        outname = outname.substr(0, outname.size() - 4) + ".pre";
    else
        outname += ".pre";

    Preprocessador pre(inname, outname);
    if (!pre.processa())
    {
        return 1;
    }
    return 0;
}
