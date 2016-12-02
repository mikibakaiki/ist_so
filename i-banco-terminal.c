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


#define MAXARGS 4
#define BUFFER_SIZE 1024

#define NUM_TRABALHADORAS 3  /*numero de threads*/
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)  /*dimensao do buffer circular*/



int main(int argc, char** argv)  {

	char *args[MAXARGS + 1];

    char buffer[BUFFER_SIZE];

	char output[BUFFER_SIZE];

    inicializarContas();

    char pipeTerminalName[1024];	/* Vector que guarda o nome do pipe /tmp/pipe-terminal */
    int numB;	 					/* Variavel que guarda numero de bytes escritos */
	int error;						/* Variavel que testa erros */
	int pipeD;						/* /tmp/i-banco-pipe pipe descriptor */
	int pipeTerm;					/* /tmp/pipe-terminal-PID pipe descriptor */


	/* Se o i-banco-terminal receber um signal SIGPIPE, ignora-o. */
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)  {

		perror("signal SIGPIPE: ");
	}

	/* Escreve para o vector pipeTerminalName uma string personalizada. */
    if ((numB = snprintf(pipeTerminalName, sizeof(pipeTerminalName), "/tmp/pipe-terminal-%d", getpid())) >= sizeof(pipeTerminalName))  {
        printf("Erro snprintf\n");
    }

	/* Caso ja exista ficheiro com o caminho indicado, remove-o. */
	if ((error = unlink(pipeTerminalName)) == -1 && errno != ENOENT)  {
        perror("unlink /tmp/pipe-terminal: ");
    }

	/* Cria um pipe com o nome contido em pipeTerminalName. */
    if ((error = mkfifo(pipeTerminalName, 0777)) == -1)  {

    	perror("mkfifo: ");
    }

	/* Abre o ficheiro cuja directoria e dada como argumento. */
    if ((pipeD = open(argv[1], O_WRONLY)) == -1)  {

    	perror("open: ");
    }

    printf("Bem-vinda/o ao i-banco-terminal\n\n");

    /* A funcao signal() recebe um signal SIGUSR1, predefinido pelo utilizador.
     * Como estamos no processo pai, define-se SIG_IGN para tratar o sinal, ou seja, ignora-o.*/

    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)

        perror("signal: ");

    while (1)  {

        int numargs;
		comando_t input;

        numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

        /* EOF (end of file) do stdin ou comando "sair" */

        if  ((numargs < 0) ||
             (numargs > 0 &&
             (strcmp(args[0], COMANDO_SAIR) == 0)))  {

            if (numargs > 1 && strcmp(args[1], "agora") == 0)  {

	            input = produzir(OP_SAIR, -2, -2, -2, pipeTerminalName);

				/* Caso a funcao write() de erro, e esse erro for EPIPE (a ponta do pipe dedicada a ler esta fechada)
				 * remove-se o ficheiro com o caminho indicado, fecha-se o seu file descriptor e o i-banco-terminal encerra. */

				if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

					if (errno == EPIPE)  {

						printf("\nPerdida conexao com i-banco.\n");

						printf("O i-banco-terminal vai encerrar.\n\n");

						if((error = unlink(pipeTerminalName)) == -1)  {

							perror("unlink: ");
						}

						if ((error = close(pipeD)) == -1)  {
							perror("close: ");
							exit(EXIT_FAILURE);
						}

						printf("--\ni-banco-terminal terminou.\n--\n");
						exit(EXIT_SUCCESS);
					}
	            	perror("write: ");
	        	}
			}

			input = produzir(OP_SAIR, -1, -1, -1, pipeTerminalName);

			/* Ver linhas 109 e 110. */

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("\nPerdida conexao com i-banco.\n");

					printf("O i-banco-terminal vai encerrar.\n\n");

					if((error = unlink(pipeTerminalName)) == -1)  {

						perror("unlink: ");
					}

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					printf("--\ni-banco-terminal terminou.\n--\n");
					exit(EXIT_SUCCESS);
				}
				perror("write: ");
			}
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

            input = produzir(OP_DEBITAR, atoi(args[1]), atoi(args[2]), -1, pipeTerminalName);

			/* Ver linhas 109 e 110. */

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("\nPerdida conexao com i-banco.\n");

					printf("O i-banco-terminal vai encerrar.\n\n");

					if((error = unlink(pipeTerminalName)) == -1)  {

						perror("unlink: ");
					}

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					printf("--\ni-banco-terminal terminou.\n--\n");
					exit(EXIT_SUCCESS);
				}
				perror("write: ");
			}

			time(&start);

			/* O pipe de leitura e aberto*/

		    if ((pipeTerm = open(pipeTerminalName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			/* Efectuada leitura do conteudo proveniente do pipe que recebe a resposta do i-banco. */

			if ((error = read(pipeTerm, output, BUFFER_SIZE)) == -1)  {
				perror("read_debitar: ");
			}

			time(&end);

			diff = difftime(end, start);

			if((error = close(pipeTerm)) == -1)  {
				perror("close: ");
			}

			printf("%s", output);
			printf("Tempo de execucao: %f\n\n", diff);
        }

        /* Creditar */

        else if (strcmp(args[0], COMANDO_CREDITAR) == 0)  {

            if (numargs < 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_CREDITAR);

                continue;
            }

            input = produzir(OP_CREDITAR, atoi(args[1]), atoi(args[2]), -1, pipeTerminalName);

			/* Ver linhas 109 e 110. */

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("\nPerdida conexao com i-banco.\n");

					printf("O i-banco-terminal vai encerrar.\n\n");

					if((error = unlink(pipeTerminalName)) == -1)  {

						perror("unlink: ");
					}

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					printf("--\ni-banco-terminal terminou.\n--\n");
					exit(EXIT_SUCCESS);
				}
				perror("write: ");
			}

			time(&start);

			/* O pipe de leitura e aberto*/

			if ((pipeTerm = open(pipeTerminalName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			/* Efectuada leitura do conteudo proveniente do pipe que recebe a resposta do i-banco. */

			if ((error = read(pipeTerm, output, BUFFER_SIZE)) == -1)  {
				perror("read_creditar: ");
			}

			time(&end);

			diff = difftime(end, start);

			if((error = close(pipeTerm)) == -1)  {
				perror("close: ");
			}

			printf("%s", output);
			printf("Tempo de execucao: %f\n\n", diff);

        }

        /* Ler Saldo */

        else if (strcmp(args[0], COMANDO_LER_SALDO) == 0)  {

            if (numargs < 2)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_LER_SALDO);

                continue;
            }

            input = produzir(OP_LERSALDO, atoi(args[1]), -1, -1, pipeTerminalName);

			/* Ver linhas 109 e 110. */

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("\nPerdida conexao com i-banco.\n");

					printf("O i-banco-terminal vai encerrar.\n\n");

					if((error = unlink(pipeTerminalName)) == -1)  {

						perror("unlink: ");
					}

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					printf("--\ni-banco-terminal terminou.\n--\n");
					exit(EXIT_SUCCESS);
				}
				perror("write: ");
			}

			time(&start);

			/* O pipe de leitura e aberto*/

			if ((pipeTerm = open(pipeTerminalName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			/* Efectuada leitura do conteudo proveniente do pipe que recebe a resposta do i-banco. */

			if ((error = read(pipeTerm, output, BUFFER_SIZE)) == -1)  {
				perror("read_lerSaldo: ");
			}

			time(&end);

			diff = difftime(end, start);

			if((error = close(pipeTerm)) == -1)  {
				perror("close: ");
			}

			printf("%s", output);
			printf("Tempo de execucao: %f\n\n", diff);
        }

        /* Transferir */

        else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0)  {

            if (numargs <= 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_TRANSFERIR);

                continue;
            }

            input = produzir(OP_TRANSFERIR, atoi(args[1]), atoi(args[2]), atoi(args[3]), pipeTerminalName);

			/* Ver linhas 109 e 110. */

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("\nPerdida conexao com i-banco.\n");

					printf("O i-banco-terminal vai encerrar.\n\n");

					if((error = unlink(pipeTerminalName)) == -1)  {

						perror("unlink: ");
					}

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					printf("--\ni-banco-terminal terminou.\n--\n");
					exit(EXIT_SUCCESS);
				}
				perror("write: ");
			}

			time(&start);

			/* O pipe de leitura e aberto*/

			if ((pipeTerm = open(pipeTerminalName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			/* Efectuada leitura do conteudo proveniente do pipe que recebe a resposta do i-banco. */

			if ((error = read(pipeTerm, output, BUFFER_SIZE)) == -1)  {
				perror("read_transferir: ");
			}

			time(&end);
			diff = difftime(end, start);

			if((error = close(pipeTerm)) == -1)  {
				perror("close: ");
			}
			printf("%s", output);
			printf("Tempo de execucao: %f\n\n", diff);
        }

		/* Simular */

		else if (strcmp(args[0], COMANDO_SIMULAR) == 0)  {

            if (numargs < 2)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_SIMULAR);

                continue;
            }

            input = produzir(OP_SIMULAR, -1, atoi(args[1]), -1, pipeTerminalName);

			/* Ver linhas 109 e 110. */

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("\nPerdida conexao com i-banco.\n");

					printf("O i-banco-terminal vai encerrar.\n\n");

					if((error = unlink(pipeTerminalName)) == -1)  {

						perror("unlink: ");
					}

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					printf("--\ni-banco-terminal terminou.\n--\n");
					exit(EXIT_SUCCESS);
				}
				perror("write: ");
			}
        }

		/* Sair Terminal */

		else if (strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0)  {


			if((error = unlink(pipeTerminalName)) == -1)  {
				perror("unlink: ");
			}

			if((error = close(pipeD)) == -1)  {
				perror("close: ");
			}

			printf("--\ni-banco-terminal terminou.\n--\n");
			exit(EXIT_SUCCESS);
		}

        else

            printf("Comando desconhecido. Tente de novo.\n\n");

    }
}
