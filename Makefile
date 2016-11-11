#************************************************#
#            Projecto SO 2016/2017               #
#                                                #
#         Joao Miguel Campos, n 75785            #
#          Carolina Monteiro, n 77990            #
#                                                #
#                                                #
#                   Grupo 32                     #
#                                                #
#************************************************#

all: i-banco

i-banco: i-banco.o contas.o commandlinereader.o 
		gcc -g -Wall -pedantic -pthread -o i-banco i-banco.o commandlinereader.o contas.o

i-banco.o: i-banco.c contas.h commandlinereader.h
		gcc -g -Wall -pedantic -c i-banco.c

contas.o: contas.c contas.h
		gcc -g -Wall -pedantic -c contas.c

commandlinereader.o: commandlinereader.c commandlinereader.h
		gcc -g -Wall -pedantic -c commandlinereader.c

clean:
	rm -f *.o i-banco
