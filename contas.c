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


#include "contas.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>


#define atrasar() sleep(ATRASO)


/***   CONSTANTES   ***/ 


int buff_write_idx = 0, buff_read_idx = 0;


/* Flag para verificar signals.*/

int sig_find = 0;

pthread_mutex_t mutexContas[NUM_CONTAS];
pthread_mutex_t mutexTransf;



/***   FUNCOES   ***/


int contaExiste(int idConta)  {

  	return (idConta > 0 && idConta <= NUM_CONTAS);
}


void inicializarContas()  {

  	int i, rc;

  	for (i=0; i<NUM_CONTAS; i++)  {

		contasSaldos[i] = 0;

  		if ((rc = pthread_mutex_init(&mutexContas[i], NULL)) != 0)  {

        	errno = rc;

        	perror("pthread_mutex_init: ");
		}
	}

  	if ((rc = pthread_mutex_init(&mutexTransf, NULL)) != 0)  {

		errno = rc;

    	perror("pthread_mutex_init: ");
    }
}


int debitar(int idConta, int valor)  {

  	atrasar();

  	if (!contaExiste(idConta) || valor < 0)

		return -1;

	testMutexLock(&mutexContas[idConta - 1]);

  	if (contasSaldos[idConta - 1] < valor)  {

		testMutexUnlock(&mutexContas[idConta - 1]);
		
		return -1;
	}

  	atrasar();

  	contasSaldos[idConta - 1] -= valor;

  	testMutexUnlock(&mutexContas[idConta - 1]);

  	return 0;
}


int creditar(int idConta, int valor)  {

  	atrasar();

  	if (!contaExiste(idConta) || valor < 0)

		return -1;

	testMutexLock(&mutexContas[idConta - 1]);

  	contasSaldos[idConta - 1] += valor;

  	testMutexUnlock(&mutexContas[idConta - 1]);

  	return 0;
}


int lerSaldo(int idConta)  {

	int i;

	atrasar();

	if (!contaExiste(idConta))

		return -1;

	testMutexLock(&mutexContas[idConta - 1]);

	i = contasSaldos[idConta - 1];

	testMutexUnlock(&mutexContas[idConta - 1]);

	return i;
}


void simular(int numAnos)  {

  	int ano, saldo, idConta;

  	/* Ciclo que percorre o numero de anos até que seja igual ao
   	* Numero de anos que o utilizador introduziu ou até o processo filho
   	* receber um signal do processo pai.*/

	for (ano = 0; ano < numAnos && sig_find != 1; ano++)  {

		printf("\nSIMULACAO: Ano %d\n", ano);

		printf("=================\n");
		
		/* Simula o valor de cada conta, num determinado ano.
		 * Credita-se a taxa de juro e, caso o saldo nao seja suficiente para debitar
		 * o custo de manutencao, a conta fica a zero.
		 * Caso contrario, e debitado o valor.*/

		for (idConta = 1; idConta <= NUM_CONTAS; idConta++)  {

			saldo = lerSaldo(idConta);

			if (ano != 0)  {
			
		  		creditar(idConta, saldo*TAXAJURO);

		  	if (saldo < CUSTOMANUTENCAO)

				debitar(idConta, saldo);

		  	else
				debitar(idConta, CUSTOMANUTENCAO);

			saldo = lerSaldo(idConta);
			}

		printf("Conta %d, Saldo %d\n", idConta, saldo);

		}
 	}

	/* Se um signal for enviado pelo processo pai, imprime a mensagem.*/

  	if (sig_find == 1)

		printf("Simulacao terminada por signal\n");
}


/* Funcao de tratamento de um signal. Ao reconhecer que recebe um signal,
 * volta a reconfigurar o que o processo filho faz, caso receba outro signal.*/


void handler(int sig)  {

	sig_find = 1;

	signal(sig, handler);
}

int transferir(int idConta, int idContaDest, int valor)  {

	atrasar();

	if (!contaExiste(idConta) || !contaExiste(idContaDest) || idConta == idContaDest)

		return -1;

	testMutexLock(&mutexTransf);

	debitar(idConta, valor);

	creditar(idContaDest, valor);

	testMutexUnlock(&mutexTransf);

	return 0;
}



/*************************/
/*       PARTE 2         */
/*************************/

/* Funcao que recebe os 3 argumentos iniciais e os coloca na estrutura comando_t, retornando-a. */

comando_t produzir(int op, int idOri, int val, int idDest)  {

  	comando_t i;

  	if(op <= OP_SAIR_AGORA)  {

	  	i.operacao = op;

	  	i.idConta = idOri;
	  	
	  	i.valor = val;

	  	i.idContaDestino = idDest;
	}

	else  {

		i.operacao = op;

		i.idConta = idOri;

		i.valor = idDest;

		i.idContaDestino = val;
	}

  	return i;
}


/* Funcao que recebe uma estrutura e a coloca no buffer circular. */

void writeBuf(comando_t item)  {

	testSemWait(&escrita);

	cmdbuffer[buff_write_idx] = item;

	buff_write_idx = (buff_write_idx + 1) % CMD_BUFFER_DIM;

	testSemPost(&leitura);
}

/* Funcao a ser executada em cada uma das threads consumidoras. */

void* thr_consumer (void *arg) {

  	while(1)  {

		comando_t item;

		item = readBuf();

		consume(item);
			
  	}

  	return NULL;
}

/* Funcao que retira uma estrutura comando_t do buffer circular e devolve-a. */

comando_t readBuf()  {

  	testSemWait(&leitura);

  	testMutexLock(&cadeadoC);

  	comando_t item;

  	item = cmdbuffer[buff_read_idx];

  	buff_read_idx = (buff_read_idx + 1) % CMD_BUFFER_DIM;

  	testMutexUnlock(&cadeadoC);

	testSemPost(&escrita);

	return item;
}


/* Funcao que analisa a estrutura recebida e executa o comando respectivo. */

int consume(comando_t item)  {

	int saldo;

	if (item.operacao == OP_LERSALDO)  {

		saldo = lerSaldo(item.idConta);

		if (saldo < 0)

        	printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, item.idConta);

    	else

        	printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, item.idConta, saldo);
	}

	if (item.operacao == OP_CREDITAR)  {

		if (creditar (item.idConta, item.valor) < 0)

            printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, item.idConta, item.valor);

    	else

            printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, item.idConta, item.valor);
	}

	if (item.operacao == OP_DEBITAR)  {
		
		if (debitar (item.idConta, item.valor) < 0)

            printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, item.idConta, item.valor);

        else

            printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, item.idConta, item.valor);
		
	}

	if (item.operacao == OP_TRANSFERIR)  {
		if (item.idConta == item.idContaDestino)

			printf("Erro ao transferir valor da conta %d para a conta %d", item.idConta, item.idContaDestino);


	}

	if (item.operacao == OP_SAIR)  {

		pthread_exit(EXIT_SUCCESS);
	}

	return 0;
}

/* Funcao testMutexLock verifica se e possivel trancar o mutex, atraves da funcao pthread_mutex_lock(). 
 * Retorna 0 em caso afirmativo e resulta em erro caso contrario. */

int testMutexLock(pthread_mutex_t *cadeado)  {

	int rc;

	if((rc = pthread_mutex_lock(cadeado)) != 0)  {

		errno = rc;

		perror("pthread_mutex_lock: ");
	}

	return 0;
}

/* Funcao testMutexUnlock verifica se e possivel destrancar o mutex, atraves da funcao pthread_mutex_unlock(). 
 * Retorna 0 em caso afirmativo e resulta em erro caso contrario. */

int testMutexUnlock(pthread_mutex_t *cadeado)  {

	int rc;

	if((rc = pthread_mutex_unlock(cadeado)) != 0)  {

		errno = rc;

		perror("pthread_mutex_unlock: ");
	}

	return 0;
}

/* Funcao testSemWait verifica se e possivel fechar o semaforo, atraves da funcao sem_wait(). 
 * Retorna 0 em caso afirmativo e resulta em erro caso contrario. */

int testSemWait(sem_t *semaforo)  {

	if(sem_wait(semaforo) != 0)  {

		perror("sem_wait: ");
	}

	return 0;
}

/* Funcao testSemPost verifica se e possivel abrir o semaforo, atraves da funcao sem_post(). 
 * Retorna 0 em caso afirmativo e resulta em erro caso contrario. */

int testSemPost(sem_t *semaforo)  {

	if(sem_post(semaforo) != 0)  {

		perror("sem_post: ");
	}

	return 0;
}

/* Funcao testSemDestroy verifica se e possivel destruir o semaforo, atraves da funcao sem_destroy(). 
 * Retorna 0 em caso afirmativo e resulta em erro caso contrario. */

int testSemDestroy(sem_t *semaforo)  {

	if(sem_destroy(semaforo) != 0)  {

		perror("sem_destroy: ");
	}

	return 0;
}

/* Funcao testMutexDestroy verifica se e possivel destruir o mutex, atraves da funcao pthread_mutex_destroy(). 
 * Retorna 0 em caso afirmativo e resulta em erro caso contrario. */

int testMutexDestroy(pthread_mutex_t *cadeado)  {

	int rc;

	if((rc = pthread_mutex_destroy(cadeado)) != 0)  {

		errno = rc;

		perror("pthread_mutex_destroy: ");
	}

	return 0;
}