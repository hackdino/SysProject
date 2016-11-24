# SysProject

 1.) Clone Repo
  
     git clone https://github.com/hackdino/SysProject.git

 2.) Compile Server
 
     gcc hash_server.c crc32.c -o hash_server -Wall -pedantic-errors -lpthread
     
 3.) Compile Client
 
     gcc -std=c99 -o hash_client hash_client.c -Wall -pedantic -lpthread
    
 4.) Start Server
     
     ./hash_server
     
 5.) Start Client(s)
 
     ./hash_client
