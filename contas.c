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
#include <string.h>
#include <time.h>

#define atrasar() sleep(ATRASO)


/***   CONSTANTES   ***/
int buff_write_idx = 0, buff_read_idx = 0, count = 0;

int numB;   /* Variavel que guarda numero de bytes escritos */
int error;  /* Variavel que testa erros */
int fd;     /* /tmp/log.txt file descriptor */

/* Flag para verificar signals. */
int sig_find = 0;

/***   FUNCOES   ***/

int contaExiste(int idConta)  {

  	return (idConta > 0 && idConta <= NUM_CONTAS);
}


void inicializarContas()  {

  	int i;

  	for (i=0; i<NUM_CONTAS; i++)  {

		contasSaldos[i] = 0;

  		if ((error = pthread_mutex_init(&mutexContas[i], NULL)) != 0)  {

        	errno = error;

        	perror("pthread_mutex_init: ");
		}
	}

  	if ((error = pthread_mutex_init(&mutexCount, NULL)) != 0)  {

		errno = error;

    	perror("pthread_mutex_init: ");
    }

   if ((error = pthread_cond_init(&cond, NULL)) != 0)  {

        errno = error;

        perror("pthread_cond_init: ");
    }
}


int debitar(int idConta, int valor, int num)  {

  	atrasar();
    char buf[1024];

  	if (!contaExiste(idConta) || valor < 0)  {

        if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d, %d) - ERRO\n", num, COMANDO_DEBITAR, idConta, valor)) >= sizeof(buf))  {
            printf("Erro snprintf\n");
        }

        if ((error = write(fd, buf, strlen(buf))) == -1)  {
            perror("write_debitar: ");
        }
		return -1;
    }

	testMutexLock(&mutexContas[idConta - 1]);

  	if (contasSaldos[idConta - 1] < valor)  {

        if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d, %d) - ERRO\n", num, COMANDO_DEBITAR, idConta, valor)) >= sizeof(buf))  {
            printf("Erro snprintf\n");
        }

        if ((error = write(fd, buf, strlen(buf))) == -1)  {
            perror("write_debitar: ");
        }

        testMutexUnlock(&mutexContas[idConta - 1]);

		return -1;
	}

  	atrasar();

  	contasSaldos[idConta - 1] -= valor;

    if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d, %d) - OK\n", num, COMANDO_DEBITAR, idConta, valor)) >= sizeof(buf))  {
        printf("Erro snprintf\n");
    }

    if ((error = write(fd, buf, strlen(buf))) == -1)  {
        perror("write_debitar: ");
    }

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

    char buf[1024];

  	if (!contaExiste(idConta) || valor < 0)  {

        if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d, %d) - ERRO\n", num, COMANDO_CREDITAR, idConta, valor)) >= sizeof(buf))  {
            printf("Erro snprintf\n");
        }
        printf("fd = %d\n", fd);
        if ((error = write(fd, buf, strlen(buf))) == -1)  {
            perror("write_creditar: ");
        }

		return -1;
    }

	testMutexLock(&mutexContas[idConta - 1]);

  	contasSaldos[idConta - 1] += valor;

    if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d,%d) - OK\n", num, COMANDO_CREDITAR, idConta, valor)) >=sizeof(buf))  {
        printf("Erro snprintf\n");
    }

    if ((error = write(fd, buf, strlen(buf))) == -1)  {
        perror("write_creditar: ");
    }

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
    char buf[1024];

	atrasar();

	if (!contaExiste(idConta))  {

        if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d) - ERRO\n", num, COMANDO_LER_SALDO, idConta)) >= sizeof(buf))  {
            printf("Erro snprintf\n");
        }

        if ((error = write(fd, buf, strlen(buf))) == -1)  {
            perror("write_lerSaldo: ");
        }

		return -1;
    }

	testMutexLock(&mutexContas[idConta - 1]);

	i = contasSaldos[idConta - 1];

    if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d) - OK\n", num, COMANDO_LER_SALDO, idConta)) >= sizeof(buf))  {
        printf("Erro snprintf\n");
    }

    if ((error = write(fd, buf, strlen(buf))) == -1)  {
        perror("write_lerSaldo: ");
    }

	testMutexUnlock(&mutexContas[idConta - 1]);

	return i;
}

int lerSaldoTransf(int idConta, int num)  {

	int i;

	atrasar();

	if (!contaExiste(idConta))  {
		return -1;
    }

	i = contasSaldos[idConta - 1];

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

			saldo = lerSaldoTransf(idConta, 0);

			if (ano != 0)  {

		  		creditarTransf(idConta, saldo*TAXAJURO);

		  		if (saldo < CUSTOMANUTENCAO)

					debitarTransf(idConta, saldo);

		  		else
					debitarTransf(idConta, CUSTOMANUTENCAO);

			saldo = lerSaldoTransf(idConta, 0);

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

    char buf[1024];

	if (!contaExiste(idConta) || !contaExiste(idContaDest) || idConta == idContaDest)  {

        if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d, %d, %d) - ERRO\n", num, COMANDO_TRANSFERIR, idConta, idContaDest, valor)) >= sizeof(buf))  {
            printf("Erro snprintf\n");
        }

        if ((error = write(fd, buf, strlen(buf))) == -1)  {
            perror("write_transferir: ");
        }

		return -1;
    }

	testMutexLock(&mutexContas[min(idConta - 1, idContaDest -1 )]);
	testMutexLock(&mutexContas[max(idConta - 1, idContaDest - 1)]);

	if (debitarTransf(idConta, valor) !=0)  {

        if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d, %d, %d) - ERRO\n", num, COMANDO_TRANSFERIR, idConta, idContaDest, valor)) >= sizeof(buf))  {
            printf("Erro snprintf\n");
        }

        if ((error = write(fd, buf, strlen(buf))) == -1)  {
            perror("write_transferir: ");
        }

		testMutexUnlock(&mutexContas[max(idConta - 1, idContaDest - 1)]);
		testMutexUnlock(&mutexContas[min(idConta - 1, idContaDest - 1)]);

		return -1;
	}

	if ( creditarTransf(idContaDest, valor) != 0)  {

        if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d, %d, %d) - ERRO\n", num, COMANDO_TRANSFERIR, idConta, idContaDest, valor)) >= sizeof(buf))  {
            printf("Erro snprintf\n");
        }

        if ((error = write(fd, buf, strlen(buf))) == -1)  {
            perror("write_transferir: ");
        }

		testMutexUnlock(&mutexContas[max(idConta - 1, idContaDest - 1)]);
		testMutexUnlock(&mutexContas[min(idConta - 1, idContaDest - 1)]);

		return -1;
	}

    if ((numB = snprintf(buf, sizeof(buf), "%d: %s(%d, %d, %d) - OK\n", num, COMANDO_TRANSFERIR, idConta, idContaDest, valor)) >= sizeof(buf))  {
        printf("Erro snprintf\n");
    }

    if ((error = write(fd, buf, strlen(buf))) == -1)  {
        perror("write_transferir: ");
    }

	testMutexUnlock(&mutexContas[max(idConta - 1, idContaDest - 1)]);
	testMutexUnlock(&mutexContas[min(idConta - 1, idContaDest - 1)]);

	return 0;
}



/*************************/
/*       PARTE 2         */
/*************************/

/* Funcao que recebe os 3 argumentos iniciais e os coloca na estrutura comando_t, retornando-a. */

comando_t produzir(int op, int idOri, int val, int idDest, char *nome)  {

  	comando_t i;

  	if (op <= OP_SAIR)  {

	  	i.operacao = op;

	  	i.idConta = idOri;

	  	i.valor = val;

	  	i.idContaDestino = idDest;

	  	strcpy(i.nome, nome);

	}

	else  {

		i.operacao = op;

		i.idConta = idOri;

		i.valor = idDest;

		i.idContaDestino = val;

		strcpy(i.nome, nome);
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

    //printf("thr_consumer\n");

    int t_num;

    t_num = *((int *)arg) + 1;

  	while(1)  {

		comando_t item;

		item = readBuf();

        //printf("Acabei de ler e vou consumir\n");

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

    //printf("cheguei ao consume\n");

    int pipeTerm;
    //printf("check\n");
    if((pipeTerm = open(item.nome, O_WRONLY)) == -1) {
        perror("open do consume: ");
        exit(-1);
    }

    //printf("passei o open %d\n\n", pipeTerm);

	int saldo;
    char text[1024];

	if (item.operacao == OP_LERSALDO)  {

		saldo = lerSaldo(item.idConta, num);

		if (saldo < 0)  {

        	if ((numB = snprintf(text, sizeof(text), "%s(%d): Erro.\n", COMANDO_LER_SALDO, item.idConta)) >= sizeof(text))  {
                printf("Erro snprintf\n");
            }
        }

    	else {

        	if ((numB = snprintf(text, sizeof(text), "%s(%d): O saldo da conta é %d.\n", COMANDO_LER_SALDO, item.idConta, saldo)) >= sizeof(text))  {
                printf("Erro snprintf\n");
            }
        }
	}

	if (item.operacao == OP_CREDITAR)  {

		if (creditar (item.idConta, item.valor, num) < 0)  {

            if ((numB = snprintf(text, sizeof(text), "%s(%d, %d): Erro.\n", COMANDO_CREDITAR, item.idConta, item.valor)) >= sizeof(text))  {
                printf("Erro snprintf\n");
            }
        }

    	else  {
            if ((numB = snprintf(text, sizeof(text), "%s(%d, %d): OK.\n", COMANDO_CREDITAR, item.idConta, item.valor)) >= sizeof(text))  {
                printf("Erro snprintf\n");
            }

        }
    }

	if (item.operacao == OP_DEBITAR)  {

        //printf("entrei no debitar\n");

		if (debitar (item.idConta, item.valor, num) < 0)  {

            if ((numB = snprintf(text, sizeof(text), "%s(%d, %d): Erro.\n", COMANDO_DEBITAR, item.idConta, item.valor)) >= sizeof(text))  {
                printf("Erro snprintf\n");
            }
        }

        else  {

            if ((numB = snprintf(text, sizeof(text), "%s(%d, %d): OK.\n", COMANDO_DEBITAR, item.idConta, item.valor)) >= sizeof(text))  {
                printf("Erro snprintf\n");
            }
        }
	}

	if (item.operacao == OP_TRANSFERIR)  {

		if ( (transferir(item.idConta, item.idContaDestino, item.valor, num) != 0) )  {

            if ((numB = snprintf(text, sizeof(text), "Erro ao transferir valor da conta %d para a conta %d.\n", item.idConta, item.idContaDestino)) >= sizeof(text))  {
                printf("Erro snprintf\n");
            }
        }

		else  {

            if ((numB = snprintf(text, sizeof(text), "transferir(%d, %d, %d): OK.\n", item.idConta, item.idContaDestino, item.valor)) >= sizeof(text))  {
                printf("Erro snprintf\n");
            }
        }
    }

	if (item.operacao == OP_SAIR)  {

        if ((error = close(pipeTerm)) == -1)  {
            perror("close: ");
            exit(EXIT_FAILURE);
        }

		pthread_exit(EXIT_SUCCESS);
	}

    if ((error = write(pipeTerm, text, sizeof(text))) == -1)  {
        perror("write: ");
    }
    //printf("ja mandei os dados\n\n");

    if ((error = close(pipeTerm)) == -1)  {
        perror("close: ");
        exit(EXIT_FAILURE);
    }


	testMutexLock(&mutexCount);

	/* Depois de "consumido" um comando, decrementamos a variavel count,
	 * que conta o numero comandos por executar. */

	count --;

	/* Quando count == 0, e enviado um signal para a variavel pthread_cond_t &cond. */

	if (count == 0)  {

		if ((error = pthread_cond_signal(&cond)) != 0)  {

			errno = error;

			perror("pthread_cond_signal: ");
		}
	}

	testMutexUnlock(&mutexCount);

	return 0;
}


/*****************************************/
/*          FUNCOES AUXILIARES           */
/*****************************************/

/* Funcao testMutexLock verifica se e possivel trancar o mutex, atraves da funcao pthread_mutex_lock().
 * Retorna 0 em caso afirmativo e resulta em erro caso contrario. */

int testMutexLock(pthread_mutex_t *cadeado)  {

	if((error = pthread_mutex_lock(cadeado)) != 0)  {

		errno = error;

		perror("pthread_mutex_lock: ");
	}

	return 0;
}

/* Funcao testMutexUnlock verifica se e possivel destrancar o mutex, atraves da funcao pthread_mutex_unlock().
 * Retorna 0 em caso afirmativo e resulta em erro caso contrario. */

int testMutexUnlock(pthread_mutex_t *cadeado)  {

	if((error = pthread_mutex_unlock(cadeado)) != 0)  {

		errno = error;

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

	if((error = pthread_mutex_destroy(cadeado)) != 0)  {

		errno = error;

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
