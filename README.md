Initial Commit
- Gaurav -- Working in Connection
- Deby -- Working in File Transfer
- Harsha -- LRU, file serach

API CALLS:
1. MKDIR and MKFILE: enables the creation of directory with a given set of permissions. Implemented by the makeDirectory call which checks whether a given path exists or not, and using the reversePermissions call to get the permissions set by the user. Default permissions for a directory are ```drwxr-xr-x``` and for a file are ```-rw-r--r--```. After the succesful completion of the call, the newly created path is added to the records. 
2. RMDIR and RMFILE: deletes a file or a directory and the paths are removed from the records. For deletion of a directory, rmdir system call is used and for that of a file, rmfile command is used. Note that the expected behaviour of a directory that is not empty cannot be deleted is followed. 
3. COPY is used to copy a file or a directory. We are recursively traversing the accessible path n-ary tree (nextSibling for all except the source itself and the firstChild for every intermediate node). If we encounter a directory, we use the already defined MKDIR call otherwise we use MKFILE. We then send over the contents of the file via the network and send it to the destination storage server. The temporarily created file at the naming server location is subsequently deleted. 

LOCKS:
There is a field within each recird struct which contains mutex lock record_lock. Before using any record, we lock it and once we receive its acknowledgement, we unlock it. This ensures that a given record can only be accesed by one client/ server at a time. 