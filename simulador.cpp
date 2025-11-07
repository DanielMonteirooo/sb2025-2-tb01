/*
 * Simulador da máquina hipotética (Software Básico - UnB)
 * Formato .o2: uma LINHA com inteiros separados por espaço.
 *
 * OPCODES:
 * 01 ADD op     ACC = ACC + mem[op]
 * 02 SUB op     ACC = ACC - mem[op]
 * 03 MUL op     ACC = ACC * mem[op]
 * 04 DIV op     ACC = ACC / mem[op]         (erro se mem[op] == 0)
 * 05 JMP op     PC = op
 * 06 JMPN op    if (ACC < 0) PC = op
 * 07 JMPP op    if (ACC > 0) PC = op
 * 08 JMPZ op    if (ACC == 0) PC = op
 * 09 COPY a b   mem[b] = mem[a]
 * 10 LOAD op    ACC = mem[op]
 * 11 STORE op   mem[op] = ACC
 * 12 INPUT op   lê int do stdin -> mem[op]
 * 13 OUTPUT op  imprime mem[op]\n
 * 14 STOP       halt
 *
 * Tamanhos: quase tudo 2 palavras; COPY = 3; STOP = 1.
 *
 * Uso:
 *   gcc -O2 -std=c99 -o simulador simulator.c
 *   ./simulador programa.o2 [--trace] [--max-steps=N]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MEM_SIZE 65536

typedef struct
{
  int trace;
  long long max_steps;
} Options;

static void die(const char *m)
{
  fprintf(stderr, "Erro: %s\n", m);
  exit(1);
}
static void usage(const char *a) { fprintf(stderr, "Uso: %s arquivo.o2 [--trace] [--max-steps=N]\n", a); }

static int parse_options(int argc, char **argv, Options *opt)
{
  opt->trace = 0;
  opt->max_steps = 10000000LL; // 10 milhões
  for (int i = 2; i < argc; ++i)
  {
    if (!strcmp(argv[i], "--trace"))
      opt->trace = 1;
    else if (!strncmp(argv[i], "--max-steps=", 12))
    {
      char *end = NULL;
      long long v = strtoll(argv[i] + 12, &end, 10);
      if (!*(argv[i] + 12) || (end && *end) || v <= 0)
        return 0;
      opt->max_steps = v;
    }
    else
      return 0;
  }
  return 1;
}

static int read_all_ints(const char *path, int32_t *mem, size_t *out_len)
{
  FILE *f = fopen(path, "r");
  if (!f)
    return 0;
  size_t i = 0;
  while (1)
  {
    long long v;
    int r = fscanf(f, "%lld", &v);
    if (r == EOF)
      break;
    if (r != 1)
    {
      int c = fgetc(f);
      if (c == EOF)
        break;
      continue;
    } // ignora “lixo”
    if (i >= MEM_SIZE)
    {
      fclose(f);
      die("programa maior que MEM_SIZE");
    }
    mem[i++] = (int32_t)v;
  }
  fclose(f);
  *out_len = i;
  return 1;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    usage(argv[0]);
    return 1;
  }
  Options opt;
  if (!parse_options(argc, argv, &opt))
  {
    usage(argv[0]);
    return 1;
  }

  static int32_t mem[MEM_SIZE] = {0};
  size_t n = 0;
  if (!read_all_ints(argv[1], mem, &n))
  {
    fprintf(stderr, "Não foi possível abrir '%s'\n", argv[1]);
    return 1;
  }
  if (n == 0)
    die("arquivo .o2 vazio.");

  int32_t ACC = 0;
  uint32_t PC = 0;
  long long steps = 0;

  while (1)
  {
    if (steps++ > opt.max_steps)
      die("limite de passos excedido");
    if (PC >= MEM_SIZE)
      die("PC fora da memória");
    int32_t op = mem[PC];
    if (opt.trace)
      fprintf(stderr, "[trace] PC=%u ACC=%d OPC=%d\n", PC, ACC, op);

    switch (op)
    {
    case 1:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("ADD end");
      ACC += mem[a];
      PC += 2;
      break;
    } // ADD
    case 2:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("SUB end");
      ACC -= mem[a];
      PC += 2;
      break;
    } // SUB
    case 3:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("MUL end");
      ACC *= mem[a];
      PC += 2;
      break;
    } // MUL
    case 4:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("DIV end");
      if (mem[a] == 0)
        die("DIV zero");
      ACC /= mem[a];
      PC += 2;
      break;
    } // DIV
    case 5:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("JMP end");
      PC = a;
      break;
    } // JMP
    case 6:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("JMPN end");
      PC = (ACC < 0) ? a : (PC + 2);
      break;
    } // JMPN
    case 7:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("JMPP end");
      PC = (ACC > 0) ? a : (PC + 2);
      break;
    } // JMPP
    case 8:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("JMPZ end");
      PC = (ACC == 0) ? a : (PC + 2);
      break;
    } // JMPZ
    case 9:
    {
      uint32_t a = mem[PC + 1], b = mem[PC + 2];
      if (a >= MEM_SIZE || b >= MEM_SIZE)
        die("COPY end");
      mem[b] = mem[a];
      PC += 3;
      break;
    } // COPY
    case 10:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("LOAD end");
      ACC = mem[a];
      PC += 2;
      break;
    } // LOAD
    case 11:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("STORE end");
      mem[a] = ACC;
      PC += 2;
      break;
    } // STORE
    case 12:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("INPUT end");
      long long v;
      if (scanf("%lld", &v) != 1)
        die("INPUT falha");
      mem[a] = (int32_t)v;
      PC += 2;
      break;
    } // INPUT
    case 13:
    {
      uint32_t a = mem[PC + 1];
      if (a >= MEM_SIZE)
        die("OUTPUT end");
      printf("%d\n", mem[a]);
      fflush(stdout);
      PC += 2;
      break;
    } // OUTPUT
    case 14:
    {
      return 0;
    } // STOP
    default:
    {
      fprintf(stderr, "Opcode desconhecido %d em PC=%u\n", op, PC);
      return 1;
    }
    }
  }
}
