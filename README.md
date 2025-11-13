# sb2025-2-tb01
Trabalho de **Software Básico** — 2º semestre de 2025

##Projeto: Pré-processador, Montador e Simulador

### Autores
- Gabriel Queiroz — 221020870
- Daniel Monteiro Oliveira — 202006608
- Adriele Evellen Alves de Abreu — 202042785

---

## Compilação e Execução

### Compilar todos os componentes
Para compilar o projeto completo (pré-processador, montador e simulador), basta executar:

```bash
make
```

Esse comando irá gerar os seguintes executáveis:
- `preprocessador`
- `montador`
- E também o script automatizado `compilador`

---

### 🧩 Executar o script do compilador

O script `compilador` integra as etapas de pré-processamento e montagem, e opcionalmente executa o simulador.  
Para usá-lo, rode (lembrando que o "arquivo.asm" deve ser substituido pelo seu arquivo teste):

```bash
./compilador arquivo.asm
```

Caso deseje **compilar e também executar o simulador automaticamente**, use:

```bash
./compilador arquivo.asm --run
```


- O **arquivo de entrada** deve ter extensão `.asm`  
- O nome base (`arquivo`, sem `.asm`) será usado para gerar:
  - `arquivo.pre` → Saída do pré-processador  
  - `arquivo.o1` e `arquivo.o2` → Saídas do montador  
  - O simulador utiliza `arquivo.o2`

---

### Executar os testes automáticos
O projeto inclui um **Test Harness**.  
Para compilar e executar os testes:

```bash
make test
```

---

### Limpar arquivos gerados
Para remover todos os binários e arquivos intermediários:

```bash
make clean
```

---

### Estrutura dos arquivos gerados
| Etapa | Programa | Entrada | Saída |
|:------|:----------|:---------|:-------|
| 1 | `preprocessador` | `arquivo.asm` | `arquivo.pre` |
| 2 | `montador` | `arquivo.pre` | `arquivo.o1`, `arquivo.o2` |
| 3 | `simulador` | `arquivo.o2` | Execução do programa montado |
