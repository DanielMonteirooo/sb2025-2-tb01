#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

struct MacroInfo {
    string nome;
    vector<string> args_formais;
    vector<string> definicao;
};

class Preprocessador {
public:
    Preprocessador(const string &inname, const string &outname)
        : inname(inname), outname(outname) {}

    bool processa() {
        if (!identificarMacros()) {
            cerr << "Erro durante a identificação de macros." << endl;
            return false;
        }
        if (!expandirMacros()) {
            cerr << "Erro durante a expansão de macros." << endl;
            return false;
        }
        return true;
    }

private:
    string inname;
    string outname;
    map<string, MacroInfo> tabelaMacros;

    bool identificarMacros() {
        ifstream infile(inname);
        if (!infile.is_open()) {
            cerr << "Erro ao abrir arquivo de entrada: " << inname << endl;
            return false;
        }

        string linha, macroAtualNome;
        bool dentroDefMacro = false;
        vector<string> argsAtuais, corpoAtual;

        while (getline(infile, linha)) {
            string linhaOriginal = linha;
            string linhaProcessada = normalizarLinha(linha);
            stringstream ss(linhaProcessada);
            string token1, token2;

            ss >> token1 >> token2;

            if (dentroDefMacro) {
                if (token1 == "ENDMACRO") {
                    dentroDefMacro = false;
                    if (!macroAtualNome.empty()) {
                        tabelaMacros[macroAtualNome] = {macroAtualNome, argsAtuais, corpoAtual};
                        macroAtualNome = "";
                        argsAtuais.clear();
                        corpoAtual.clear();
                    }
                } else {
                    size_t posComentario = linhaOriginal.find(';');
                    if (posComentario != string::npos)
                        corpoAtual.push_back(linhaOriginal.substr(0, posComentario));
                    else
                        corpoAtual.push_back(linhaOriginal);
                }
            } else {
                string operacao;
                bool temLabel = linhaProcessada.find(':') != string::npos;

                if (temLabel && !token1.empty() && token1.back() == ':') {
                    operacao = token2;
                    macroAtualNome = token1.substr(0, token1.length() - 1);
                } else {
                    operacao = token1;
                    macroAtualNome = "";
                }

                if (operacao == "MACRO") {
                    if (!temLabel) {
                        if (token1.find(':') != string::npos) {
                            cerr << "Erro: Nome de macro inválido na linha: " << linha << endl;
                            infile.close();
                            return false;
                        }
                        macroAtualNome = token1;
                        ss = stringstream(linhaProcessada);
                        ss >> token1;
                    }

                    if (macroAtualNome.empty()) {
                        cerr << "Erro: Definição de MACRO sem nome na linha: " << linha << endl;
                        infile.close();
                        return false;
                    }

                    if (tabelaMacros.count(macroAtualNome)) {
                        cerr << "Erro: Redefinição da macro '" << macroAtualNome << "'" << endl;
                        infile.close();
                        return false;
                    }

                    dentroDefMacro = true;
                    argsAtuais.clear();
                    corpoAtual.clear();

                    string arg;
                    while (ss >> arg) {
                        arg.erase(remove(arg.begin(), arg.end(), ','), arg.end());
                        if (!arg.empty() && arg[0] == '&') {
                            argsAtuais.push_back(arg);
                        } else if (!arg.empty()) {
                            cerr << "Aviso: Argumento de macro inválido '" << arg
                                 << "' na definição de " << macroAtualNome << ". Ignorando." << endl;
                        }
                    }

                    if (argsAtuais.size() > 2) {
                        cerr << "Erro: Macro '" << macroAtualNome
                             << "' excede o limite de 2 argumentos." << endl;
                        infile.close();
                        return false;
                    }
                }
            }
        }

        infile.close();

        if (dentroDefMacro) {
            cerr << "Erro: Definição de macro '" << macroAtualNome
                 << "' não terminada com ENDMACRO." << endl;
            return false;
        }

        return true;
    }

    bool expandirMacros() {
        ifstream infile(inname);
        if (!infile.is_open()) {
            cerr << "Erro ao abrir arquivo de entrada para expansão: " << inname << endl;
            return false;
        }

        ofstream outfile(outname);
        if (!outfile.is_open()) {
            cerr << "Erro ao abrir arquivo de saída: " << outname << endl;
            infile.close();
            return false;
        }

        string linha;
        bool pulandoDefMacro = false;

        while (getline(infile, linha)) {
            string linhaNormalizada = normalizarLinha(linha);
            stringstream ss(linhaNormalizada);
            string token1, token2, label, operacao;

            size_t colonPos = linhaNormalizada.find(':');
            if (colonPos != string::npos &&
                (colonPos == linhaNormalizada.find_first_not_of(" \t") +
                             linhaNormalizada.substr(linhaNormalizada.find_first_not_of(" \t")).find(':'))) {
                label = linhaNormalizada.substr(0, colonPos);
                label.erase(remove_if(label.begin(), label.end(), ::isspace), label.end());
                ss = stringstream(linhaNormalizada.substr(colonPos + 1));
                ss >> operacao;
            } else {
                ss >> operacao;
            }

            if (operacao == "MACRO" || (token1 == "MACRO" && !label.empty())) {
                pulandoDefMacro = true;
                continue;
            }

            if (pulandoDefMacro) {
                string tempOp;
                stringstream tempSs(linhaNormalizada);
                tempSs >> tempOp;
                if (tempOp == "ENDMACRO")
                    pulandoDefMacro = false;
                continue;
            }

            if (tabelaMacros.count(operacao)) {
                vector<string> argsPassados;
                string arg;
                while (ss >> arg) {
                    arg.erase(remove(arg.begin(), arg.end(), ','), arg.end());
                    argsPassados.push_back(arg);
                }

                MacroInfo infoMacro = tabelaMacros[operacao];
                if (argsPassados.size() != infoMacro.args_formais.size()) {
                    cerr << "Erro: Número incorreto de argumentos para a macro '" << operacao
                         << "' na linha: " << linha << endl;
                    outfile << "; *** ERRO NA CHAMADA DA MACRO ACIMA ***" << endl;
                } else {
                    if (!label.empty())
                        outfile << label << ": ";
                    vector<string> linhasExpandidas = expandir(infoMacro, argsPassados);
                    for (const string &l : linhasExpandidas)
                        outfile << "\t" << l << endl;
                }
            } else {
                size_t posComentario = linha.find(';');
                if (posComentario != string::npos)
                    outfile << linha.substr(0, posComentario) << endl;
                else
                    outfile << linha << endl;
            }
        }

        infile.close();
        outfile.close();
        return true;
    }

    vector<string> expandir(const MacroInfo &macro, const vector<string> &args_passados) {
        vector<string> resultado;
        for (const string &linha_def : macro.definicao) {
            string linha_expandida = linha_def;
            for (size_t i = 0; i < macro.args_formais.size(); ++i) {
                size_t pos = linha_expandida.find(macro.args_formais[i]);
                while (pos != string::npos) {
                    linha_expandida.replace(pos, macro.args_formais[i].length(), args_passados[i]);
                    pos = linha_expandida.find(macro.args_formais[i], pos + args_passados[i].length());
                }
            }

            stringstream ss_check(normalizarLinha(linha_expandida));
            string check_op;
            ss_check >> check_op;
            if (check_op.find(':') != string::npos)
                ss_check >> check_op;

            if (tabelaMacros.count(check_op)) {
                vector<string> args_passados_nested;
                string arg_nested;
                while (ss_check >> arg_nested) {
                    arg_nested.erase(remove(arg_nested.begin(), arg_nested.end(), ','), arg_nested.end());
                    args_passados_nested.push_back(arg_nested);
                }
                if (tabelaMacros[check_op].args_formais.size() == args_passados_nested.size()) {
                    vector<string> nested_expansion = expandir(tabelaMacros[check_op], args_passados_nested);
                    resultado.insert(resultado.end(), nested_expansion.begin(), nested_expansion.end());
                } else {
                    cerr << "Erro: Número incorreto de argumentos para macro aninhada '" << check_op
                         << "' dentro de '" << macro.nome << "'" << endl;
                    resultado.push_back("; *** ERRO NA CHAMADA DA MACRO ANINHADA ACIMA ***");
                }
            } else {
                resultado.push_back(linha_expandida);
            }
        }
        return resultado;
    }

    string normalizarLinha(const string &linha) {
        string res = linha;
        size_t posComentario = res.find(';');
        if (posComentario != string::npos)
            res = res.substr(0, posComentario);
        transform(res.begin(), res.end(), res.begin(), ::toupper);
        replace(res.begin(), res.end(), '\t', ' ');
        res.erase(0, res.find_first_not_of(" \t"));
        res.erase(res.find_last_not_of(" \t") + 1);
        string temp, word;
        stringstream ss(res);
        while (ss >> word)
            temp += word + " ";
        if (!temp.empty())
            temp.pop_back();
        return temp;
    }
};

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <nome_base_arquivo>\n";
        cerr << "Exemplo: " << argv[0] << " exemplo (para processar exemplo.asm)" << endl;
        return 1;
    }

    string nome_base = argv[1];
    string inname = nome_base + ".asm";
    string outname = nome_base + ".pre";

    Preprocessador pre(inname, outname);
    if (!pre.processa())
        return 1;

    return 0;
}
