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

typedef struct
{
  char opcode[15];
  char ra[15];
  char rb[15];
  char rc[15];
} operador3;

typedef struct
{
  char opcode[15];
  char end[15];
  char rc[15];
} operador2;

typedef struct
{
  char opcode[15];
  char end[15];
} operador;

/*======================================================================================*/
/*                                   PROTOTIPOS DE FUNCAO                               */
/*======================================================================================*/
static void itob(int valor, char* string, int quantBits);
static void converteLowerUpper(char* linha);
static int primeiroPasso(Hash enderecos, const char* entrada);
static void segundoPasso(Hash enderecos, const char* entrada, const char* saida, int inicio);
static void geraLinha(void* ed, int quantOp, char* linha);
static int verificaRegistrador(char* token);

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

  /*inicia o segundo passo do montador*/
  segundoPasso(enderecos, argv[1], argv[2], inicio);

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
    if(strlen(linha) > 1 && linha[0] != '#')
    {

      if(linha[strlen(linha)-1] == '\n')
        linha[strlen(linha)-1] = '\0';

      converteLowerUpper(linha);

      /*faz o split da linha*/
      token = strtok(linha, " ,#");
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

        token = strtok(NULL, " ,#");

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
  char temp[33] = "";
  char *token;
  char* rotulo;
  Palavra endereco;
  operador3 op3;
  operador2 op2;
  operador op;

  /*printa o address na primeira linha*/
  itob(inicio, valor, 16);
  fprintf(arqSaida, "address ");
  fprintf(arqSaida, "%s\n", valor);

  while(fgets(linha, 35, arqEntrada) != NULL)
  {

    /*caso nao for uma linha vazia*/
    if(strlen(linha) > 1 && linha[0] != '#')
    {

      /*manipula a linha para nao conter \n e coloca em uppercase*/
      if(linha[strlen(linha)-1] == '\n')
        linha[strlen(linha)-1] = '\0';

      converteLowerUpper(linha);

      /*verifica se a linha é um rotulo ou nao*/
      token = strtok(linha, " ,#");
      if(token[strlen(token)-1] != ':')
      {
        if(!strcmp(token, "NOP"))
        {
          strcat(temp, "00000000000000000000000000000000");
        }
        else
        if(!strcmp(token, "ADD"))
        {
          strcpy(op3.opcode, "00000001");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "SUB"))
        {
          strcpy(op3.opcode, "00000010");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "ZEROS"))
        {
          strcpy(op3.opcode, "00000011");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, "0");
          strcpy(op3.rb, "0");
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "XOR"))
        {
          strcpy(op3.opcode, "00000100");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "OR"))
        {
          strcpy(op3.opcode, "00000101");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "NOT"))
        {
          strcpy(op3.opcode, "00000110");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, "0");
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "AND"))
        {
          strcpy(op3.opcode, "00000111");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "ASL"))
        {
          strcpy(op3.opcode, "00001000");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "ASR"))
        {
          strcpy(op3.opcode, "00001001");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "LSL"))
        {
          strcpy(op3.opcode, "00001010");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "LSR"))
        {
          strcpy(op3.opcode, "00001011");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "PASSA"))
        {
          strcpy(op3.opcode, "00001100");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, "0");
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "LCH"))
        {
          strcpy(op2.opcode, "00001110");
          strcpy(op2.rc, strtok(NULL, " ,#"));
          strcpy(op2.end, strtok(NULL, " ,#"));
          geraLinha((void*) &op2, 2, temp);
        }
        else
        if(!strcmp(token, "LCL"))
        {
          strcpy(op2.opcode, "00001111");
          strcpy(op2.rc, strtok(NULL, " ,#"));
          strcpy(op2.end, strtok(NULL, " ,#"));
          geraLinha((void*) &op2, 2, temp);
        }
        else
        if(!strcmp(token, "LOAD"))
        {
          strcpy(op3.opcode, "00010000");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, "0");
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "STORE"))
        {
          strcpy(op3.opcode, "00010001");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, "0");
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "JAL"))
        {
          strcpy(op.opcode, "00100000");
          rotulo = strtok(NULL, " ,#");
          /*caso o primeiro caractere for uma letra eh um rotulo*/
          if(isalpha(rotulo[0]))
          {
            endereco = buscaHash(enderecos, rotulo);
            sprintf(valor, "%ld", strtol(retornaBinario(endereco), NULL, 2) + inicio);
            strcpy(op.end, valor);
          }
          else
            strcpy(op.end, strtok(NULL, " ,#"));

          geraLinha((void*) &op, 1, temp);
        }
        else
        if(!strcmp(token, "JR"))
        {
          strcpy(op3.opcode, "00100001");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, "0");
          strcpy(op3.rb, "0");
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "BEQ"))
        {
          strcpy(op3.opcode, "00100010");
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          rotulo = strtok(NULL, " ,#");
          /*caso o primeiro caractere for uma letra eh um rotulo*/
          if(isalpha(rotulo[0]))
          {
            endereco = buscaHash(enderecos, rotulo);
            sprintf(valor, "%ld", strtol(retornaBinario(endereco), NULL, 2) + inicio);
            strcpy(op3.rc, valor);
          }
          else
            strcpy(op3.rc, strtok(NULL, " ,#"));

          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "BNE"))
        {
          strcpy(op3.opcode, "00100011");
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          rotulo = strtok(NULL, " ,#");
          /*caso o primeiro caractere for uma letra eh um rotulo*/
          if(isalpha(rotulo[0]))
          {
            endereco = buscaHash(enderecos, rotulo);
            sprintf(valor, "%ld", strtol(retornaBinario(endereco), NULL, 2) + inicio);
            strcpy(op3.rc, valor);
          }
          else
            strcpy(op3.rc, strtok(NULL, " ,#"));

          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "J"))
        {
          strcpy(op.opcode, "00100100");
          rotulo = strtok(NULL, " ,#");
          /*caso o primeiro caractere for uma letra eh um rotulo*/
          if(isalpha(rotulo[0]))
          {
            endereco = buscaHash(enderecos, rotulo);
            sprintf(valor, "%ld", strtol(retornaBinario(endereco), NULL, 2) + inicio);
            strcpy(op.end, valor);
          }
          else
            strcpy(op.end, strtok(NULL, " ,#"));

          geraLinha((void*) &op, 1, temp);
        }
        else
        if(!strcmp(token, "MULT"))
        {
          strcpy(op3.opcode, "01000000");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "DIV"))
        {
          strcpy(op3.opcode, "01000001");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "MOD"))
        {
          strcpy(op3.opcode, "01000010");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "ADDI"))
        {
          strcpy(op3.opcode, "01000011");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "SUBI"))
        {
          strcpy(op3.opcode, "01000100");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "MULTI"))
        {
          strcpy(op3.opcode, "01000101");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "DIVI"))
        {
          strcpy(op3.opcode, "01000110");
          strcpy(op3.rc, strtok(NULL, " ,#"));
          strcpy(op3.ra, strtok(NULL, " ,#"));
          strcpy(op3.rb, strtok(NULL, " ,#"));
          geraLinha((void*) &op3, 3, temp);
        }
        else
        if(!strcmp(token, "LOADD"))
        {
          strcpy(op2.opcode, "01000111");
          strcpy(op2.rc, strtok(NULL, " ,#"));
          rotulo = strtok(NULL, " ,#");
          /*caso o primeiro caractere for uma letra eh um rotulo*/
          if(rotulo[0] == '[' && isalpha(rotulo[1]))
          {
            endereco = buscaHash(enderecos, rotulo);
            sprintf(valor, "%ld", strtol(retornaBinario(endereco), NULL, 2));
            strcpy(op2.end, valor);
          }
          else
            strcpy(op2.end, strtok(NULL, " ,#"));
          geraLinha((void*) &op2, 2, temp);
        }
        else
        if(!strcmp(token, "STORED"))
        {
          strcpy(op2.opcode, "01001000");
          strcpy(op2.rc, strtok(NULL, " ,#"));
          rotulo = strtok(NULL, " ,#");
          /*caso o primeiro caractere for uma letra eh um rotulo*/
          if(rotulo[0] == '[' && isalpha(rotulo[1]))
          {
            endereco = buscaHash(enderecos, rotulo);
            sprintf(valor, "%ld", strtol(retornaBinario(endereco), NULL, 2));
            strcpy(op2.end, valor);
          }
          else
            strcpy(op2.end, strtok(NULL, " ,#"));
          geraLinha((void*) &op2, 2, temp);
        }
        else
        if(!strcmp(token, "HALT"))
        {
           strcat(temp, "11111111111111111111111111111111");
        }
      }

      /*caso a linha temporaria nao for vazia*/
      if(strlen(temp) != 0)
      {
        fprintf(arqSaida, "%s\n", temp);
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

/*======================================================================================*/
/*PREENCHE OPCODES - FUNCAO QUE INSERE OS OPCODES NA HASH                               */
/*IN: HASH DE OPCODES                                                          OUT: VOID*/
/*======================================================================================*/
void geraLinha(void* ed, int quantOp, char* linha)
{
  char bin[33];
  char lin[33] = "";

  //strcpy(linha, "");
  if(quantOp == 3)
  {
    operador3 *op3 = (operador3*) ed;
    strcat(lin, op3->opcode);

    /*verifica se o valor e um registrador ou nao*/
    if(verificaRegistrador(op3->ra))
      itob(atoi(&op3->ra[1]), bin, 8);
    else
      itob(atoi(op3->ra), bin, 8);

    strcat(lin, bin);

    if(verificaRegistrador(op3->rb))
      itob(atoi(&op3->rb[1]), bin, 8);
    else
      itob(atoi(op3->rb), bin, 8);

    strcat(lin, bin);

    if(verificaRegistrador(op3->rc))
      itob(atoi(&op3->rc[1]), bin, 8);
    else
      itob(atoi(op3->rc), bin, 8);

    strcat(lin, bin);
  }
  else
  if(quantOp == 2)
  {
    operador2 *op2 = (operador2*) ed;
    strcat(lin, op2->opcode);

    itob(atoi(op2->end), bin, 16);
    strcat(lin, bin);

    itob(atoi(&op2->rc[1]), bin, 8);
    strcat(lin, bin);
  }
  else
  if(quantOp == 1)
  {
    operador *op = (operador*) ed;
    strcat(lin, op->opcode);
    itob(atoi(op->end), bin, 24);
    strcat(lin, bin);
  }

  strcpy(linha, lin);
}

/*======================================================================================*/
/*PREENCHE OPCODES - FUNCAO QUE INSERE OS OPCODES NA HASH                               */
/*IN: HASH DE OPCODES                                                          OUT: VOID*/
/*======================================================================================*/
int verificaRegistrador(char* token)
{
  if((token[0] == 'R' && isdigit(token[1]) && isdigit(token[2])) || 
     (token[0] == 'R' && isdigit(token[1]) && token[2] == '\0'))
  {
    return 1;
  }
  return 0;
}