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

#define atrasar() sleep(ATRASO)


sem_t read;
sem_init(&read, 1, 0);

sem_t write;
sem_init(&write, 1, CMD_BUFFER_DIM);


/***   CONSTANTES   ***/ 


int contasSaldos[NUM_CONTAS];

/* Flag para verificar signals.*/

int sig_find = 0;


/***   FUNCOES   ***/


int contaExiste(int idConta)  {

  return (idConta > 0 && idConta <= NUM_CONTAS);
}


void inicializarContas()  {

  int i;

  for (i=0; i<NUM_CONTAS; i++)

	contasSaldos[i] = 0;
}


int debitar(int idConta, int valor)  {

  atrasar();

  if (!contaExiste(idConta))

	return -1;

  if (contasSaldos[idConta - 1] < valor)

	return -1;

  atrasar();

  contasSaldos[idConta - 1] -= valor;

  return 0;
}


int creditar(int idConta, int valor)  {

  	atrasar();

  	if (!contaExiste(idConta))

		return -1;

  	contasSaldos[idConta - 1] += valor;

  	return 0;
}


int lerSaldo(int idConta)  {

	atrasar();

	if (!contaExiste(idConta))

		return -1;

	return contasSaldos[idConta - 1];
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

			saldo = lerSaldo(idConta);

			if (ano != 0)  {
			
		  		creditar(idConta, saldo*TAXAJURO);

		  	if (saldo < CUSTOMANUTENCAO)

				debitar(idConta, saldo);

		  	else
				debitar(idConta, CUSTOMANUTENCAO);

			saldo = lerSaldo(idConta);
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

	signal(sig, handler);
}


/*************************/
/*       PARTE 2         */
/*************************/


comando_t produzir(int op, int id, int val)  {

  comando_t *i = (comando_t *) malloc(sizeof(struct));

  i->operacao = op;
  i->idConta = id;
  i->valor = val;
  /*Damos free no final de usarmos cada ponteiro ou depois de usarmos todos ?! */

  return *i;
}


void put(comando_t *item)  {

	sem_wait(&write);
	/*fechar(MutexP);*/

	cmdbuffer[buff_write_idx] = *item;

	buff_write_idx = (buff_write_idx + 1) % CMD_BUFFER_DIM;

	/*abrir*/

	sem_post(&read);

}


void *thr_consumer (void *item) {

  	while(1)  {

		item = (comando_t *) malloc(sizeof(struct));

		item = get();

		consume(item);
  	}
}


comando_t get()  {

  	sem_wait(&read);

  	/*mutex*/

  	comando_t *item = (comando_t *) malloc(sizeof(struct));

  	*item = cmdbuffer[buff_read_idx];

  	buff_read_idx = (buff_read_idx + 1) % CMD_BUFFER_DIM;

  	/*mutex*/

	sem_post(&write);

	return *item;

}


void consume(comando_t *item)  {

	int saldo;

	if (item->operacao == OP_LERSALDO)  {

		saldo = lerSaldo(item->idConta);

		if (saldo < 0)

        	printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, item->idConta);

    	else

        	printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, item->idConta, saldo);
	}

	if (item->operacao == OP_CREDITAR)  {

		if (creditar (item->idConta, item->valor) < 0)

            printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, item->idConta, item->valor);

    	else

            printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, item->idConta, item->valor);
	}

	if (item->operacao == OP_DEBITAR)  {

		if (debitar (item->idConta, item->valor) < 0)

            printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, item->idConta, item->valor);

        else

            printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, item->idConta, item->valor);
	}
}
