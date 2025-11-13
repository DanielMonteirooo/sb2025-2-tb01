CC_CPP = g++
CFLAGS_CPP = -std=c++17 -Wall -Werror

SIM_PROF_EXEC = simulador/simulador

all: compilador

compilador: preprocessador montador
	@echo "Criando o script 'compilador' (para simulador do professor)..."
	@echo "#!/bin/bash" > compilador
	@echo "if [ \$$# -eq 0 ]; then echo 'Uso: ./compilador arquivo.asm [--run]'; exit 1; fi" >> compilador
	@echo "INPUT_FILE=\$$1" >> compilador
	@echo "RUN_FLAG=\$$2" >> compilador
	@echo "NOME_BASE=\$$(echo \"\$$INPUT_FILE\" | sed 's/\.asm$$//')" >> compilador
	@echo "echo '--- [ETAPA 1/2] Executando Pré-Processador ---'" >> compilador
	@echo "./preprocessador \"\$$NOME_BASE\"" >> compilador
	@echo "if [ \$$? -ne 0 ]; then echo 'Erro no pré-processamento.'; exit 1; fi" >> compilador
	@echo "echo '--- [ETAPA 2/2] Executando Montador ---'" >> compilador
	@echo "./montador \"\$$NOME_BASE\"" >> compilador
	@echo "if [ \$$? -ne 0 ]; then" >> compilador
	@echo "  echo 'Erro na montagem. Verifique o arquivo .pre para detalhes.'" >> compilador
	@echo "  exit 1" >> compilador
	@echo "fi" >> compilador
	@echo "echo Compilação concluída com sucesso para \"\$$NOME_BASE\"." >> compilador
	@echo "if [ \"\$$RUN_FLAG\" == \"--run\" ]; then" >> compilador
	@echo "  echo '--- [ETAPA 3/3] Executando Simulador (Professor) ---'" >> compilador
	# --- LINHA MODIFICADA ---
	# Chama o simulador do professor no caminho correto
	@echo "  $(SIM_PROF_EXEC) \"\$$NOME_BASE.o2\"" >> compilador
	@echo "fi" >> compilador
	@chmod +x compilador

preprocessador: preprocessador.cpp
	$(CC_CPP) $(CFLAGS_CPP) -o preprocessador preprocessador.cpp

montador: montador.cpp
	$(CC_CPP) $(CFLAGS_CPP) -o montador montador.cpp

run_tests: run_tests.cpp
	$(CC_CPP) -std=c++17 -Wall -o run_tests run_tests.cpp

test: all run_tests
	@echo "--- Iniciando o Test Harness ---"
	./run_tests

clean:
	# Removido 'simulador' da lista, pois não é mais um produto deste makefile
	rm -f compilador preprocessador montador run_tests
	rm -f *.pre *.o1 *.o2
	rm -f tests/*.pre tests/*.o1 tests/*.o2
	rm -f exemplos/*.pre exemplos/*.o1 exemplos/*.o2

.PHONY: all test clean