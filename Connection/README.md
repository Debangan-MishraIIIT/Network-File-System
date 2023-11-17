## Naming Server

1. The naming server is initialized in `initializeNamingServer()` where a new socket is created and binded to a fixed port.
2. The function `acceptHost()` runs on a thread allowing clients and storage servers to join. On joining, the host sends a message indicating if its a client or a storage server and their detials are stored in `clientDetials` and `storageServers` respectively. 
3. After a storage server joins the naming server, then another socket is initialsed for listening to accessible paths dynamically on a separate thread running the `addToRecord()` function.
3. After a client joins the naming server, it continously listens for requests from the client on separate threads `acceptClientRequests()`. The detials of the storage server responsible for the given path is determined by `getRecord()` function.
4. // add lru and tries, how its used in getRecord()
5. // add all file stuff and acknowledgement

## Storage Server
1. The storage server is initialized by binding to two ports one for communication with all clients - `initialzeClientsConnection()` and another for communication with the naming server - `initializeNMConnection()`.
2. A thread running `takeInputsDynamically()` is created which continously listens on the command line for the list of accessible paths which is then sent to the naming server.
3. A thread for accepting clients is created which first accepts connection requests from clients and then responds to them using the `serveClient_Request()` thread.
4. // add how ss serves 

## Client
1. The client is initialized using the `joinNamingServerAsClient()` and is accepted in the `acceptHost()` function in the naming server.
2. A continuous thread running the `isNMConnected()` function runs to check for the socket status of the naming server. Whenever the naming server goes down, the client is notified.
3. In a continous loop, first an input is taken from the user and sent to the `sendRequest()` function which processes the input and sends it to the naming server.
4. // add how client requests are processed
