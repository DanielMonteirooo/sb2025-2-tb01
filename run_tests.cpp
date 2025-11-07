#include <iostream>
    #include <fstream>
    #include <string>
    #include <vector>
    #include <cstdio>     // Para system()
    #include <cstdlib>    // Para system()
    #include <cstring>
    #include <dirent.h>   // Para ler diretórios
    #include <sys/wait.h> // Para WIFEXITED, WEXITSTATUS
    #include <unistd.h>
    #include <algorithm>  // Para std::sort

    // --- Constantes de Cor (para o terminal) ---
    const char* GREEN = "\033[1;32m";
    const char* RED = "\033[1;31m";
    const char* YELLOW = "\033[1;33m";
    const char* RESET = "\033[0m";

    // --- Função para buscar arquivos de teste .asm ---
    std::vector<std::string> getTestFiles(const std::string& directory) {
        std::vector<std::string> files;
        DIR* dir;
        struct dirent* ent;
        if ((dir = opendir(directory.c_str())) != nullptr) {
            while ((ent = readdir(dir)) != nullptr) {
                std::string filename = ent->d_name;
                // Pega apenas arquivos .asm
                if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".asm") {
                    files.push_back(filename);
                }
            }
            closedir(dir);
        } else {
            std::perror("Erro ao abrir diretório de testes");
            exit(EXIT_FAILURE);
        }
        std::sort(files.begin(), files.end()); // Ordena os arquivos
        return files;
    }

    // --- Função Principal de Teste ---
    int main() {
        const std::string testDir = "tests";
        int passed = 0;
        int failed = 0;

        // 1. Garante que os executáveis estão compilados
        if (system("make all") != 0) {
            std::cerr << RED << "Falha ao compilar os programas! Testes abortados." << RESET << std::endl;
            return 1;
        }
        
        // 2. Pega todos os arquivos .asm de dentro do diretório "tests"
        auto testFiles = getTestFiles(testDir);
        if (testFiles.empty()) {
            std::cerr << YELLOW << "Nenhum arquivo .asm encontrado em " << testDir << "/." << RESET << std::endl;
            return 0;
        }

        std::cout << "Iniciando " << testFiles.size() << " testes..." << std::endl;
        std::cout << "---------------------------------" << std::endl;

        for (const auto& file : testFiles) {
            std::string path_to_file = testDir + "/" + file;
            std::string base_name = path_to_file.substr(0, path_to_file.length() - 4); // Remove .asm
            std::string o2_file = base_name + ".o2";

            std::string cmd_compile = "./compilador " + path_to_file + " > /dev/null 2>&1";
            std::string cmd_simulate = "./simulador " + o2_file + " > /dev/null 2>&1";

            // Determina o que esperamos do teste
            // Se o nome do arquivo contém "error", esperamos que a compilação falhe.
            bool expect_compile_fail = (file.find("error") != std::string::npos);

            std::cout << YELLOW << "TESTANDO: " << file << RESET << std::endl;
            
            // --- ETAPA 1: COMPILAÇÃO ---
            int compile_status = system(cmd_compile.c_str());
            int compile_exit_code = WEXITSTATUS(compile_status);

            if (expect_compile_fail) {
                // ESPERÁVAMOS UMA FALHA NA COMPILAÇÃO
                if (compile_exit_code != 0) {
                    std::cout << GREEN << "  [PASSOU] Compilador falhou como esperado." << RESET << std::endl;
                    passed++;
                } else {
                    std::cout << RED << "  [FALHOU] Compilador deveria falhar (erro de sintaxe), mas passou." << RESET << std::endl;
                    failed++;
                }
            } else {
                // ESPERÁVAMOS SUCESSO NA COMPILAÇÃO
                if (compile_exit_code != 0) {
                    std::cout << RED << "  [FALHOU] Compilador falhou inesperadamente." << RESET << std::endl;
                    failed++;
                } else {
                    // Compilação passou, agora verifica a simulação
                    std::cout << "  [INFO] Compilado com sucesso. Executando simulador..." << std::endl;
                    
                    int sim_status = system(cmd_simulate.c_str());
                    int sim_exit_code = WEXITSTATUS(sim_status);
                    
                    if (sim_exit_code == 0) {
                        std::cout << GREEN << "  [PASSOU] Simulador rodou com sucesso." << RESET << std::endl;
                        passed++;
                    } else {
                        std::cout << RED << "  [FALHOU] Simulador falhou inesperadamente (erro de runtime)." << RESET << std::endl;
                        failed++;
                    }
                }
            }
            std::cout << "---------------------------------" << std::endl;
        } // Fim do loop de testes

        // --- Relatório Final ---
        std::cout << "Testes Concluídos." << std::endl;
        if (failed > 0) {
            std::cout << RED << "RESULTADO: " << failed << " falharam, " << RESET;
            std::cout << GREEN << passed << " passaram." << RESET << std::endl;
            return 1;
        } else {
            std::cout << GREEN << "RESULTADO: Todos os " << passed << " testes passaram!" << RESET << std::endl;
            return 0;
        }
    }