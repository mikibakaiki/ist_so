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

#include "commandlinereader.h"
#include "contas.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 100

#define NUM_TRABALHADORAS 3  /*numero de threads*/
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)  /*dimensao do buffer circular*/


int main (int argc, char** argv)  {

    int numFilhos = 0;

    inicializarContas();

    int error;                          /* Variavel que testa erros */
    int pipeD;                          /* /tmp/i-banco-pipe pipe descriptor */

    pthread_t tid[NUM_TRABALHADORAS];   /* Vector que guarda as ids das threads */
    int t_num[NUM_TRABALHADORAS];       /* Vector que guarda o numero de threads */


    printf("A espera de conexao.\n\n");

    /* Caso ja exista ficheiro com o caminho indicado, remove-o. */

    if ((error = unlink("/tmp/i-banco-pipe")) == -1 && errno != ENOENT)  {
        perror("unlink i-banco-pipe: ");
    }

    /* Cria um pipe com o nome contido no primeiro argumento. */

    if ((error = mkfifo("/tmp/i-banco-pipe", 0777)) == -1)  {
    	perror("mkfifo: ");
        exit(EXIT_FAILURE);
    }

    /* Abre o ficheiro cuja directoria e dada como argumento. */

    if ((pipeD = open("/tmp/i-banco-pipe", O_RDONLY)) == -1)  {
    	perror("open i-banco-pipe i-banco.c: ");
        exit(EXIT_FAILURE);
    }

    printf("Bem-vinda/o ao i-banco\n\n");

    /* Abre o ficheiro cuja directoria e dada como argumento. */

    if ((fd = open("/tmp/log.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO )) == -1)  {
        perror("open /tmp/log.txt no i.banco.c: ");
        exit(EXIT_FAILURE);
    }

    if ((error = pthread_mutex_init(&cadeadoC, NULL)) != 0)  {
        errno = error;
        perror("pthread_mutex_init: ");
    }

 	if (sem_init(&escrita, 1, CMD_BUFFER_DIM) == -1)  {
     	perror("sem_init: ");
 	}

    if (sem_init(&leitura, 1, 0) == -1)  {
     	perror("sem_init: ");
    }

    int t;

    for(t = 0; t < NUM_TRABALHADORAS; t++)  {

        t_num[t] = t;

        error = pthread_create(&tid[t], NULL, thr_consumer,(void *) &t_num[t]);

        if(error != 0)  {
        	errno = error;
        	perror("pthread_create: ");
        	exit(EXIT_FAILURE);
        }
    }


    /* A funcao signal() recebe um signal SIGUSR1, predefinido pelo utilizador.
     * Como estamos no processo pai, define-se SIG_IGN para tratar o sinal, ou seja, ignora-o.*/

    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)
        perror("signal: ");


    while (1)  {

        comando_t comando;

        int errClose;

        error = read(pipeD, &comando, sizeof(comando_t));

        /* Caso a funcao read() retorne zero, fecha-se o pipe e volta a abrir-se, e fica a espera de uma nova ligacao. */

        if (error == 0)  {

            if((errClose = close(pipeD)) == -1)  {
                perror("close: ");
                exit(EXIT_FAILURE);
            }

            printf("Todos os terminais foram fechados.\n");
            printf("Abra um i-banco-terminal para continuar a executar.\n\n");

            if((pipeD = open("/tmp/i-banco-pipe", O_RDONLY) ) == -1)  {
                perror("open: ");
                exit(EXIT_FAILURE);
            }
            continue;
        }

        else if (error == -1)
            perror("read: ");


        if (comando.operacao == OP_LERSALDO || comando.operacao == OP_CREDITAR || comando.operacao == OP_DEBITAR || comando.operacao == OP_TRANSFERIR)  {
            writeBuf(comando);
        }

       	else if (comando.operacao == OP_SAIR)  {

            if (comando.valor == -2)  {

                /* A funcao kill() envia um signal a todos os processos, incluindo o processo pai.*/

                kill(0, SIGUSR1);

            }

            printf("--\ni-banco vai terminar.\n\n");

    	    int i;

    	    for(i = 0; i < NUM_TRABALHADORAS; i++)  {
                writeBuf(comando);
            }

            int estado;
            pid_t test;

            while (numFilhos > 0)  {
                /* A funcao wait() aguarda que um processo filho termine.
                 * Em caso de sucesso, devolve o PID do processo filho terminado.
                 * Em caso de erro, devolve -1 e devolve para a variavel errno o codigo do erro.
                 * O erro EINTR significa que chegou um signal que interrompeu a espera pela terminacao
                 * do filho; voltamos a esperar. */

                if ((test = wait(&estado)) < 0)  {
                    if (errno == EINTR)  {
                        continue;
                    }

                    else  {
                        perror("wait: ");
                        exit(EXIT_FAILURE);
                    }
                }

                numFilhos--;

                if (WIFEXITED(estado))
                    printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n", test);
                else
                    printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n", test);
            }

            for (i = 0; i < NUM_CONTAS; i++)  {

                testMutexDestroy(&mutexContas[i]);
            }

            if ((error = pthread_cond_destroy(&cond)) != 0)  {

                errno = error;

                perror("pthread_cond_destroy: ");
            }

            testMutexDestroy(&cadeadoC);
            testMutexDestroy(&mutexCount);
            testSemDestroy(&escrita);
            testSemDestroy(&leitura);


            if((error = close(pipeD)) == -1)  {
                perror("close: ");
                exit(EXIT_FAILURE);
            }

            if ((error = unlink("/tmp/i-banco-pipe")) == -1 && errno != ENOENT)  {
                perror("unlink i-banco-pipe: ");
            }

            printf("\ni-banco terminou.\n--\n");

            exit(EXIT_SUCCESS);
        }

        else if (comando.operacao == -1)

            /* Nenhum argumento; ignora e volta a pedir */

            continue;


        /* Simular */

        else if (comando.operacao == OP_SIMULAR)  {

            int numAnos;
            int pid;
            numAnos = comando.valor + 1;

            testMutexLock(&mutexCount);
            /* Enquanto count != 0, o comando simular fica em espera de um
             * signal enviado para a variavel pthread_cond_t &cond. */

            while (!(count == 0))  {
            	if ((error = pthread_cond_wait(&cond, &mutexCount)) != 0)  {
                    errno = error;
                    perror("pthread_cond_wait: ");
                }
            }

            testMutexUnlock(&mutexCount);

            /* A funcao fork() cria um processo filho.
             * Se devolver 0, signifca que estamos no processo filho. */

            pid = fork();

            if (pid == -1)  {
                perror("fork: ");
            }

            /* A funcao signal() define a funcao handler() como a funcao que processa
             * o signal SIGUSR1, que e definido pelo utilizador. */

            if (signal(SIGUSR1, handler) == SIG_ERR)
                perror("signal: ");

            if (pid == 0)  {

                /* Processo filho. */

                if((error = close(fd)) == -1)  {
                    perror("close: ");
                    exit(EXIT_FAILURE);
                }

                /* close(1) fecha a entrada 1 da tabela de ficheiros abertos do processo filho.
                 * Esta e a razao pela qual nao utilizamos o dup2. */

                if((error = close(1)) == -1)  {
                    perror("close: ");
                    exit(EXIT_FAILURE);
                }

                int numB;               /* Variavel que guarda numero de bytes escritos */
                char fileName[1024];    /* Vector que guarda uma string. */

                /* Escreve para o vector pipeTerminalName uma string personalizada. */

                if ((numB = snprintf(fileName, sizeof(fileName), "/tmp/i-banco-sim-%d.txt", getpid())) >= sizeof(fileName))  {
                    printf("Erro snprintf\n");
                }

                /* Abre o ficheiro com nome fileName. */

                if ((newF = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO )) == -1)  {
                    perror("open do processo filho: ");
                }
                /* Caso nao estivesse um close(1) no inicio do processo filho, este dup2 tinha que estar aqui.
                 * No entanto, fica comentado. */

                // if ((error = dup2(newF, 1)) == -1)  {
                //     perror("dup2: ");
                // }

                simular(numAnos);

                /* Depois de simular e de escrever para o ficheiro indicado, fechamo-lo. */

                if((error = close(newF)) == -1)  {
                    perror("close: ");
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);
            }

            /* Processo pai. */

            if (comando.valor > 1 || comando.valor == 0)  {
                printf("Simulacao de %d anos iniciada em background\n", comando.valor);
            }

            else
                printf("Simulacao de %d ano iniciada em background\n", comando.valor);

            pidFilhos[numFilhos] = pid;
            numFilhos ++;
            continue;
        }

        else if(comando.operacao > 6)

            printf("Comando desconhecido. Tente de novo.\n");

    }
}
