CC = gcc
CFLAGS = -Wall -Wextra -g
OBJFILES = fileparser.o iluvatarSon.o arda.o shellHelper.o communications.o connection.o hashmap.o messages.o ipcHelper.o socketHelper.o ardaFunctions.o iluvatarFunctions.o iluvatarThreads.o stdinout.o
TARGETS = iluvatarSon arda

all: clean iluvatarSon arda

iluvatarSon: iluvatarSon.o fileparser.o communications.o shellHelper.o connection.o hashmap.o messages.o ipcHelper.o socketHelper.o iluvatarThreads.o iluvatarFunctions.o stdinout.o
	$(CC) $(CFLAGS) -pthread -o iluvatarSon fileparser.o iluvatarSon.o shellHelper.o ipcHelper.o communications.o connection.o hashmap.o messages.o socketHelper.o iluvatarThreads.o iluvatarFunctions.o stdinout.o -lrt

arda: arda.o fileparser.o communications.o connection.o hashmap.o messages.o socketHelper.o ardaFunctions.o stdinout.o
	$(CC) $(CFLAGS) -pthread -o arda arda.o fileparser.o communications.o connection.o hashmap.o messages.o socketHelper.o ardaFunctions.o stdinout.o -lrt

communications.o: ipcHelper.o
	$(CC) $(CFLAGS) -c modules/communications.c -lrt

ipcHelper.o: shellHelper.o
	$(CC) $(CFLAGS) -c modules/ipcHelper.c -lrt

socketHelper.o:
	$(CC) $(CFLAGS) -c modules/socketHelper.c

shellHelper.o:
	$(CC) $(CFLAGS) -c modules/shellHelper.c

fileparser.o:
	$(CC) $(CFLAGS) -c modules/fileparser.c

connection.o:
	$(CC) -c modules/connection.c

hashmap.o:
	$(CC) -c modules/hashmap.c

messages.o:
	$(CC) $(CFLAGS) -c modules/messages.c

ardaFunctions.o:
	$(CC) $(CFLAGS) -c modules/ardaFunctions.c

iluvatarFunctions.o: iluvatarThreads.o
	$(CC) $(CFLAGS) -c modules/iluvatarFunctions.c

iluvatarThreads.o:
	$(CC) $(CFLAGS) -c modules/iluvatarThreads.c

stdinout.o:
	$(CC) $(CFLAGS) -c modules/stdinout.c


clean:
	rm -f $(OBJFILES) $(TARGETS) *~
