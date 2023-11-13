rules:
	gcc client.c socket_functionality.c -o client
	gcc server.c socket_functionality.c -o server

client:
	gcc client.c socket_functionality.c -o client -pthread
	./client
	rm client

client%:
	gcc client.c socket_functionality.c -o $@ -pthread
	./$@
	rm $@

storageServer:
	gcc storageServer.c socket_functionality.c -o storageServer -pthread
	./storageServer
	rm storageServer

storageServer%: 
	gcc storageServer.c socket_functionality.c -o $@ -pthread
	./$@
	rm $@

namingServer:
	gcc namingServer.c socket_functionality.c search.c -o namingServer -pthread
	./namingServer
	rm namingServer

clean:
	rm namingServer client storageServer