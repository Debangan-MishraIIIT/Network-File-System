## Naming Server

1. The naming server is initialized in `initializeNamingServer()` where a new socket is created and binded to a fixed port.
2. The function `acceptHost()` runs on a thread allowing clients and storage servers to join. On joining, the host sends a message indicating if its a client or a storage server and its detials are stored in `clientDetials` and `storageServers` respectively. 
3. After a client joins the naming server, it continously listens for requests from the client on separate threads. Each request is answered by the details of the storage server requested.