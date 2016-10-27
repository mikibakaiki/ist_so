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



#define MAXARGS 3
#define BUFFER_SIZE 100

#define NUM_TRABALHADORAS 3  /*numero de threads*/
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)  /*dimensao do buffer circular*/



int main (int argc, char** argv)  {

    char *args[MAXARGS + 1];

    char buffer[BUFFER_SIZE];

    inicializarContas();

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

    for(t = 0; t < NUM_TRABALHADORAS; t++)  {

        rc = pthread_create(&tid[t], NULL, thr_consumer, NULL);

        if(rc != 0)  {
        	errno = rc;
        	perror("pthread_create: ");
        	exit(EXIT_FAILURE);
        }
    }



    printf("Bem-vinda/o ao i-banco\n\n");

    /* A funcao signal() recebe um signal SIGUSR1, predefinido pelo utilizador.
     * Como estamos no processo pai, define-se SIG_IGN para tratar o sinal, ou seja, ignora-o.*/
    
    signal(SIGUSR1, SIG_IGN);

    while (1)  {
        
        int numargs;
    
        numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

        /* EOF (end of file) do stdin ou comando "sair" */

        if  ((numargs < 0) ||
             (numargs > 0 &&
             (strcmp(args[0], COMANDO_SAIR) == 0)) ||
             ((numargs > 1) && (strcmp(args[0], COMANDO_SAIR) == 0) && (strcmp(args[1], "agora") == 0)))  {

			sleep(5);
        	printf("i-banco vai terminar.\n--\n");

        	int i;
        	comando_t input;

        	

        	for(i = 0; i < NUM_TRABALHADORAS; i++)  {
        		
            	input = produzir(OP_SAIR, -1, -1);
     
            	writeBuf(input);
            }

        	for(i = 0; i < NUM_TRABALHADORAS; i++)  {
        		
        		if((rc = pthread_join(tid[i], NULL)) != 0)  {

        			errno = rc;

        			perror("pthread_join: ");

        			exit(EXIT_FAILURE);
        		}
        	}
          	
            /* A funcao kill() envia um signal a todos os processos, incluindo o processo pai.*/

            /*kill(0, SIGUSR1);*/
            
            /*int estado;
            pid_t test;
            
            while (1)  {

                 * A funcao wait() aguarda que um processo filho termine. 
                 * Em caso de sucesso, devolve o PID do processo filho terminado.
                 * Em caso de erro, devolve -1 e devolve para a variavel errno o codigo do erro.  
                 * O erro ECHILD ocorre quando ja nao ha mais processos filho.*

                test = wait(&estado);

                if (test == -1 && errno == ECHILD)

                    break;

                if (WIFEXITED(estado) > 0)

                    printf("FILHO TERMINADO (PID=%d; terminou normalmente)\n", test);
                

                if (WIFSIGNALED(estado) != 0)

                    printf("FILHO TERMINADO (PID=%d; terminou abruptamente)\n", test);
            }*/

            printf("--\ni-banco terminou.\n");
               
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

            input = produzir(OP_DEBITAR, atoi(args[1]), atoi(args[2]));

            writeBuf(input);
        }

        /* Creditar */

        else if (strcmp(args[0], COMANDO_CREDITAR) == 0)  {

            if (numargs < 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);

                continue;
            }

            comando_t input;

            input = produzir(OP_CREDITAR, atoi(args[1]), atoi(args[2]));

            writeBuf(input);

        }

        /* Ler Saldo */

        else if (strcmp(args[0], COMANDO_LER_SALDO) == 0)  {

            if (numargs < 2)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);

                continue;
            }

            comando_t input;

            input = produzir(OP_LERSALDO, atoi(args[1]), -1);

            writeBuf(input);
        }

        /* Simular */

        else if (strcmp(args[0], COMANDO_SIMULAR) == 0)  {

            int numAnos;
            int pid;

            if (numargs < 2)  {

                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);

                continue;
            }

            numAnos = atoi(args[1]) + 1;

            /* A funcao fork() cria um processo filho. 
             * Se devolver 0, signifca que estamos no processo filho.*/

            pid = fork();

            if (pid == 0)  {

                /* A funcao signal() define a funcao handler() como a funcao que processa
                 * o signal SIGUSR1, que e definido pelo utilizador.*/

                signal(SIGUSR1,handler);

                simular(numAnos);

                exit(0);
            }

            continue;
        }

        else

            printf("Comando desconhecido. Tente de novo.\n"); 
    }
}




