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

all: i-banco i-banco-terminal

i-banco: i-banco.o commandlinereader.o contas.o
		gcc -g -Wall -pedantic -pthread -o i-banco i-banco.o commandlinereader.o contas.o

i-banco-terminal:i-banco-terminal.o commandlinereader.o contas.o
		gcc -g -Wall -pedantic -pthread -o i-banco-terminal i-banco-terminal.o commandlinereader.o contas.o

i-banco.o: i-banco.c contas.h commandlinereader.h
		gcc -g -Wall -pedantic -c i-banco.c

i-banco-terminal.o: i-banco-terminal.c contas.h commandlinereader.h
		gcc -g -Wall -pedantic -c i-banco-terminal.c

contas.o: contas.c contas.h
		gcc -g -Wall -pedantic -c contas.c

commandlinereader.o: commandlinereader.c commandlinereader.h
		gcc -g -Wall -pedantic -c commandlinereader.c

clean:
	rm -f *.o i-banco i-banco-terminal i-banco-sim-*.txt log.txt /tmp/i-banco* /tmp/log.txt
