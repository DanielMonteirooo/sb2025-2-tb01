#! /bin/sh
# Refaz build se necessário
make run
# Executa o programa
./preprocessador ./$@.asm
./montador ./$@.pre
# Mostra o resultado
echo ""
echo "Conteúdo do arquivo $@.o1:"
cat ./$@.o1
echo ""
echo "Conteúdo do arquivo $@.o2:"
cat ./$@.o2
echo ""