# Online-Judge-for-C-Using-Socket-Programming

A console app written in C++ using socket progamming where the client can send C/C++ programs to be judged by the server. 
The server checks the client's program and matches its output with some hidden testcases and sends some feedback messages accordingly to the client.


### COMMANDS FOR EXECUTION:

	Server program:

		COMPILE: 
			g++ server.cpp -o server

		RUN:
			./server <port_number>
			e.g ./server 3456

	Client program:

		COMPILE: 
			g++ client.cpp -o client

		RUN:
			./client localhost <port_number>
			e.g ./client localhost 3456
	
			

			
### AVAILABLE COMMANDS AND THEIR FUNCTIONALITY:

	1. CODEJUD:
	
		-> File extensions allowed - .c, .cpp

		Example format for c file:
		    CODEJUD div.c c
		
		Example format for c++ file:
		    CODEJUD add.cpp c++
		    
		-> The server will send feedback about the client's program in 3 phases:
			a. COMPILATION PHASE ["COMPILE_SUCCESS" / "COMPILE_ERROR"]
			b. EXECUTION PHASE ["RUN_SUCCESS" / "RUN_ERROR" / "TIME_LIMIT_EXCEEDED"]
			c. MATCHING PHASE ["ACCEPTED" / "WRONG ANSWER"]

		-> For each program, the judge at server contains one input file and one testcase file following the naming convention input_<testprog_name_without_extension>.txt and testcase_<testprog_name_without_extension>.txt respectively. If the user program is compiled successfully, an output file is generated named output_<testprog_name_without_extension>.txt which contains output of the user program for each testcase input.
		
		-> If client gives the command "CODEJUD add.c c", "add.c" should be present in client's directory and "input_add.txt", "testcase_add.txt" should be present in server's directory.
    
    -> Some example codes are included in client's directory and their corressponding input and testcase files are included in server's directory.
		
		-> Each line of the input file is considered as a different testcase input.

		-> Suppose, user submits a program file named 'x.c'.
		
		-> An input file 'input_x.txt' and a testcase file (containing correct outputs) 'testcase_x.txt' must be present in server's directory.
		
		-> Input redirection is used so that the input comes from the input_x.txt file during executing the program with each testcase input. 
			
		-> The whole input file cannot be passed during execution. So, temporary files each containing one line of testcase input are created one by one for each testcase execution while scanning the input file line by line and the outputs are obtained for one testcase at a time. After each output is obtained, its temporary input file is deleted. 
			
		-> User program's output for each testcase input is combined to generate the final output file named 'output_x.txt' in server's directory. Its whole content is matched with the testcase file 'testcase_x.txt' to generate the final decision of the online judge.

		-> If the program exceeds time limit or gives runtime error on a testcase, then 'TIME_LIMIT_EXCEEDED' or 'RUN_ERROR' is message is sent to client and the operation is stopped.
		
		-> If contents of output_x.txt and testcase_x.txt match completely, 'ACCEPTED' message is sent to client. Otherwise 'WRONG_ANSWER' message is sent.

		-> If client gives the command "CODEJUD add.c c" and a file named "add.c" already exists in server's directory, the operation will be stopped to prevent overwriting the file present at server.
		
		

	2. RETR: 
		-> This command causes the remote host to initiate a data connection and to send the requested file over the data connection.
		-> example: RETR myCode.c
	
	3. STOR: 
		-> This command causes to store a file into the current directory of the remote host.
		-> example: STOR myCode.c
	
	4. LIST: 
		-> This command sends a request to display the list of all files present in the server's directory.
		-> example: LIST
		
	5. DELE: 
		-> This command deletes a file in the current directory of server.
		-> example: DELE myCode.c
		
	6. QUIT: 
		-> This command terminates a USER and if file transfer is not in progress, the server closes the control connection.
		
		
    

### EXITING THE PROGRAM:
    -> Give the QUIT command or type ctrc+c to exit from client program.
    -> Type ctrc+c to exit from server program.




### OTHER INFO:

    -> Extra whitespaces in the begging and ending of command are removed. Whitespaces between command name and filename are also removed.
		Example (below commands are all valid):
		    CODEJUD       myCode.cpp    c++
		        LIST    
		    RETR        myCode.c
		    DELE            my text.txt    
    
    -> The command names can be written in both cases (but the filename is case sensitive).
		Example (below commands are all valid):
		    Retr my text.txt   
		    retr my text.txt
		    codejud myCode.c c
 
    -> If a filename already exists in the server's current directory, and client does STOR with the same filename then the file transfer is stopped to prevent overwriting the file in server's directory. 
    
    -> In the same way, if a filename already exists in the client's current directory, and client does RETR with the same filename then the file transfer is stopped to prevent overwriting the file in client's directory.
    
    -> The program was tested and worked correctly with file types: .txt, .c, .cpp, .pdf, .mp4, .mkv

    -> Maximum filesize allowed to be transferred - 1GB 

    -> Buffer size for sending and receiving data - 2048

    -> Port number range - from 1024 to 65535
    
    
    
    
    
    
    
