rules:
	gcc client.c socket_functionality.c -o client
	gcc server.c socket_functionality.c -o server

client:
	gcc client.c socket_functionality.c -o client
	./client
	rm client

client%:
	gcc client.c socket_functionality.c -o $@
	./$@
	rm $@

storageServer:
	gcc storageServer.c socket_functionality.c -o storageServer
	./storageServer
	rm storageServer

storageServer%: 
	gcc storageServer.c socket_functionality.c -o $@
	./$@
	rm $@

namingServer:
	gcc namingServer.c socket_functionality.c -o namingServer
	./namingServer
	rm namingServer

clean:
	rm namingServer client storageServer