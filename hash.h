/*=======================================================================*/
/*                Trabalho de AED II - Indice remissivo                  */
/* Nome: Arthur Alexsander Martins Teodoro                               */
/* MAtrícula: 0022427         Data: 13/09/2016                           */
/*=======================================================================*/
#ifndef _HASHENCADEADA_
#define _HASHENCADEADA_

typedef struct palavra *Palavra;
typedef struct hash *Hash;

extern int funcaoHash(char* palavra, int tam);
extern Hash criaHash(int tam);
extern void destroiHash(Hash hash);
extern Palavra insereHash(Hash hash, char* opcode, char* bin);
extern Palavra buscaHash(Hash hash, char* opcode);
extern char* retornaBinario(Palavra plv);
extern void insereBinario(Palavra plv, char* binario);
void printaHash(Hash hash);

#endif