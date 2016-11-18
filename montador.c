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
enum op_codes{NOP,ADD,SUB,ZEROS,XOR,OR,NOT,AND,ASL,ASR,LSL,LSR,PASSA,LCH = 14,LCL,LOAD,STORE,JAL = 32,JR,
              BEQ,BNE,J,MULT = 64,DIV, MOD, ADDI, SUBI, MULTI, DIVI, LOADD, STORED}opcode;

/*======================================================================================*/
/*                                   PROTOTIPOS DE FUNCAO                               */
/*======================================================================================*/
static void itob(int valor, char* string, int quantBits);
static void converteLowerUpper(char* linha);
static int primeiroPasso(Hash enderecos, const char* entrada);
static void segundoPasso(Hash enderecos, const char* entrada, const char* saida, int inicio);

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

  /*Cria a hash de enderecos*/
  Hash enderecos = criaHash(50);

  /*inicia primeiro passo do montador*/
  int inicio = primeiroPasso(enderecos, argv[1]);
  printaHash(enderecos);

  /*inicia o segundo passo do montador*/
  //segundoPasso(enderecos, argv[1], argv[2], inicio);

  /*destroi as hash utilizadas*/
  destroiHash(enderecos);

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
  int PC = 0, dadosInicio = 0;
  char bin[33];
  char *token;
  Palavra endereco;

  while(fgets(linha, 35, arq) != NULL)
  {

    /*caso a linha for vazia nao faz nada*/
    if(strlen(linha) > 1)
    {

      if(linha[strlen(linha)-1] == '\n')
        linha[strlen(linha)-1] = '\0';

      converteUpperLower(linha);

      /*faz o split da linha*/
      token = strtok(linha, " ,");
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

        token = strtok(NULL, " ,");

      }
      PC++;
    }
  }

  fclose(arq);
  return dadosInicio;
}

/*======================================================================================*/
/*SEGUNDO PASSO - FUNCAO QUE REALIZA O SEGUNDO PASSO DO MONTADOR                        */
/*IN: HASH ENDERECOS, NOME ENTRADA, SAIDA, INICIO                              OUT: VOID*/
/*======================================================================================*/
void segundoPasso(Hash enderecos, const char* entrada, const char* saida, int inicio)
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
    if(strlen(linha) > 1)
    {

      /*manipula a linha para nao conter \n e coloca em uppercase*/
      if(linha[strlen(linha)-1] == '\n')
        linha[strlen(linha)-1] = '\0';

      converteUpperLower(linha);

      /*verifica se a linha é um rotulo ou nao*/
      token = strtok(linha, " ,");
      if(token[strlen(token)-1] != ':')
      {
        if(!strcmp(token, "ADD"))
        {
          char* rc = strtok(NULL, " ,");
          char* ra = strtok(NULL, " ,");
          char* rb = strtok(NULL, " ,");
        }

      }


      /*caso a linha temporaria nao for vazia*/
      if(strlen(temp) != 0)
      {
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
void converteLowerUpper(char* linha)
{
  int i;
  for(i = 0; i < strlen(linha); i++)
  {
    linha[i] = toupper(linha[i]);
  }
}