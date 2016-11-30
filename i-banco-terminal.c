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


#define MAXARGS 4
#define BUFFER_SIZE 1024

#define NUM_TRABALHADORAS 3  /*numero de threads*/
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)  /*dimensao do buffer circular*/



int main(int argc, char** argv)  {

	char *args[MAXARGS + 1];

    char buffer[BUFFER_SIZE];

	char output[BUFFER_SIZE];

    inicializarContas();

    char paipeName[1024];

    int paipe;

    if ((paipe = snprintf(paipeName, sizeof(paipeName), "/tmp/pipe-terminal-%d", getpid())) >= sizeof(paipeName))  {
        printf("Erro snprintf\n");
    }

	printf("pipe name: %s\n", paipeName);
    int pip;

	unlink(paipeName);  /*Falta fazer o erro*/

    if((pip = mkfifo(paipeName, 0777)) == -1)  {

    	perror("mkfifo: ");
    }

    int fdPipeP;

    if ((fdPipeP = open(argv[1], O_WRONLY)) == -1)  {

    	perror("open: ");
    }

    int rc;

    printf("Bem-vinda/o ao i-banco\n\n");

    /* A funcao signal() recebe um signal SIGUSR1, predefinido pelo utilizador.
     * Como estamos no processo pai, define-se SIG_IGN para tratar o sinal, ou seja, ignora-o.*/

    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)

        perror("signal: ");

    while (1)  {

        int numargs;

        numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

        /* EOF (end of file) do stdin ou comando "sair" */

        if  ((numargs < 0) ||
             (numargs > 0 &&
             (strcmp(args[0], COMANDO_SAIR) == 0)))  {


            //if (numargs > 1 && strcmp(args[1], "agora") == 0)  {

            /*A funcao kill() envia um signal a todos os processos, incluindo o processo pai.*/

            	//kill(0, SIGUSR1);
            //}

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


        else if (numargs == 0)

            /* Nenhum argumento; ignora e volta a pedir */

            continue;


        /* Debitar */

        else if (strcmp(args[0], COMANDO_DEBITAR) == 0)  {

        	if (numargs < 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_DEBITAR);

                continue;
            }
            int error;

            comando_t input;

            input = produzir(OP_DEBITAR, atoi(args[1]), atoi(args[2]), -1, paipeName);

			//printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

            if((error = write(fdPipeP, &input, sizeof(comando_t))) == -1)  {

            	perror("write: ");
            }

			int fdPipeT;

		    if ((fdPipeT = open(paipeName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(fdPipeT, output, BUFFER_SIZE);

			if((error = close(fdPipeT)) == -1)  {
				perror("close: ");
			}

			printf("%s", output);


            //printf("Processei o comando e pus no buffer\n");
        }

        /* Creditar */

        else if (strcmp(args[0], COMANDO_CREDITAR) == 0)  {

            if (numargs < 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_CREDITAR);

                continue;
            }

            comando_t input;
			int error;

            input = produzir(OP_CREDITAR, atoi(args[1]), atoi(args[2]), -1, paipeName);

			//printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

            if((error = write(fdPipeP, &input, sizeof(comando_t))) == -1)  {

            	perror("write: ");
            }
			int fdPipeT;
			if ((fdPipeT = open(paipeName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(fdPipeT, output, BUFFER_SIZE);

			if((error = close(fdPipeT)) == -1)  {
				perror("close: ");
			}
			printf("%s", output);


        }

        /* Ler Saldo */

        else if (strcmp(args[0], COMANDO_LER_SALDO) == 0)  {

            if (numargs < 2)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_LER_SALDO);

                continue;
            }

            comando_t input;
			int error;

            input = produzir(OP_LERSALDO, atoi(args[1]), -1, -1, paipeName);

			//printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

            if((error = write(fdPipeP, &input, sizeof(comando_t))) == -1)  {

            	perror("write: ");
            }

			int fdPipeT;

			if ((fdPipeT = open(paipeName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(fdPipeT, output, BUFFER_SIZE);

			if((error = close(fdPipeT)) == -1)  {
				perror("close: ");
			}
			printf("%s", output);

        }

        /* Transferir */

        else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0)  {

            if (numargs <= 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_TRANSFERIR);

                continue;
            }

            comando_t input;
			int error;

            input = produzir(OP_TRANSFERIR, atoi(args[1]), atoi(args[2]), atoi(args[3]), paipeName);

			//printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

            if((error = write(fdPipeP, &input, sizeof(comando_t))) == -1)  {

            	perror("write: ");
            }

			int fdPipeT;

			if ((fdPipeT = open(paipeName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(fdPipeT, output, BUFFER_SIZE);

			if((error = close(fdPipeT)) == -1)  {
				perror("close: ");
			}
			printf("%s", output);

        }
		/* Simular */

		else if (strcmp(args[0], COMANDO_SIMULAR) == 0)  {

            if (numargs < 2)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_SIMULAR);

                continue;
            }

            comando_t input;
			int error;

            input = produzir(OP_SIMULAR, -1, atoi(args[1]), -1, paipeName);

			printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

            if((error = write(fdPipeP, &input, sizeof(comando_t))) == -1)  {

            	perror("write: ");
            }

			/*int fdPipeT;

			if ((fdPipeT = open(paipeName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(fdPipeT, output, BUFFER_SIZE);

			if((error = close(fdPipeT)) == -1)  {
				perror("close: ");
			}*/

        }


		/* Sair Terminal */

		else if (strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0)  {

			int error;

			if((error = unlink(paipeName)) == -1)  {

				perror("unlink: ");
			}

			if((error = close(fdPipeP)) == -1)  {
				perror("close: ");
			}

			printf("--\ni-banco-terminal terminou.\n--\n");
			exit(EXIT_SUCCESS);
		}

        else

            printf("Comando desconhecido. Tente de novo.\n");

    }
}
