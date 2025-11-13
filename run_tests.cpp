#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm> 

const char* GREEN = "\033[1;32m";
const char* RED = "\033[1;31m";
const char* YELLOW = "\033[1;33m";
const char* RESET = "\033[0m";

std::vector<std::string> getTestFiles(const std::string& directory) {
	std::vector<std::string> files;
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(directory.c_str())) != nullptr) {
		while ((ent = readdir(dir)) != nullptr) {
			std::string filename = ent->d_name;
			if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".asm") {
				files.push_back(filename);
			}
		}
		closedir(dir);
	} else {
		std::perror("Erro ao abrir diretório de testes");
		exit(EXIT_FAILURE);
	}
	std::sort(files.begin(), files.end());
	return files;
}

int main() {
	const std::string testDir = "tests";
	int passed = 0;
	int failed = 0;

	
	auto testFiles = getTestFiles(testDir);
	if (testFiles.empty()) {
		std::cerr << YELLOW << "Nenhum arquivo .asm encontrado em " << testDir << "/." << RESET << std::endl;
		return 0;
	}

	std::cout << "Iniciando " << testFiles.size() << " testes..." << std::endl;
	std::cout << "---------------------------------" << std::endl;

	for (const auto& file : testFiles) {
		std::string path_to_file = testDir + "/" + file;

		bool expect_compile_fail = (file.find("error") != std::string::npos);

		std::cout << YELLOW << "TESTANDO: " << file << RESET << std::endl;
		
		std::string cmd;
		if (expect_compile_fail) {
			cmd = "./compilador " + path_to_file + " > /dev/null 2>&1";
		} else {
			cmd = "./compilador " + path_to_file + " --run > /dev/null 2>&1";
		}

		int status = system(cmd.c_str());
		int exit_code = WEXITSTATUS(status);

		if (expect_compile_fail) {
			if (exit_code != 0) {
				std::cout << GREEN << "  [PASSOU] Compilador falhou como esperado." << RESET << std::endl;
				passed++;
			} else {
				std::cout << RED << "  [FALHOU] Compilador deveria falhar (erro de sintaxe), mas passou." << RESET << std::endl;
				failed++;
			}
		} else {
			if (exit_code == 0) {
				std::cout << GREEN << "  [PASSOU] Compilador e Simulador rodaram com sucesso." << RESET << std::endl;
				passed++;
			} else {
				std::cout << RED << "  [FALHOU] Falha na compilação ou no runtime do simulador." << RESET << std::endl;
				failed++;
			}
		}
		std::cout << "---------------------------------" << std::endl;
	}

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