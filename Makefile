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



i-banco: contas.o commandlinereader.o i-banco.o
		gcc -Wall -pedantic -pthread -o i-banco contas.o commandlinereader.o i-banco.o

contas.o: contas.c contas.h
		gcc -g -Wall -pedantic -c contas.c

commandlinereader.o: commandlinereader.c commandlinereader.h
		gcc -g -Wall -pedantic -c commandlinereader.c

i-banco.o: i-banco.c contas.h commandlinereader.h
		gcc -g -Wall -pedantic -c i-banco.c

