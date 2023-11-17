NM_PORT := 6969
SS_PORTS := 6970

client:
	gcc client.c socket_functionality.c api_functionality.c errors.c -o client -pthread -Wno-format-truncation
	./client $(NM_PORT)
	rm client

client%:
	gcc client.c socket_functionality.c api_functionality.c  errors.c -o $@ -pthread -Wno-format-truncation
	./$@ $(NM_PORT)
	rm $@

storageServer:
	gcc storageServer.c socket_functionality.c api_functionality.c errors.c -o storageServer -pthread -Wno-format-truncation
	./storageServer $(SS_PORTS) $$((1  + $(SS_PORTS)))
	rm storageServer

storageServer%: 
	gcc storageServer.c socket_functionality.c api_functionality.c errors.c -o $@ -pthread -Wno-format-truncation
	./$@ $$(($(subst storageServer,,$@) * 2 + $(SS_PORTS))) $$(($(subst storageServer,,$@) * 2 + 1  + $(SS_PORTS)))
	rm $@

namingServer:
	gcc namingServer.c socket_functionality.c search.c api_functionality.c cache.c errors.c -o namingServer -pthread -Wno-format-truncation
	./namingServer $(NM_PORT)
	rm namingServer

clean:
	rm namingServer client storageServer

temp:
	gcc temp.c api_functionality.c errors.c 