########################################
#~ définitions
########################################

# nom de l'executable
#BIN=runPeriod

BIN=bin/client  bin/server
#BIN=bin/serveur

# liste des fichiers sources 
SRCS0=client.c
SRCS1=server.c

default: $(BIN)

########################################
#~ regles pour l'executable
########################################

obj/%.o: %.c
	gcc -Wall -Iinclude -c $< -o $@ -lpthread

bin/client: $(SRCS2:%.c=obj/%.o) $(SRCS0:%.c=obj/%.o)
	gcc -o $@ $+ -lpthread

bin/server: $(SRCS2:%.c=obj/%.o) $(SRCS1:%.c=obj/%.o)
	gcc -o $@ $+ -lpthread



clean:
	rm -f $(BIN) obj/*.o *~
