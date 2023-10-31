# LRU

## Mechanism

Using `Doubly linked lists (DLL)`, one can implement LRU caching since traditional LRU caching is inefficient. DLL reduces time for `deletion` and `addition` of objects stored in cache ( `O(1)` in DLL ).

The maximum size of the DLL will be fixed. The most recently accessed files will be near the front end and the least recently accessed files will be near the rear end. Whenever a new file is added when the cache is full, the least accessed file/file at rear is deleted.
