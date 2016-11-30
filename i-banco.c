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
    	perror("open: ");
        exit(-1);
    }

    if ((fd = open("/tmp/log.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO )) == -1)  {
        perror("open: ");
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

        //printf("entrei no while\n");

        //int numargs;

        //printf("vou chegar ao numargs\n");

        //numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

        //printf("passei o numargs\n");
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


        /*if (comparar sair ou sair agora)

            if(sair agora)

        else if(simular)

        else if(threads trabalhadoras)

        else (comando desconhecido)*/












        /* EOF (end of file) do stdin ou comando "sair" */

       	if  (comando.operacao == OP_SAIR)  {

            if (comando.operacao == OP_SAIR_AGORA)  {

                /* A funcao kill() envia um signal a todos os processos, incluindo o processo pai.*/

                kill(0, SIGUSR1);
            }

        	printf("i-banco vai terminar.\n--\n");

    	    int i;
            comando_t input;
    	    for(i = 0; i < NUM_TRABALHADORAS; i++)  {

                input = produzir(OP_SAIR, -1, -1, -1, "sair");
                writeBuf(input);
            }

            int estado;
            pid_t test;

            while (1)  {


                 /* A funcao wait() aguarda que um processo filho termine.
                 * Em caso de sucesso, devolve o PID do processo filho terminado.
                 * Em caso de erro, devolve -1 e devolve para a variavel errno o codigo do erro.
                 * O erro ECHILD ocorre quando ja nao ha mais processos filho.*/

                if ((test = wait(&estado)) == -1)  {

                    if (errno == ECHILD)

                        break;

                    else

                        perror("wait: ");
                }

                if (WIFEXITED(estado) > 0)

                    printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n", test);


                if (WIFSIGNALED(estado) != 0)

                    printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n", test);
            }

       	    printf("--\ni-banco terminou.\n");

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

            if (signal(SIGUSR1,handler) == SIG_ERR)
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
                    perror("open: ");
                }

                aux = dup2(newF, 1);

                //printf("vou simular\n");

                simular(numAnos);
                //printf("acabei de simular\n");
                close(aux);
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
