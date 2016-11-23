# SysProject

 1.) Compile Server
 
     gcc hash_server.c crc32.c -o hash_server -Wall -pedantic-errors -lpthread
     
 2.) Compile Client
 
     gcc -std=c99 -o hash_client hash_client.c -Wall -pedantic
    
 3.) Start Server
     
     ./hash_server
     
 4.) Start Client(s)
 
     ./hash_client
