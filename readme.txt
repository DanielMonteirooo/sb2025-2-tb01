Trabalho 1 de Software Básico
Pre-processador e Montador
=======================
Autores:
- Gabriel Queiroz - 221020870
- Daniel Monteiro Oliveira - 202006608
- Adriele Evellen Alves de Abreu - 202042785
=======================
Instruções para compilar e executar
=======================
O modo mais simples de compilar e executar o pré-processador e o montador é utilizando o
script shell `run.sh`. É necessário informar o NOME do arquivo de entrada (sem a extensão) como argumento.
Por exemplo, se o arquivo de entrada é "exemplo.asm":
```
bash ./run.sh exemplo
```

Para compilar utilizando o makefile, basta rodar o make no terminal

Para executar o pré-processador, utilize o comando:
```
./preprocessador <arquivo_entrada.asm>
```
Para executar o montador, utilize o comando:
```
./montador <arquivo_entrada.pre>
```
=======================
