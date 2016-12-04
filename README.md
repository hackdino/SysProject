# SysProject

 1.) Clone Repo
  
     git clone https://github.com/hackdino/SysProject.git

 2.) Compile project

     make all
    
 4.) start server
     
     ./hash_server [-i IP] [-p port] [-h]
     
 5.) start client(s)
 
     ./hash_client [-i IP] [-p port] [-h]

 6.) usage client(s)

     the client program provides an interactive modus:

     	hc >> crack "String" 		//calculates an collision hash key

     	hc >> quit			//quit program

	hc >> help			//print client usage

 6.) Clean generated files

     make clean
