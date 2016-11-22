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


<<<<<<< HEAD
=======

>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
#define MAXARGS 4
#define BUFFER_SIZE 100

#define NUM_TRABALHADORAS 3  /*numero de threads*/
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)  /*dimensao do buffer circular*/


int main (int argc, char** argv)  {

    char *args[MAXARGS + 1];

    char buffer[BUFFER_SIZE];

    inicializarContas();

    if ((fd = open("./log.txt",O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO )) == -1)  {
        errno = fd;
        perror("open: ");
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

    int t_num[NUM_TRABALHADORAS]; /*##aux*/

    for(t = 0; t < NUM_TRABALHADORAS; t++)  {

        t_num[t] = t;

        rc = pthread_create(&tid[t], NULL, thr_consumer,(void *) &t_num[t]);

        if(rc != 0)  {

        	errno = rc;

        	perror("pthread_create: ");

        	exit(EXIT_FAILURE);
        }
    }


    printf("Bem-vinda/o ao i-banco\n\n");

    /* A funcao signal() recebe um signal SIGUSR1, predefinido pelo utilizador.
     * Como estamos no processo pai, define-se SIG_IGN para tratar o sinal, ou seja, ignora-o.*/
<<<<<<< HEAD

=======
    
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)

        perror("signal: ");


    while (1)  {

        int numargs;

        numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

        /* EOF (end of file) do stdin ou comando "sair" */

        if  ((numargs < 0) ||
             (numargs > 0 &&
<<<<<<< HEAD
             (strcmp(args[0], COMANDO_SAIR) == 0)))  {
=======
             (strcmp(args[0], COMANDO_SAIR) == 0)))  { 
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93

            if (numargs > 1 && strcmp(args[1], "agora") == 0)

                /* A funcao kill() envia um signal a todos os processos, incluindo o processo pai.*/

                kill(0, SIGUSR1);

        	printf("i-banco vai terminar.\n--\n");

    	    int i;
<<<<<<< HEAD

    	    comando_t input;

    	    for(i = 0; i < NUM_TRABALHADORAS; i++)  {

                input = produzir(OP_SAIR, -1, -1, -1);

=======

    	    comando_t input;

    	    for(i = 0; i < NUM_TRABALHADORAS; i++)  {
    		
                input = produzir(OP_SAIR, -1, -1, -1);

>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
                writeBuf(input);
            }

    	    for(i = 0; i < NUM_TRABALHADORAS; i++)  {
<<<<<<< HEAD

=======
    		
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
                if((rc = pthread_join(tid[i], NULL)) != 0)  {

                    errno = rc;

                    perror("pthread_join: ");
<<<<<<< HEAD

                    exit(EXIT_FAILURE);
                }
            }


=======

                    exit(EXIT_FAILURE);
                }
            }   	
        
          	
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
            int estado;
            pid_t test;

            while (1)  {

<<<<<<< HEAD
                 /* A funcao wait() aguarda que um processo filho termine.
                 * Em caso de sucesso, devolve o PID do processo filho terminado.
                 * Em caso de erro, devolve -1 e devolve para a variavel errno o codigo do erro.
                 * O erro ECHILD ocorre quando ja nao ha mais processos filho.*/

                if ((test = wait(&estado)) == -1)  {

                    if (errno == ECHILD)

                        break;

                    else

=======
                 /* A funcao wait() aguarda que um processo filho termine. 
                 * Em caso de sucesso, devolve o PID do processo filho terminado.
                 * Em caso de erro, devolve -1 e devolve para a variavel errno o codigo do erro.  
                 * O erro ECHILD ocorre quando ja nao ha mais processos filho.*/

                if ((test = wait(&estado)) == -1)  {

                    if (errno == ECHILD)

                        break;

                    else

>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
                        perror("wait: ");
                }

                if (WIFEXITED(estado) > 0)

                    printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n", test);


                if (WIFSIGNALED(estado) != 0)

                    printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n", test);
            }
<<<<<<< HEAD

       	    printf("--\ni-banco terminou.\n");

=======
 
       	    printf("--\ni-banco terminou.\n");
            
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
            for (i = 0; i < NUM_CONTAS; i++)  {

                testMutexDestroy(&mutexContas[i]);
            }

            if ((rc = pthread_cond_destroy(&cond)) != 0)  {
<<<<<<< HEAD

                errno = rc;

                perror("pthread_cond_destroy: ");
            }

            testMutexDestroy(&cadeadoC);
            testMutexDestroy(&mutexCount);
            testSemDestroy(&escrita);
            testSemDestroy(&leitura);

=======

                errno = rc;

                perror("pthread_cond_destroy: ");
            }

            testMutexDestroy(&cadeadoC);
            testMutexDestroy(&mutexCount);
            testSemDestroy(&escrita);
            testSemDestroy(&leitura);
               
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
            exit(EXIT_SUCCESS);
        }


        else if (numargs == 0)

            /* Nenhum argumento; ignora e volta a pedir */

            continue;


        /* Debitar */

        else if (strcmp(args[0], COMANDO_DEBITAR) == 0)  {

        	if (numargs < 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);

                continue;
            }

            comando_t input;

            input = produzir(OP_DEBITAR, atoi(args[1]), atoi(args[2]), -1);

            writeBuf(input);
        }

        /* Creditar */

        else if (strcmp(args[0], COMANDO_CREDITAR) == 0)  {

            if (numargs < 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);

                continue;
            }

            comando_t input;

            input = produzir(OP_CREDITAR, atoi(args[1]), atoi(args[2]), -1);

            writeBuf(input);

        }

        /* Ler Saldo */

        else if (strcmp(args[0], COMANDO_LER_SALDO) == 0)  {

            if (numargs < 2)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);

                continue;
            }

            comando_t input;

            input = produzir(OP_LERSALDO, atoi(args[1]), -1, -1);
<<<<<<< HEAD

            writeBuf(input);
        }

        /* Transferir */

        else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0)  {

            if (numargs <= 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_TRANSFERIR);

                continue;
            }

            comando_t input;

            input = produzir(OP_TRANSFERIR, atoi(args[1]), atoi(args[2]), atoi(args[3]));
=======
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93

            writeBuf(input);
        }

<<<<<<< HEAD
=======
        /* Transferir */

        else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0)  {
        	
            if (numargs <= 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_TRANSFERIR);

                continue;
            }

            comando_t input;

            input = produzir(OP_TRANSFERIR, atoi(args[1]), atoi(args[2]), atoi(args[3]));

            writeBuf(input);
        }

>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93

        /* Simular */

        else if (strcmp(args[0], COMANDO_SIMULAR) == 0)  {

            int numAnos;
<<<<<<< HEAD

=======
            
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
            int pid;

            if (numargs < 2)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);

                continue;
            }

            numAnos = atoi(args[1]) + 1;

            testMutexLock(&mutexCount);
<<<<<<< HEAD

            /* Enquanto count != 0, o comando simular fica em espera de um
             * signal enviado para a variavel pthread_cond_t &cond. */

            while (!(count == 0))  {

=======
            
            /* Enquanto count != 0, o comando simular fica em espera de um 
             * signal enviado para a variavel pthread_cond_t &cond. */

            while (!(count == 0))  {
            	
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
            	if ((rc = pthread_cond_wait(&cond, &mutexCount)) != 0)  {

                    errno = rc;

                    perror("pthread_cond_wait: ");
                }
            }

            testMutexUnlock(&mutexCount);

<<<<<<< HEAD
            /* A funcao fork() cria um processo filho.
=======
            /* A funcao fork() cria um processo filho. 
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
             * Se devolver 0, signifca que estamos no processo filho. */

            pid = fork();

            if (pid == -1)  {

                perror("fork: ");
            }

<<<<<<< HEAD

            /* A funcao signal() define a funcao handler() como a funcao que processa
             * o signal SIGUSR1, que e definido pelo utilizador. */
=======
            /* A funcao signal() define a funcao handler() como a funcao que processa
             * o signal SIGUSR1, que e definido pelo utilizador. */

            if (signal(SIGUSR1,handler) == SIG_ERR)

                    perror("signal: ");

            if (pid == 0)  {

                /* Processo filho. */
>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93

            if (signal(SIGUSR1,handler) == SIG_ERR)

                    perror("signal: ");

            if (pid == 0)  {

                /* Processo filho. */
                close(fd);
                simular(numAnos);

                exit(EXIT_SUCCESS);
            }

            /* Processo pai. */

            continue;
        }

        else

<<<<<<< HEAD
            printf("Comando desconhecido. Tente de novo.\n");

    }
}
=======
            printf("Comando desconhecido. Tente de novo.\n"); 

    }
}





>>>>>>> 5cee5a16e8d6a5daa42b8322241d78a44fcaef93
