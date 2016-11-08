/*=======================================================================*/
/*                Trabalho de AED II - Indice remissivo                  */
/* Nome: Arthur Alexsander Martins Teodoro                               */
/* MAtrícula: 0022427         Data: 13/09/2016                           */
/*=======================================================================*/

/*=======================================================================*/
/*                        BIBLIOTECAS USADAS                             */
/*=======================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "hash.h"

/*=======================================================================*/
/*                         ESTRUTURA CRIADA                              */
/*=======================================================================*/
struct palavra
{
	char opcode[51];
	char bin[33];
	struct palavra *prox;
};

struct hash
{
	int tam;
	int colisao;
	struct palavra **vetor;
};

int funcaoHash(char* palavra, int tam)
{
	unsigned int ascii = 0;
	int i;

	for(i = 1; i <= strlen(palavra); i++)
	{
		ascii = ascii + (palavra[i-1] * i);
	}

	return ascii % tam;
}

Hash criaHash(int tam)
{
	int i;
	srand(time(NULL));
	Hash hash = (Hash) malloc(sizeof(struct hash));
	hash->tam = tam;
	hash->colisao = 0;
	hash->vetor = (Palavra*) malloc(sizeof(Palavra)*tam);
	for(i = 0; i < tam; i++)
	{
		hash->vetor[i] = NULL;
	}
	return hash;
}

void destroiHash(Hash hash)
{
	Palavra excluir;
	int i;
	for(i = 0; i < hash->tam; i++)
	{
		while(hash->vetor[i] != NULL)
		{
			excluir = hash->vetor[i];
			hash->vetor[i] = excluir->prox;
			free(excluir);
		}
	}
	free(hash->vetor);
	free(hash);
	hash = NULL;
	return;
}

Palavra insereHash(Hash hash, char* opcode, char* bin)
{
	int posicao;
	posicao = funcaoHash(opcode, hash->tam);

	Palavra inserir = (Palavra) malloc(sizeof(struct palavra));

	inserir->prox = hash->vetor[posicao];
	hash->vetor[posicao] = inserir;

	strcpy(inserir->opcode, opcode);
	strcpy(inserir->bin, bin);

	return inserir;
}

Palavra buscaHash(Hash hash, char* opcode)
{
	int posicao;
	posicao = funcaoHash(opcode, hash->tam);

	Palavra olhar = hash->vetor[posicao];

	while(olhar != NULL && strcmp(opcode, olhar->opcode))
	{
		olhar = olhar->prox;
	}

	if(olhar == NULL)
	{
		return NULL;
	}

	return olhar;
}

void insereBinario(Palavra plv, char* binario)
{
	if(plv == NULL)
		return;

	strcpy(plv->bin, binario);
}

void printaHash(Hash hash)
{
	int i;
	Palavra plv;
	for(i = 0; i < hash->tam; i++)
	{
		if(hash->vetor[i] != NULL)
		{
			plv = hash->vetor[i];
			while(plv != NULL)
			{
				printf("%s - ", plv->opcode);
				printf("%s\n", plv->bin);
				plv = plv->prox;
			}
		}
	}
}

char* retornaBinario(Palavra plv)
{
	return plv->bin;
}