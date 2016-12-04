# SysProject

 1.) Clone Repo
  
     git clone https://github.com/hackdino/SysProject.git

 2.) Compile project

     make all
    
 4.) start server
     
     ./hash_server [-i IP] [-p port] [-l logfile] [-h]
     
 5.) start client(s)
 
     ./hash_client [-i IP] [-p port] [-h]

 6.) usage client(s)

     the client program provides an interactive modus:

     	* hc >> crack "String" 		//calculates an collision hash key

     	* hc >> quit			//quit program

	* hc >> help			//print client usage

 7.) usage server

	* with ^C server will shutdown properly
	* logging information will be write into logfile.txt(default)


 6.) Clean generated files (optional)

     make clean
