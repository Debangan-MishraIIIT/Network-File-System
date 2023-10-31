# LRU

## Mechanism

Using `Doubly linked lists (DLL)`, one can implement LRU caching since traditional LRU caching is inefficient. DLL reduces time for `deletion` and `addition` of objects stored in cache ( `O(n)` in traditional method and `O(1)` in DLL ).

The maximum size of the DLL will be fixed. The most recently accessed files will be near the front end and the least recently accessed files will be near the rear end.
