/**************************************************/
/*            Projecto SO 2016/2017               */
/*                                                */
/*         Joao Miguel Campos, n 75785            */
/*          Carolina Monteiro, n 77990            */
/*                                                */
/*                                                */
/*                   Grupo 32                     */
/*                                                */
/**************************************************/

#ifndef CONTAS_H
#define CONTAS_H

#define NUM_CONTAS 10
#define TAXAJURO 0.1
#define CUSTOMANUTENCAO 1

#define ATRASO 1

#define NUM_TRABALHADORAS 3  /*numero de threads*/
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)  /*dimensao do buffer circular*/

#define OP_LERSALDO 0
#define OP_CREDITAR 1
#define OP_DEBITAR 2
#define OP_SAIR 3
#define OP_SAIR_AGORA 3


typedef struct  {

	int operacao;
	int idConta;
	int valor;
	
} comando_t;

comando_t cmdbuffer[CMD_BUFFER_DIM];



void inicializarContas();
int contaExiste(int idConta);
int debitar(int idConta, int valor);
int creditar(int idConta, int valor);
int lerSaldo(int idConta);
void simular(int numAnos);
void handler(int sig);
comando_t produzir(int op, int id, int val);
void *thr_consumer (void *item);
comando_t *get();
int consume(comando_t *item);


#endif
