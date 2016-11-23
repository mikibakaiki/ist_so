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
#include <fcntl.h>


#define atrasar() sleep(ATRASO)


/***   CONSTANTES   ***/


int buff_write_idx = 0, buff_read_idx = 0, count = 0;


/* Flag para verificar signals. */

int sig_find = 0;


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

  	if ((rc = pthread_mutex_init(&mutexCount, NULL)) != 0)  {

		errno = rc;

    	perror("pthread_mutex_init: ");
    }

   if ((rc = pthread_cond_init(&cond, NULL)) != 0)  {

        errno = rc;

        perror("pthread_cond_init: ");
    }
}


int debitar(int idConta, int valor, int num)  {

  	atrasar();

  	if (!contaExiste(idConta) || valor < 0)  {

        dprintf(fd, "%d: %s - ERRO\n", num, COMANDO_DEBITAR);

		return -1;
    }

	testMutexLock(&mutexContas[idConta - 1]);

  	if (contasSaldos[idConta - 1] < valor)  {

        dprintf(fd, "%d: %s - ERRO\n", num, COMANDO_DEBITAR);

        testMutexUnlock(&mutexContas[idConta - 1]);

		return -1;
	}

  	atrasar();

  	contasSaldos[idConta - 1] -= valor;

    dprintf(fd, "%d: %s - OK\n", num, COMANDO_DEBITAR);

  	testMutexUnlock(&mutexContas[idConta - 1]);

  	return 0;
}


/* Funcao igual a debitar(), apenas nao contem mutex,
 * para nao ocorrerem eventuais erros de interblocagem. */

int debitarTransf(int idConta, int valor)  {

  	atrasar();

  	if (!contaExiste(idConta) || valor < 0)

		return -1;

  	if (contasSaldos[idConta - 1] < valor)  {

		return -1;
	}

  	atrasar();

  	contasSaldos[idConta - 1] -= valor;

  	return 0;
}



int creditar(int idConta, int valor, int num)  {

  	atrasar();

  	if (!contaExiste(idConta) || valor < 0)  {
        dprintf(fd, "%d: %s - ERRO\n", num, COMANDO_CREDITAR);
		return -1;
    }

	testMutexLock(&mutexContas[idConta - 1]);

  	contasSaldos[idConta - 1] += valor;

    dprintf(fd, "%d: %s - OK\n", num, COMANDO_CREDITAR);

  	testMutexUnlock(&mutexContas[idConta - 1]);

  	return 0;
}


/* Funcao igual a creditar(), apenas nao contem mutex,
 * para nao ocorrerem eventuais erros de interblocagem. */

int creditarTransf(int idConta, int valor)  {

  	atrasar();

  	if (!contaExiste(idConta) || valor < 0)

		return -1;

  	contasSaldos[idConta - 1] += valor;

  	return 0;
}


int lerSaldo(int idConta, int num)  {

	int i;

	atrasar();

	if (!contaExiste(idConta))  {

        dprintf(fd, "%d: %s - ERRO\n", num, COMANDO_LER_SALDO);

		return -1;
    }

	testMutexLock(&mutexContas[idConta - 1]);

	i = contasSaldos[idConta - 1];

    dprintf(fd, "%d: %s - OK\n", num, COMANDO_LER_SALDO);

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

			saldo = lerSaldo(idConta, 0);

			if (ano != 0)  {

		  		creditar(idConta, saldo*TAXAJURO, 0);

		  		if (saldo < CUSTOMANUTENCAO)

					debitar(idConta, saldo, 0);

		  		else
					debitar(idConta, CUSTOMANUTENCAO, 0);

			saldo = lerSaldo(idConta, 0);

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

	if (signal(sig, handler) == SIG_ERR)

		perror("signal: ");
}


int transferir(int idConta, int idContaDest, int valor, int num)  {


	if (!contaExiste(idConta) || !contaExiste(idContaDest) || idConta == idContaDest)  {

        dprintf(fd, "%d: %s - ERRO\n", num, COMANDO_TRANSFERIR);
		return -1;
    }

	testMutexLock(&mutexContas[min(idConta - 1, idContaDest -1 )]);
	testMutexLock(&mutexContas[max(idConta - 1, idContaDest - 1)]);

	if (debitarTransf(idConta, valor) !=0)  {

        dprintf(fd, "%d: %s - ERRO\n", num, COMANDO_TRANSFERIR);
		testMutexUnlock(&mutexContas[max(idConta - 1, idContaDest - 1)]);
		testMutexUnlock(&mutexContas[min(idConta - 1, idContaDest - 1)]);

		return -1;
	}

	if ( creditarTransf(idContaDest, valor) != 0)  {

        dprintf(fd, "%d: %s - ERRO\n", num, COMANDO_TRANSFERIR);
		testMutexUnlock(&mutexContas[max(idConta - 1, idContaDest - 1)]);
		testMutexUnlock(&mutexContas[min(idConta - 1, idContaDest - 1)]);

		return -1;
	}

    dprintf(fd, "%d: %s - OK\n", num, COMANDO_TRANSFERIR);

	testMutexUnlock(&mutexContas[max(idConta - 1, idContaDest - 1)]);
	testMutexUnlock(&mutexContas[min(idConta - 1, idContaDest - 1)]);

	return 0;
}



/*************************/
/*       PARTE 2         */
/*************************/

/* Funcao que recebe os 3 argumentos iniciais e os coloca na estrutura comando_t, retornando-a. */

comando_t produzir(int op, int idOri, int val, int idDest)  {

  	comando_t i;

  	if (op <= OP_SAIR_AGORA)  {

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

	testMutexLock(&mutexCount);

	/* Depois de colocar um comando no buffer, incrementamos a variavel count,
	 * que conta o numero comandos por executar. */

	count++;

	testMutexUnlock(&mutexCount);

	cmdbuffer[buff_write_idx] = item;

	buff_write_idx = (buff_write_idx + 1) % CMD_BUFFER_DIM;

	testSemPost(&leitura);
}

/* Funcao a ser executada em cada uma das threads consumidoras. */

void* thr_consumer (void *arg) {

    int t_num;

    t_num = *((int *)arg) + 1;

  	while(1)  {

		comando_t item;

		item = readBuf();

		consume(item,t_num);
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

int consume(comando_t item, int num)  {

	int saldo, rc;

	if (item.operacao == OP_LERSALDO)  {

		saldo = lerSaldo(item.idConta, num);

		if (saldo < 0)

        	printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, item.idConta);

    	else

        	printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, item.idConta, saldo);
	}

	if (item.operacao == OP_CREDITAR)  {

		if (creditar (item.idConta, item.valor, num) < 0)

            printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, item.idConta, item.valor);

    	else

            printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, item.idConta, item.valor);
	}

	if (item.operacao == OP_DEBITAR)  {

		if (debitar (item.idConta, item.valor, num) < 0)

            printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, item.idConta, item.valor);

        else

            printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, item.idConta, item.valor);

	}

	if (item.operacao == OP_TRANSFERIR)  {

		if ( (transferir(item.idConta, item.idContaDestino, item.valor, num) != 0) )

			printf("Erro ao transferir valor da conta %d para a conta %d\n\n", item.idConta, item.idContaDestino);

		else

			printf("transferir(%d, %d, %d): OK\n\n", item.idConta, item.idContaDestino, item.valor);
	}

	if (item.operacao == OP_SAIR)  {

		pthread_exit(EXIT_SUCCESS);
	}

    /* Aqui deve levar o write para poder escrever que ja concluiu o comando*/

	testMutexLock(&mutexCount);

	/* Depois de "consumido" um comando, decrementamos a variavel count,
	 * que conta o numero comandos por executar. */

	count --;

	/* Quando count == 0, e enviado um signal para a variavel pthread_cond_t &cond. */

	if (count == 0)  {

		if ((rc = pthread_cond_signal(&cond)) != 0)  {

			errno = rc;

			perror("pthread_cond_signal: ");
		}
	}

	testMutexUnlock(&mutexCount);

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


/* Funcao que recebe dois inteiros, e devolve o menor de ambos. */
int min(int x, int y)  {

	if (x < y)

		return x;

	return y;
}

/* Funcao que recebe dois inteiros, e devolve o maior de ambos. */

int max(int x, int y)  {

	if (x > y)

		return x;

	return y;
}
