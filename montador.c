/*======================================================================================*/
/*                   TRABALHO DE AOC - SIMULADOR DE PROCESSADOR RISC                    */
/* AUTOR: ARTHUR ALEXSANDER MARTINS TEODORO - 0022427                  DATA: 07/11/2016 */
/*======================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hash.h"

/*======================================================================================*/
/*                                     TIPOS CRIADOS                                    */
/*======================================================================================*/
enum op_codes{NOP,ADD,SUB,ZEROS,XOR,OR,NOT,AND,ASL,ASR,LSL,LSR,PASSA,LCH,LCL,LOAD,STORE,JAL,JR,
              BEQ,BNE,J,MULT,DIV, MOD, ADDI, SUBI, MULTI, DIVI, LOADD, STORED}opcode;


/*======================================================================================*/
/*                                   PROTOTIPOS DE FUNCAO                               */
/*======================================================================================*/
static void itob(int valor, char* string, int quantBits);
static void preencheOpcodes(Hash opcodes);
static void preencheRegistradores(Hash registradores);
static int primeiroPasso(Hash enderecos, const char* entrada);
static void segundoPasso(Hash opcode, Hash registradores, Hash enderecos, const char* entrada, const char* saida, int inicio);

/*======================================================================================*/
/*                                        FUNCAO MAIN                                   */
/*======================================================================================*/
int main(int argc, char const *argv[])
{
 
  /*vificação de arquivos*/
  if(argc != 3)
  {
    fprintf(stderr, "ERRO - QUANTIDADE DE ARQUIVOS INVALIDA\n");
    return 1;
  }

  FILE* arqEntrada = fopen(argv[1], "rt");
  FILE* arqSaida = fopen(argv[2], "wt");
  if(arqEntrada == NULL)
  {
    fprintf(stderr, "ERRO - ARQUIVO DE ENTRADA NAO EXISTENTE\n");
    return 1;
  }
  fclose(arqEntrada);
  fclose(arqSaida);

  /*Cria a hash de opcodes, hash de enderecos e hash de registradores*/
  Hash opcodes = criaHash(20);
  Hash enderecos = criaHash(50);
  Hash registradores = criaHash(20);

  /*preenche hash que valores nao dependem do arquivo*/
  preencheOpcodes(opcodes);
  preencheRegistradores(registradores);

  /*inicia primeiro passo do montador*/
  int inicio = primeiroPasso(enderecos, argv[1]);
  segundoPasso(opcodes, registradores, enderecos, argv[1], argv[2], inicio);

  /*destroi as hash utilizadas*/
  destroiHash(opcodes);
  destroiHash(enderecos);
  destroiHash(registradores);

  return 0;
}

/*======================================================================================*/
/*ITOB - FUNCAO QUE TRANSFORMA INTEIRO EM BINARIO                                       */
/*IN: VALOR A CONVERTER, VETOR DESTINO E QUANTIDADE DE BITS                    OUT: VOID*/
/*======================================================================================*/
void itob(int valor, char* string, int quantBits)
{
  int i;/*contador*/
  int j = 0;/*indice do vetor resultado*/
  int r;/*deslocamento*/

  for(i = quantBits-1; i >= 0; i--)
  {
    r = valor >> i;
    if(r & 1)
      string[j] = '1';
    else
      string[j] = '0';
    j++;
  }
  string[j] = '\0';
}

/*======================================================================================*/
/*PRIMEIRO PASSO - FUNCAO QUE REALIZA O PRIMEIRO PASSO DO MONTADOR                      */
/*IN: HASH ENDERECOS E NOME DA ENTRADA                              OUT: PONTO DE INICIO*/
/*======================================================================================*/
int primeiroPasso(Hash enderecos, const char* entrada)
{
  /*abre o arquivo de entrada e variaveis usadas*/
  FILE* arq = fopen(entrada, "rt");
  char linha[35];
  int PC = 0, i, dadosInicio = 0;
  char bin[33];
  char valor[33];
  char *token;
  Palavra endereco;

  while(fgets(linha, 35, arq) != NULL)
  {

    /*caso a linha for vazia nao faz nada*/
    if(strlen(linha) > 3)
    {

      if(linha[strlen(linha)-1] == '\n')
        linha[strlen(linha)-1] = '\0';

      /*faz o split da linha*/
      token = strtok(linha, " ");
      while(token != NULL)
      {

        /*caso a token for um label tera dois pontos no fim*/
        if(token[strlen(token)-1] == ':')
        {
          /*retirar os dois pontos, transformar PC em binario e inserir na hash*/
          token[strlen(token)-1] = '\0';
          itob(PC, bin, 24);
          insereHash(enderecos, token, bin);
          PC--;
        }

        /*caso for um endereco de memoria*/
        if(token[0] == '[')
        {
          
          if(!isdigit(token[1]))
          {
            endereco = buscaHash(enderecos, token);
            if(endereco == NULL)
            {
              itob(dadosInicio, bin, 24);
              insereHash(enderecos, token, bin);
              dadosInicio++;
            }
          }
        }

        token = strtok(NULL, " ");

      }
      PC++;
    }
  }

  fclose(arq);
  return dadosInicio;
}

/*======================================================================================*/
/*SEGUNDO PASSO - FUNCAO QUE REALIZA O SEGUNDO PASSO DO MONTADOR                        */
/*IN: HASH OPCODE, REGISTRADORES, ENDERECOS, NOME ENTRADA, SAIDA, INICIO       OUT: VOID*/
/*======================================================================================*/
void segundoPasso(Hash opcode, Hash registradores, Hash enderecos, const char* entrada, const char* saida, int inicio)
{

  /*abre os dois arquivos e variaveis usadas*/
  FILE* arqEntrada = fopen(entrada, "rt");
  FILE* arqSaida = fopen(saida, "wt");
  char linha[35];
  char valor[33];
  char bin[33];
  char temp[33] = "";
  char *token;
  int val, i, quantOperando = 0;
  Palavra conteudo;

  /*printa o address na primeira linha*/
  itob(inicio, valor, 16);
  fprintf(arqSaida, "address ");
  fprintf(arqSaida, "%s\n", valor);

  while(fgets(linha, 35, arqEntrada) != NULL)
  {

    /*caso nao for uma linha vazia*/
    if(strlen(linha) > 3)
    {

      if(linha[strlen(linha)-1] == '\n')
        linha[strlen(linha)-1] = '\0';

      token = strtok(linha, " ");
      while(token != NULL)
      {
        
        /*caso for um label parar o split*/
        if(quantOperando == 0 && (buscaHash(opcode, token) == NULL))
        {
          break;
        }

        /*pega o opcode e analisa e insere no vetor temporario*/
        conteudo = buscaHash(opcode, token);
        if(!strcmp(token, "NOP") || !strcmp(token, "HALT"))
        {
          sprintf(temp, "%s", retornaBinario(conteudo));
          break;
        }
        if(conteudo != NULL)
        {
          sprintf(valor, "%s", retornaBinario(conteudo));
          strcat(temp, valor);
        }

        /*caso for registrador*/
        conteudo = buscaHash(registradores, token);
        if(conteudo != NULL)
        {
          sprintf(valor, "%s", retornaBinario(conteudo));
          strcat(temp, valor);
        }

        /*caso for um valor imediato*/
        /*caso possuir dois operadores que dizer que o valor imediato eh de 16 bits*/
        if(quantOperando == 2 && token[0] == '#')
        {
          for(i = 1; i < strlen(token); i++)
          {
            valor[i-1] = token[i];
          }
          valor[i-1] = '\0';

          val = strtol(valor, NULL, 10);
          itob(val, bin, 16);
          sprintf(valor, "%s", bin);
          strcat(temp, valor);
        }

        /*caso possuir tres operadores que dizer que o valor imediato eh de 8 bits*/
        if(quantOperando == 3 && token[0] == '#')
        {
          for(i = 1; i < strlen(token); i++)
          {
            valor[i-1] = token[i];
          }
          valor[i-1] = '\0';

          val = strtol(valor, NULL, 10);
          itob(val, bin, 8);
          sprintf(valor, "%s", bin);
          strcat(temp, valor);
        }

        /*caso for um valor colhido na primeira passagem*/
        conteudo = buscaHash(enderecos, token);
        if(conteudo != NULL)
        {

          /*caso tenha so um operando o endereco sera de 24 bits*/
          if(quantOperando == 1)
          {
            strcpy(valor, retornaBinario(conteudo));
            val = strtol(valor, NULL, 2);
            val = val + inicio;
            itob(val, bin, 24);
            sprintf(valor, "%s", bin);
            strcat(temp, valor);
          }

          /*caso tenha so um operando o endereco sera de 16 bits*/
          if(quantOperando == 2)
          {
            strcpy(valor, retornaBinario(conteudo));
            val = strtol(valor, NULL, 2);
            val = val + inicio;
            itob(val, bin, 16);
            sprintf(valor, "%s", bin);
            strcat(temp, valor);
          }

          /*caso tenha so um operando o endereco sera de 8 bits*/
          if(quantOperando == 3)
          {
            strcpy(valor, retornaBinario(conteudo));
            val = strtol(valor, NULL, 2);
            val = val + inicio;
            itob(val, bin, 8);
            sprintf(valor, "%s", bin);
            strcat(temp, valor);
          }

        }

        quantOperando++;
        token = strtok(NULL, " ");
      }

      if(strlen(temp) != 0)
      {

        while(strlen(temp) != 32)
        {
          strcat(temp, "0");
        }

        fprintf(arqSaida, "%s\n", temp);
        quantOperando = 0;
        strcpy(temp, "");
      }
    }
  }

  fclose(arqEntrada);
  fclose(arqSaida);
}

/*======================================================================================*/
/*PREENCHE OPCODES - FUNCAO QUE INSERE OS OPCODES NA HASH                               */
/*IN: HASH DE OPCODES                                                          OUT: VOID*/
/*======================================================================================*/
void preencheOpcodes(Hash opcodes)
{

  char bin[33];

  for(opcode = NOP; opcode <= STORED; opcode++)
  {

    itob(opcode, bin, 8);

    switch(opcode)
    {

      case NOP:
        itob(0x00000000, bin, 32);
        insereHash(opcodes, "NOP", bin);
        break;

      case ADD:
        insereHash(opcodes, "ADD", bin);
        break;

      case SUB:
        insereHash(opcodes, "SUB", bin);
        break;

      case ZEROS:
        insereHash(opcodes, "ZEROS", bin);
        break;

      case XOR:
        insereHash(opcodes, "XOR", bin);
        break;

      case OR:
        insereHash(opcodes, "OR", bin);
        break;

      case NOT:
        insereHash(opcodes, "NOT", bin);
        break;

      case AND:
        insereHash(opcodes, "AND", bin);
        break;

      case ASL:
        insereHash(opcodes, "ASL", bin);
        break;

      case ASR:
        insereHash(opcodes, "ASR", bin);
        break;

      case LSL:
        insereHash(opcodes, "LSL", bin);
        break;

      case LSR:
        insereHash(opcodes, "LSR", bin);
        break;

      case PASSA:
        insereHash(opcodes, "PASSA", bin);
        break;

      case LCH:
        insereHash(opcodes, "LCH", bin);
        break;

      case LCL:
        insereHash(opcodes, "LCL", bin);
        break;

      case LOAD:
        insereHash(opcodes, "LOAD", bin);
        break;

      case STORE:
        insereHash(opcodes, "STORE", bin);
        break;

      case JAL:
        insereHash(opcodes, "JAL", bin);
        break;

      case JR:
        insereHash(opcodes, "JR", bin);
        break;

      case BEQ:
        insereHash(opcodes, "BEQ", bin);
        break;

      case BNE:
        insereHash(opcodes, "BNE", bin);
        break;

      case J:
        insereHash(opcodes, "J", bin);
        break;

      case MULT:
        insereHash(opcodes, "MULT", bin);
        break;

      case DIV:
        insereHash(opcodes, "DIV", bin);
        break;

      case MOD:
        insereHash(opcodes, "MOD", bin);
        break;

      case ADDI:
        insereHash(opcodes, "ADDI", bin);
        break;

      case SUBI:
        insereHash(opcodes, "SUBI", bin);
        break;

      case MULTI:
        insereHash(opcodes, "MULTI", bin);
        break;

      case DIVI:
        insereHash(opcodes, "DIVI", bin);
        break;

      case LOADD:
        insereHash(opcodes, "LOADD", bin);
        break;

      case STORED:
        insereHash(opcodes, "STORED", bin);
        break;
    }

  }

  itob(0xffffffff, bin, 32);
  insereHash(opcodes, "HALT", bin);
}

/*======================================================================================*/
/*PREENCHE REGISTRADORES - FUNCAO QUE INSERE OS REGISTRADORES NA HASH                   */
/*IN: HASH DE REGISTRADORES                                                    OUT: VOID*/
/*======================================================================================*/
void preencheRegistradores(Hash registradores)
{

  enum registradores{R0,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,R13,R14,R15,R16,R17,R18,R19,
                     R20,R21,R22,R23,R24,R25,R26,R27,R28,R29,R30,R31,R32,R33}reg;

  char bin[33];

  for(reg = R0; reg <= R33; reg++)
  {

    itob(reg, bin, 8);

    switch(reg)
    {

      case R0:
        insereHash(registradores, "R0", bin);
        break;

      case R1:
        insereHash(registradores, "R1", bin);
        break;

      case R2:
        insereHash(registradores, "R2", bin);
        break;

      case R3:
        insereHash(registradores, "R3", bin);
        break;

      case R4:
        insereHash(registradores, "R4", bin);
        break;

      case R5:
        insereHash(registradores, "R5", bin);
        break;

      case R6:
        insereHash(registradores, "R6", bin);
        break;

      case R7:
        insereHash(registradores, "R7", bin);
        break;

      case R8:
        insereHash(registradores, "R8", bin);
        break;

      case R9:
        insereHash(registradores, "R9", bin);
        break;

      case R10:
        insereHash(registradores, "R10", bin);
        break;

      case R11:
        insereHash(registradores, "R11", bin);
        break;

      case R12:
        insereHash(registradores, "R12", bin);
        break;

      case R13:
        insereHash(registradores, "R13", bin);
        break;

      case R14:
        insereHash(registradores, "R14", bin);
        break;

      case R15:
        insereHash(registradores, "R15", bin);
        break;

      case R16:
        insereHash(registradores, "R16", bin);
        break;

      case R17:
        insereHash(registradores, "R17", bin);
        break;

      case R18:
        insereHash(registradores, "R18", bin);
        break;

      case R19:
        insereHash(registradores, "R19", bin);
        break;

      case R20:
        insereHash(registradores, "R20", bin);
        break;

      case R21:
        insereHash(registradores, "R21", bin);
        break;

      case R22:
        insereHash(registradores, "R22", bin);
        break;

      case R23:
        insereHash(registradores, "R23", bin);
        break;

      case R24:
        insereHash(registradores, "R24", bin);
        break;

      case R25:
        insereHash(registradores, "R25", bin);
        break;

      case R26:
        insereHash(registradores, "R26", bin);
        break;

      case R27:
        insereHash(registradores, "R27", bin);
        break;

      case R28:
        insereHash(registradores, "R28", bin);
        break;

      case R29:
        insereHash(registradores, "R29", bin);
        break;

      case R30:
        insereHash(registradores, "R30", bin);
        break;

      case R31:
        insereHash(registradores, "R31", bin);
        break;

      case R32:
        insereHash(registradores, "R32", bin);
        break;

      case R33:
        insereHash(registradores, "R33", bin);
        break;
    }

  }

}