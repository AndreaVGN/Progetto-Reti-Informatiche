#Makefile 

all: server client other

server: server.o manage_client_command.o utility.o 
		gcc -Wall server.o manage_client_command.o utility.o -o server 

client: client.o 
		gcc -Wall client.o -o client 

other: client.o 
	   gcc -Wall client.o -o other

server.o: server.c 
		gcc -Wall -c server.c -o server.o 

client.o: client.c 
		gcc -Wall -c client.c -o client.o 

utility.o: utility.c utility.h 
		gcc -Wall -c utility.c -o utility.o 

manage_client_command.o: manage_client_command.c 
						gcc -Wall -c manage_client_command.c -o manage_client_command.o

clean:
		rm *.o server client other

run: all

launch:
		gnome-terminal --tab --title="Server Terminal" --command="bash -c './server 4242; exec bash'" \
					   --tab --title="Client Terminal" --command="bash -c './client 4242; exec bash'" \
					   --tab --title="Other Client Terminal" --command="bash -c './client 4242; exec bash'"
