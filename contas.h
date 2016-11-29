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

#include <pthread.h>
#include <semaphore.h>

#define NUM_CONTAS 10
#define TAXAJURO 0.1
#define CUSTOMANUTENCAO 1

#define ATRASO 0

#define NUM_TRABALHADORAS 3  /*numero de threads*/
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)  /*dimensao do buffer circular*/
#define MAXFILHOS 20

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_TRANSFERIR "transferir"

#define OP_LERSALDO 0
#define OP_CREDITAR 1
#define OP_DEBITAR 2
#define OP_SAIR 3
#define OP_SAIR_AGORA 4
#define OP_TRANSFERIR 5
#define OP_SIMULAR 6


typedef struct  {

	int operacao;
	int idConta;
	int valor;
	int idContaDestino;
	char nome[1024];

} comando_t;

comando_t cmdbuffer[CMD_BUFFER_DIM];


pthread_mutex_t cadeadoC;
pthread_mutex_t mutexContas[NUM_CONTAS];
pthread_mutex_t mutexCount;
pthread_cond_t cond;

sem_t escrita;
sem_t leitura;


int count;  /* Numero de comandos por executar. */

int fd, newF;

int contasSaldos[NUM_CONTAS];
pid_t pidFilhos[MAXFILHOS];



void inicializarContas();
int contaExiste(int idConta);
int debitar(int idConta, int valor, int num);
int creditar(int idConta, int valor, int num);
int lerSaldo(int idConta, int num);

void simular(int numAnos);
void handler(int sig);


/***   PARTE 2   ***/


comando_t produzir(int op, int idOri, int val, int idDest, char nome[50]);
void writeBuf(comando_t item);
void* thr_consumer(void *arg);
comando_t readBuf();
int consume(comando_t item, int t_num);


int testMutexLock(pthread_mutex_t *cadeado);
int testMutexUnlock(pthread_mutex_t *cadeado);

int testSemWait(sem_t *semaforo);
int testSemPost(sem_t *semaforo);

int testSemDestroy(sem_t *semaforo);
int testMutexDestroy(pthread_mutex_t *cadeado);


/***   PARTE 3   ***/

int transferir(int idConta, int idContaDest, int valor, int num);

int debitarTransf(int idConta, int valor);
int creditarTransf(int idConta, int valor);
int lerSaldoTransf(int idConta, int num);
int min(int x, int y);
int max(int x, int y);


#endif
