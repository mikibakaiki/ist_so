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

	/*if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)  {

		perror("signal SIGPIPE: ");
	}*/

    if ((numB = snprintf(pipeTerminalName, sizeof(pipeTerminalName), "/tmp/pipe-terminal-%d", getpid())) >= sizeof(pipeTerminalName))  {
        printf("Erro snprintf\n");
    }

	/*printf("pipe name: %s\n", pipeTerminalName);*/


	if ((error = unlink(pipeTerminalName)) == -1 && errno != ENOENT)  {
        perror("unlink /tmp/pipe-terminal: ");
    }

    if ((error = mkfifo(pipeTerminalName, 0777)) == -1)  {

    	perror("mkfifo: ");
    }



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

				//printf("estrutura SAIR AGORA:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n\n", input.operacao, input.idConta, input.valor, input.nome);

				if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

					if (errno == EPIPE)  {

						printf("Perdida conexao com i-banco.\n");

						printf("A tentar conexao...\n");

						if ((error = close(pipeD)) == -1)  {
		                    perror("close: ");
		                    exit(EXIT_FAILURE);
		                }

						if ((pipeD = open(argv[1], O_WRONLY)) == -1)  {
					    	perror("open: ");
					    }

						continue;
					}
	            	perror("write: ");
	        	}
			}

			input = produzir(OP_SAIR, -1, -1, -1, pipeTerminalName);

			//printf("estrutura SAIR:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n\n", input.operacao, input.idConta, input.valor, input.nome);

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("Perdida conexao com i-banco.\n");

					printf("A tentar conexao...\n");

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					if ((pipeD = open(argv[1], O_WRONLY)) == -1)  {
						perror("open: ");
					}

					continue;
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

			//printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n\n", input.operacao, input.idConta, input.valor, input.nome);
			//printf("Vou receber comando do pipe\n");

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("Perdida conexao com i-banco.\n");

					printf("A tentar conexao...\n");

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					if ((pipeD = open(argv[1], O_WRONLY)) == -1)  {
						perror("open: ");
					}

					continue;
				}
				perror("write: ");
			}

			time(&start);

		    if ((pipeTerm = open(pipeTerminalName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(pipeTerm, output, BUFFER_SIZE);

			time(&end);

			diff = difftime(end, start);

			if((error = close(pipeTerm)) == -1)  {
				perror("close: ");
			}

			printf("%s", output);
			printf("Tempo de execucao: %f\n\n", diff);


            //printf("Processei o comando e pus no buffer\n");
        }

        /* Creditar */

        else if (strcmp(args[0], COMANDO_CREDITAR) == 0)  {

            if (numargs < 3)  {

                printf("%s: Sintaxe inválida, tente de novo.\n\n", COMANDO_CREDITAR);

                continue;
            }

            input = produzir(OP_CREDITAR, atoi(args[1]), atoi(args[2]), -1, pipeTerminalName);

			//printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("Perdida conexao com i-banco.\n");

					printf("A tentar conexao...\n");

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					if ((pipeD = open(argv[1], O_WRONLY)) == -1)  {
						perror("open: ");
					}

					continue;
				}
				perror("write: ");
			}

			time(&start);

			if ((pipeTerm = open(pipeTerminalName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(pipeTerm, output, BUFFER_SIZE);

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

			//printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("Perdida conexao com i-banco.\n");

					printf("A tentar conexao...\n");

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					if ((pipeD = open(argv[1], O_WRONLY)) == -1)  {
						perror("open: ");
					}

					continue;
				}
				perror("write: ");
			}

			time(&start);

			if ((pipeTerm = open(pipeTerminalName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(pipeTerm, output, BUFFER_SIZE);

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

			//printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("Perdida conexao com i-banco.\n");

					printf("A tentar conexao...\n");

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					if ((pipeD = open(argv[1], O_WRONLY)) == -1)  {
						perror("open: ");
					}

					continue;
				}
				perror("write: ");
			}

			time(&start);

			if ((pipeTerm = open(pipeTerminalName ,O_RDONLY)) == -1)  {
		        perror("open: ");
				exit(-1);
		    }

			read(pipeTerm, output, BUFFER_SIZE);
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

			printf("estrutura:\noperacao: %d\nConta: %d\nValor: %d\nPATH: %s\n\n", input.operacao, input.idConta, input.valor, input.nome);

			//printf("Vou receber comando do pipe\n");

			if ((error = write(pipeD, &input, sizeof(comando_t))) == -1)  {

				if (errno == EPIPE)  {

					printf("Perdida conexao com i-banco.\n");

					printf("A tentar conexao...\n");

					if ((error = close(pipeD)) == -1)  {
						perror("close: ");
						exit(EXIT_FAILURE);
					}

					if ((pipeD = open(argv[1], O_WRONLY)) == -1)  {
						perror("open: ");
					}

					continue;
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
