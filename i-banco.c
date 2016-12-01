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

    //char *args[MAXARGS + 1];

    // char buffer[BUFFER_SIZE];

    int numFilhos = 0;

    inicializarContas();

    int pip;

    printf("Bem-vinda/o ao i-banco\n\n");

    unlink("/tmp/i-banco-pipe");  /*falta ver o erro!*/

    if((pip = mkfifo("/tmp/i-banco-pipe", 0777)) == -1)  {
    	perror("mkfifo: ");
        exit(-1);
    }

    int paipe;

    if((paipe = open("/tmp/i-banco-pipe", O_RDONLY) ) == -1)  {
    	perror("open i-banco-pipe i-banco.c: ");
        exit(-1);
    }

    if ((fd = open("/tmp/log.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO )) == -1)  {
        perror("open log.txt i.banco.c: ");
        exit(-1);
    }

    int rc, t;

    if ((rc = pthread_mutex_init(&cadeadoC, NULL)) != 0)  {

        errno = rc;

        perror("pthread_mutex_init: ");
    }

 	if (sem_init(&escrita, 1, CMD_BUFFER_DIM) == -1)  {

     	perror("sem_init: ");

 	}

    if (sem_init(&leitura, 1, 0) == -1)  {

     	perror("sem_init: ");
    }

    pthread_t tid[NUM_TRABALHADORAS];

    int t_num[NUM_TRABALHADORAS]; /*aux*/

    for(t = 0; t < NUM_TRABALHADORAS; t++)  {

        t_num[t] = t;

        rc = pthread_create(&tid[t], NULL, thr_consumer,(void *) &t_num[t]);

        if(rc != 0)  {

        	errno = rc;

        	perror("pthread_create: ");

        	exit(EXIT_FAILURE);
        }
    }


    /* A funcao signal() recebe um signal SIGUSR1, predefinido pelo utilizador.
     * Como estamos no processo pai, define-se SIG_IGN para tratar o sinal, ou seja, ignora-o.*/

    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)
        perror("signal: ");


    while (1)  {

        int error;

        comando_t comando;

        //printf("vou receber um comando\n");
        int errClose;

        error = read(paipe, &comando, sizeof(comando_t));

        if (error == 0)  {

            if((errClose = close(paipe)) == -1)  {
                perror("close: ");
                exit(EXIT_FAILURE);
            }

            if((paipe = open("/tmp/i-banco-pipe", O_RDONLY) ) == -1)  {
                perror("open: ");
                exit(EXIT_FAILURE);
            }
            continue;
        }

        else if(error == -1)
            perror("read: ");


        //printf("recebi um comando\n");

        //printf("recebi o comando e vou escreve-lo\n");

        printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n\n", comando.operacao, comando.idConta, comando.valor, comando.nome);

        if (comando.operacao == OP_LERSALDO || comando.operacao == OP_CREDITAR || comando.operacao == OP_DEBITAR || comando.operacao == OP_TRANSFERIR)  {
            writeBuf(comando);
        }

       	if (comando.operacao == OP_SAIR)  {

            if (comando.valor == -2)  {

                /* A funcao kill() envia um signal a todos os processos, incluindo o processo pai.*/

                kill(0, SIGUSR1);

            }

            printf("--\ni-banco vai terminar.\n--\n");

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
                // printf("%d\n\n", numFilhos);
                if (WIFEXITED(estado))
                    printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n", test);
                else
                    printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n", test);
            }

            for (i = 0; i < NUM_CONTAS; i++)  {

                testMutexDestroy(&mutexContas[i]);
            }

            if ((rc = pthread_cond_destroy(&cond)) != 0)  {

                errno = rc;

                perror("pthread_cond_destroy: ");
            }

            testMutexDestroy(&cadeadoC);
            testMutexDestroy(&mutexCount);
            testSemDestroy(&escrita);
            testSemDestroy(&leitura);

            printf("--\ni-banco terminou.\n--\n");

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
            	if ((rc = pthread_cond_wait(&cond, &mutexCount)) != 0)  {
                    errno = rc;
                    perror("pthread_cond_wait: ");
                }
            }

            testMutexUnlock(&mutexCount);

            /* A funcao fork() cria um processo filho.
             * Se devolver 0, signifca que estamos no processo filho. */

            pid = fork();

            //printf("PID = %d\n\n", pid);

            if (pid == -1)  {
                perror("fork: ");
            }

            /* A funcao signal() define a funcao handler() como a funcao que processa
             * o signal SIGUSR1, que e definido pelo utilizador. */

            if (signal(SIGUSR1, handler) == SIG_ERR)
                perror("signal: ");

            if (pid == 0)  {

                /* Processo filho. */

                close(fd);
                close(1);

                int fileDes, aux;
                char fileName[1024];

                if ((fileDes = snprintf(fileName, sizeof(fileName), "/tmp/i-banco-sim-%d.txt", getpid())) >= sizeof(fileName))  {
                    printf("Erro sprintf\n");
                }

                if ((newF = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO )) == -1)  {
                    perror("open do processo filho: ");
                }
                printf("newf e %d\n\n", newF);
                aux = dup2(newF, 1);
                printf("aux e %d\n\n", aux);

                //printf("vou simular\n");

                simular(numAnos);
                //printf("acabei de simular\n");
                close(aux);  /*tenho que fechar este e o newF ??? */
                close(newF);
                exit(EXIT_SUCCESS);
            }

            /* Processo pai. */
            printf("Simulacao de %d anos iniciada em background\n", comando.valor);

            pidFilhos[numFilhos] = pid;
            numFilhos ++;
            continue;
        }

        else if(comando.operacao > 6)

            printf("Comando desconhecido. Tente de novo.\n");

    }
}
