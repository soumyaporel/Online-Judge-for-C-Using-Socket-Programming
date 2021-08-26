/*
NAME- Soumya Porel
ROLL- 20CS60R36
Assignement 4 (server program)
*/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 

#define BUFFER_SIZE 2048 // maximum size of buffers used in reading and writing

using namespace std;

// for storing information of command sent from client
struct CommandInfo {
    bool is_valid;
    char *name;
    bool has_file_arg;
    char *filename_arg;
	bool has_ext;
	char *ext;
    char *filename_without_extension;
};

// checks the validity of command sent from client
struct CommandInfo get_command_info(string command) {

    struct CommandInfo ci;

	// remove beginning spaces
	int l = 0;
	while (l < command.size() && (command[l] == ' ' || command[l] == '\t' || command[l] == '\n')) {
		l++;
	}
	// l is now at non-whitespace position

	// remove ending spaces
	int r = command.size() - 1;
	while (r >= 0 && (command[r] == ' ' || command[r] == '\t' || command[r] == '\n')) {
		r--;
	}
	// r is now at non-whitespace position

	if (r - l + 1 <= 0) { // string doesn't have non-whitespace char
        ci.is_valid = false;
		return ci;
	}

	// copy the string from index l to index r
	char stripped_command[r - l + 2]; // one extra space for null char
	for (int i = 0; i < r - l + 1; i++) {
		stripped_command[i] = command[l + i];
	}
	stripped_command[r - l + 1] = '\0';

    // following ftp commands are available
    // 1.RETR <filename> 2.STOR <filename> 3.LIST 4.ABOR 
	// 5.QUIT 6.DELE <filename>, 7. CODEJUD <filename> <ext(c/c++)>
    
    if (strlen(stripped_command) < 4) {
        ci.is_valid = false;
        return ci;
    }

	// 1st 4 chars of stripped_command must be equal to these commands
    if (strncasecmp("RETR", stripped_command, 4) == 0 || strncasecmp("STOR", stripped_command, 4) == 0 || strncasecmp("DELE", stripped_command, 4) == 0) {

        // 5th char should be whitespace 
        if (stripped_command[4] != ' ' && stripped_command[4] != '\t') {
            ci.is_valid = false;
            return ci;
        }
        
        // get file name
        char file_name[strlen(stripped_command)- 5 + 1]; // reduce by 5 as 1st 5 chars are command name followed by space, add 1 for null char at end
        int i = 5;
		// skip the whitespaces before file name
		while (i < strlen(stripped_command) && (stripped_command[i] == ' ' || stripped_command[i] == '\t' || stripped_command[i] == '\n')) {
			i++;
		}
		// i is now at beginning of file name 
		int file_name_index = 0;
		for (; i < strlen(stripped_command); i++) {
            file_name[file_name_index++] = stripped_command[i];
        }
        file_name[file_name_index] = '\0';
        
        // error if file name doesn't have non-whitespace char
        i = 0;
        for (i = 0; i < strlen(file_name); i++) {
            if (!(file_name[i] == ' ' || file_name[i] == '\t' || file_name[i] == '\n')) {
                break;
            }
        }
        if (i == strlen(file_name)) {
            ci.is_valid = false;
            return ci;
        }

        // return command info
        ci.is_valid = true;
        ci.has_file_arg = true;
        ci.name = (char *)malloc(5 * sizeof(char));
        for (int i = 0; i < 4; i++) {
            ci.name[i] = toupper(stripped_command[i]);
        }
        ci.name[4] = '\0';
        ci.filename_arg = (char *)malloc(strlen(file_name) * sizeof(char));
        strcpy(ci.filename_arg, file_name);
		ci.has_ext = false;
        return ci;
    }

	// 1st 4 chars of stripped_command must be equal to these commands
    else if (strncasecmp("LIST", stripped_command, 4) == 0 || strncasecmp("QUIT", stripped_command, 4) == 0) {

        // error if remaining string contains non-whitespace char
        for (int i = 4; i < strlen(stripped_command); i++) {
            if (stripped_command[i] == ' ' || stripped_command[i] == '\t' || stripped_command[i] == '\n') {
                continue;
            }
            else {
                ci.is_valid = false;
                return ci;
            }
        }

        // return command info
        ci.is_valid = true;
        ci.has_file_arg = false;
        ci.name = (char *)malloc(5 * sizeof(char));
        for (int i = 0; i < 4; i++) {
            ci.name[i] = toupper(stripped_command[i]);
        }
        ci.name[4] = '\0';
		ci.has_ext = false;
        return ci;
    }

	// 1st 7 chars of stripped_command must be equal to CODEJUD
	else if (strlen(stripped_command) >= 7 && strncasecmp("CODEJUD", stripped_command, 7) == 0) {

        ci.has_file_arg = true;
		ci.has_ext = true;

		// 8th char should be whitespace 
        if (stripped_command[7] != ' ' && stripped_command[7] != '\t') {
            ci.is_valid = false;
            return ci;
        }

		// get the extension. scan from right of stripped command and stop on encountering whitespace.
		string ext;
		for (int i = strlen(stripped_command) - 1; i > 7; i--) {
			if (stripped_command[i] == ' ' || stripped_command[i] == '\t') {
				break;
			}
			ext += stripped_command[i];
		}
		// the extension must be "c++" or "c"
		if (ext.size() == 1 && strncasecmp("c", ext.c_str(), 1) == 0) {
			ci.ext = (char *)malloc(ext.size() * sizeof(char));
			strcpy(ci.ext, "c");
		} 
        // compare with "++c" as the string is stored in reversed order
		else if (ext.size() == 3 && strncasecmp("++c", ext.c_str(), 3) == 0) {
			ci.ext = (char *)malloc(ext.size() * sizeof(char));
			strcpy(ci.ext, "c++");
		} 
		else {
			ci.is_valid = false;
            return ci;
		}
        // get file name 
        char file_name[strlen(stripped_command)]; 
        int l = 8;
		// skip the whitespaces before file name
		while (l < strlen(stripped_command) && (stripped_command[l] == ' ' || stripped_command[l] == '\t' || stripped_command[l] == '\n')) {
			l++;
		}
		// l is now at beginning of file name, we need to scan till before whitespacs before the file extension argument
        int r = strlen(stripped_command) - strlen(ci.ext) - 1; // r is now at the last whitespace before the file extension argument
        while (r >= 7 && (stripped_command[r] == ' ' || stripped_command[r] == '\t' || stripped_command[r] == '\n')) {
			r--;
		}
        // scan from l to r
		int file_name_index = 0;
		for (; l <= r; l++) {
            file_name[file_name_index++] = stripped_command[l];
        }
        file_name[file_name_index] = '\0';
        // error if file name doesn't have non-whitespace char
        int i = 0;
        for (i = 0; i < strlen(file_name); i++) {
            if (!(file_name[i] == ' ' || file_name[i] == '\t' || file_name[i] == '\n')) {
                break;
            }
        }
        if (i == strlen(file_name)) {
            ci.is_valid = false;
            return ci;
        }

        ci.filename_arg = (char *)malloc(strlen(file_name) * sizeof(char));
        strcpy(ci.filename_arg, file_name);

        // error if filename doesn't end with "." + extension
        // get the last 1 + len(extension) chars from filename
        if (strlen(ci.filename_arg) <  1 + strlen(ci.ext)) {
            ci.is_valid = false;
            return ci;
        }
        // match the '.'
        if (ci.filename_arg[strlen(ci.filename_arg) - strlen(ci.ext) - 1] != '.') {
            ci.is_valid = false;
            return ci;
        }
        // match last len(extension) chars of ci.filename_arg
        char *filename_ending = (char *)malloc((strlen(ci.ext) + 1) * sizeof(char));
        int filename_ending_index = 0;
        for (int i = strlen(ci.filename_arg) - strlen(ci.ext); i < strlen(ci.filename_arg); i++, filename_ending_index++) {
            filename_ending[filename_ending_index] = ci.filename_arg[i];
        }
        filename_ending[filename_ending_index] = '\0';
        if (strcmp("c", ci.ext) == 0 && strcmp("c", filename_ending) != 0) {
            cout << "The file type does not match with the extension provided as argument\n";
            ci.is_valid = false;
            return ci;
        }
        if (strcmp("c++", ci.ext) == 0 && strcmp("cpp", filename_ending) != 0) {
            cout << "The file type does not match with the extension provided as argument\n";
            ci.is_valid = false;
            return ci;
        }
        
        // return command info
        ci.is_valid = true;
        ci.name = (char *)malloc(8 * sizeof(char));
		for (int i = 0; i < 7; i++) {
            ci.name[i] = toupper(stripped_command[i]);
        }
		ci.name[7] = '\0';

        ci.filename_without_extension = (char *)malloc((strlen(ci.filename_arg) - strlen(ci.ext)) * sizeof(char));
        int index;
        for (index = 0; index < strlen(ci.filename_arg) - strlen(ci.ext) - 1; index++) {
            ci.filename_without_extension[index] = ci.filename_arg[index];
        }
        ci.filename_without_extension[index] = '\0';
        
        return ci;
	}

    else {
        ci.is_valid = false;
        return ci;
    }
}

// executes a system command and returns the result as a string
string get_system_command_result(const char* cmd) {
    char buffer[BUFFER_SIZE];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        cout << "popen() failed!\n";
        exit(1);
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// the main function	
int main(int argc, char *argv[]) { 

    if (argc < 2) {
		cout << "ERROR: please provide the port number as command line argument!\n";
		exit(1);
	}

    int portno = atoi(argv[1]); // get port number from command line
    int max_clients = 30;
    int client_socket[max_clients];
	int opt = 1;
		
	// set of socket descriptors 
	fd_set readfds; 
	
	// initialise all client_socket[] to 0 so not checked 
	for (int i = 0; i < max_clients; i++) { 
		client_socket[i] = 0; 
	} 

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET for IPv4, SOCK_STREAM is for creating TCP connection, 0 is specified TCP/UDP
    if (sockfd < 0) {
        cout << "Error in creating socket\n";
        exit(1);
    }

	if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {   
        cout << "Error in setsockopt\n";   
        exit(1);   
    }   

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { // bind IP address and port number to create a socket
        cout<<"Binding failed!\n";
        exit(1);
    }   
	
	if (listen(sockfd, 5)) {
		cout<<"Server running... waiting for connections."<<endl;
    }
    // first argument specifies socket descriptor where information from client will be stored
    // Second argument defines the maximum length to which the queue of pending connections for sockfd may grow

    socklen_t serv_addr_size = sizeof(serv_addr);

    int activity;
    int newsockfd;
    int sd, max_sd; 

	char send_buffer[BUFFER_SIZE]; // server buffer to send reply to the client
    char recv_buffer[BUFFER_SIZE]; // server buffer to get command from the client
	bzero(send_buffer, BUFFER_SIZE);
    bzero(recv_buffer, BUFFER_SIZE);
		
	while (1) { 
		//clear the socket set 
		FD_ZERO(&readfds); 
	
		// add sockfd to set 
		FD_SET(sockfd, &readfds); 
		max_sd = sockfd; 
			
		// add child sockets to set 
		for (int i = 0; i < max_clients; i++) { 
			// socket descriptor 
			sd = client_socket[i]; 
				
			// if valid socket descriptor then add to read list 
			if(sd > 0) {
                FD_SET(sd, &readfds);
            }
				
			// highest file descriptor number, need it for the select function 
			if (sd > max_sd) {
				max_sd = sd; 
            }
		} 
	
		// wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely 
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL); 
		if (activity < 0) { 
			cout << "select error\n"; 
		} 
			
		// if something happened on sockfd, then its an incoming connection 
		if (FD_ISSET(sockfd, &readfds)) { 
			if ((newsockfd = accept(sockfd, (struct sockaddr *)&serv_addr, (socklen_t*)&serv_addr_size)) < 0) { 
				cout << "Error on accept\n"; 
				exit(1); 
			} 
			
			// inform user of socket number - used in send and receive commands 
			cout << "\nConnection accepted from " << inet_ntoa(serv_addr.sin_addr) << ":" << ntohs(serv_addr.sin_port) << ", client socket number " << newsockfd << endl;

			// add new socket to array of sockets 
			for (int i = 0; i < max_clients; i++) {
				// if position is empty 
				if (client_socket[i] == 0) { 
					client_socket[i] = newsockfd; 
					break; 
				} 
			} 
		} 
			
		// else its an operation on some other socket 
		for (int i = 0; i < max_clients; i++) { 
			
			sd = client_socket[i]; 
				
			if (FD_ISSET(sd, &readfds)) { 
				
				// check if it was for closing, and also read the incoming message 
                int valread;
				
				if ((valread = recv(sd, recv_buffer, BUFFER_SIZE, 0)) == 0) { 
					// somebody disconnected, get his details and print 
					getpeername(sd, (struct sockaddr*)&serv_addr, (socklen_t*)&serv_addr_size); 
					cout << "\nHost disconnected- ip " << inet_ntoa(serv_addr.sin_addr) << ", port " << ntohs(serv_addr.sin_port) << endl; 
						
					// close the socket and mark as 0 in list for reuse 
					close(sd); 
					client_socket[i] = 0; 
				} 
					
				// control information like FTP commands is received
				else { 
					cout << "\nClient socket " << newsockfd << " sent message: " << recv_buffer <<endl;
					
					struct CommandInfo ci = get_command_info(string(recv_buffer));
					bzero(recv_buffer, BUFFER_SIZE);
					bzero(send_buffer, BUFFER_SIZE);
					string msg; // the final message to be sent to client

					// command is not valid 
					if (!ci.is_valid) {
						send(sd, "0", BUFFER_SIZE, 0); // sending to client that command is invalid 
						msg += "ERROR: The command is not valid\n";
					}

					// command is valid. execute it.	
					else {
						send(sd, "1", BUFFER_SIZE, 0); // sending to client that command is valid 
						send(sd, ci.name, BUFFER_SIZE, 0); // sending command name to client
						msg += "The command is " + string(ci.name) + "\n";
						if (ci.has_file_arg) {
							msg += "The file name given is " + string(ci.filename_arg) + "\n";
						}

						// RETR : initiate a data connection and send the requested file over the data connection.
						if (strcasecmp(ci.name, "RETR") == 0) {

							send(sd, ci.filename_arg, BUFFER_SIZE, 0); // sending file name in command to client

							// stop operation if same filename is present in client's current directory (on receiving error code "0")
							recv(sd, recv_buffer, BUFFER_SIZE, 0);
							if (strcmp("0", recv_buffer) == 0) {
								msg += "ERROR: operation stopped as a file with the same name already exists in client's directory!\n";
							}
							else {
							
								FILE *fp;
								char data_buffer[BUFFER_SIZE];
								if ((fp = fopen(ci.filename_arg, "r")) != NULL) {
									// send info to client that required file is ready to be sent
									send(sd, "1", BUFFER_SIZE, 0); 

									// receive info from client whether required file was created successfully at client's directory
									recv(sd, recv_buffer, BUFFER_SIZE, 0);
									if (strcmp("1", recv_buffer) == 0) {
										msg += "File is being sent to client.\n";
										fseek(fp, 0, SEEK_END);
										int lSize = ftell(fp);
										rewind(fp);
										int num_blks = lSize / BUFFER_SIZE;
										int num_last_blk = lSize % BUFFER_SIZE;
										char char_num_blks[BUFFER_SIZE];
										sprintf(char_num_blks, "%d", num_blks);
										send(sd, char_num_blks, BUFFER_SIZE, 0);

										for (int i = 0; i < num_blks; i++) {
											fread(data_buffer, sizeof(char), BUFFER_SIZE, fp);
											send(sd, data_buffer, BUFFER_SIZE, 0);
										}

										char char_num_last_blk[BUFFER_SIZE];
										sprintf(char_num_last_blk, "%d", num_last_blk);
										send(sd, char_num_last_blk, BUFFER_SIZE, 0);
										if (num_last_blk > 0) {
											fread(data_buffer, sizeof(char), num_last_blk, fp);
											send(sd, data_buffer, BUFFER_SIZE, 0);
										}
										fclose(fp);
									}
									// required file could not be created successfully at client's directory
									else {
										msg += "ERROR: file could not be sent to client!\n";
									}
								}
								// file not found in current directory of server
								// send code 0 to convey this error message
								else {
									send(sd, "0", BUFFER_SIZE, 0);
									msg += "ERROR: file not found in directory of server!\n";
								}
							}
						}
						
						// STOR : store the file into the current directory of the remote host.
						else if (strcasecmp(ci.name, "STOR") == 0) {
							
							// stop operation if same filename is present in server's current directory
							if (access(ci.filename_arg, F_OK) == 0) {
								msg += "ERROR: operation stopped as a file with the same name already exists in server's directory!\n";
								send(sd, "0", BUFFER_SIZE, 0); // send error code "0" if filename is present at server already
							} 
							
							else {
								send(sd, "1", BUFFER_SIZE, 0); // send code "1" if filename is not present at server already
								send(sd, ci.filename_arg, BUFFER_SIZE, 0); // sending file name in command to client

								char check[BUFFER_SIZE];
								recv(sd, check, BUFFER_SIZE, 0); // check if file is available in client's directory
								if (strcmp("1", check) == 0) {
									FILE *fp;
									char data_buffer[BUFFER_SIZE];

									if ((fp = fopen(ci.filename_arg, "w")) == NULL) {
										// Error in creating file
										// send info to client that required file was not created with code 0						
										send(sd, "0", BUFFER_SIZE, 0);
										msg += "ERROR: server could not store the file!\n";
									}
									else {
										// send info to client that required file was created with code 1
										send(sd, "1", BUFFER_SIZE, 0);

										char char_num_blks[BUFFER_SIZE];
										recv(sd, char_num_blks, BUFFER_SIZE, 0);
										int num_blks = atoi(char_num_blks);
										for (int i = 0; i < num_blks; i++) {
											recv(sd, data_buffer, BUFFER_SIZE, 0);
											fwrite(data_buffer, sizeof(char), BUFFER_SIZE, fp);
										}

										char char_num_last_blk[BUFFER_SIZE];
										recv(sd, char_num_last_blk, BUFFER_SIZE, 0);
										int num_last_blk = atoi(char_num_last_blk);
										if (num_last_blk > 0)
										{
											recv(sd, data_buffer, BUFFER_SIZE, 0);
											fwrite(data_buffer, sizeof(char), num_last_blk, fp);
										}
										fclose(fp);
										msg += "File stored successfully in server's directory!\n";
									}
								}
								// file was not found in client's current directory
								else {
									msg += "ERROR: file not found in directory of client!\n";
								}
							}
						} 
						
						// LIST : display the list of all files present in the directory.
						else if (strcasecmp(ci.name, "LIST") == 0) {
							msg += "\nDisplaying list of contents present in server's directory:\n";
							msg += get_system_command_result("ls");
						} 
						
						// QUIT : terminate a USER, close the control connection.
						else if (strcasecmp(ci.name, "QUIT") == 0) {
							// somebody disconnected, get his details and print 
							getpeername(sd, (struct sockaddr*)&serv_addr, (socklen_t*)&serv_addr_size); 
							msg += "The user is going to be terminated.";
							cout << msg;
							cout << "\nHost disconnected- ip " << inet_ntoa(serv_addr.sin_addr) << ", port " << ntohs(serv_addr.sin_port) << endl; 
								
							// close the socket and mark as 0 in list for reuse 
							close(sd); 
							client_socket[i] = 0;
						} 
						
						// DELE : delete the file in the current directory of server.
						else if (strcasecmp(ci.name, "DELE") == 0) {
							
							// file exists in server's directory
							if (access(ci.filename_arg, F_OK) == 0) {
								// file is deleted successfully
								if (remove(ci.filename_arg) == 0) {
									msg += "File sucessfully deleted from server's directory.\n";
								}
								else {
									msg += "ERROR: file could not be deleted.\n";
								}
							}
							else {
								msg += "ERROR: the file could not be found in server's directory.\n";
							}
							
						} 

						// CODEJUD command
						else if (strcasecmp(ci.name, "CODEJUD") == 0) {
							msg += "The extension is " + string(ci.ext) + "\n";
							// bring the file to server using STOR
							// stop operation if same filename is present in server's current directory
							if (access(ci.filename_arg, F_OK) == 0) {
								msg += "ERROR: cannot send the program file to server as a file with the same name already exists in server's directory!\n";
								send(sd, "0", BUFFER_SIZE, 0); // send error code "0" if filename is present at server already
							} 
							
							else {
								send(sd, "1", BUFFER_SIZE, 0); // send code "1" if filename is not present at server already
								send(sd, ci.filename_arg, BUFFER_SIZE, 0); // sending file name in command to client

								char check[BUFFER_SIZE];
								recv(sd, check, BUFFER_SIZE, 0); // check if file is available in client's directory
								if (strcmp("1", check) == 0) {
									FILE *fp;
									char data_buffer[BUFFER_SIZE];

									if ((fp = fopen(ci.filename_arg, "w")) == NULL) {
										// Error in creating file
										// send info to client that required file was not created with code 0						
										send(sd, "0", BUFFER_SIZE, 0);
										msg += "ERROR: server could not store the program file!\n";
									}
									else {
										// send info to client that required file was created with code 1
										send(sd, "1", BUFFER_SIZE, 0);

										char char_num_blks[BUFFER_SIZE];
										recv(sd, char_num_blks, BUFFER_SIZE, 0);
										int num_blks = atoi(char_num_blks);
										for (int i = 0; i < num_blks; i++) {
											recv(sd, data_buffer, BUFFER_SIZE, 0);
											fwrite(data_buffer, sizeof(char), BUFFER_SIZE, fp);
										}

										char char_num_last_blk[BUFFER_SIZE];
										recv(sd, char_num_last_blk, BUFFER_SIZE, 0);
										int num_last_blk = atoi(char_num_last_blk);
										if (num_last_blk > 0)
										{
											recv(sd, data_buffer, BUFFER_SIZE, 0);
											fwrite(data_buffer, sizeof(char), num_last_blk, fp);
										}
										fclose(fp);
										
										// file stored successfully in server's directory
										// now compile and run it. then match with the testcases
										string code_filename_without_extension = string(ci.filename_without_extension);
										string code_extension;
										if (strcmp(ci.ext, "c") == 0) {
											code_extension = "c";
										}
										else if (strcmp(ci.ext, "c++") == 0) {
											code_extension = "cpp";
										}
										
										string code_executable_filename = "executable_" + code_filename_without_extension;
										string code_output_filename = "output_" + code_filename_without_extension + ".txt";
										string code_compilation_errors_filename = "compilation_errors_" + code_filename_without_extension + ".txt";
										string code_input_filename = "input_" + code_filename_without_extension + ".txt"; 
										string code_testcase_filename = "testcase_" + code_filename_without_extension + ".txt"; 
										
										string compiler_name;
										if (strcmp("cpp", code_extension.c_str()) == 0) compiler_name = "g++";
										else if (strcmp("c", code_extension.c_str()) == 0) compiler_name = "gcc";
										string compilation_command = compiler_name + " " + code_filename_without_extension + "." + code_extension + " -o " + code_executable_filename + " 2> " + code_compilation_errors_filename;

										bool run_error_occurred = false;
										bool input_file_found = true;
										bool code_run_success = true;
										bool code_time_limit_not_exceeded = true;

										msg += "\nCOMPILE PHASE STATUS:\n";

										int compilation_call_status = system(compilation_command.c_str());
										get_system_command_result(compilation_command.c_str());

										// compilation error occured
										if (compilation_call_status != 0) {
											msg += "COMPILE_ERROR\n";
											// read the errors and send them to client 
											FILE *compilation_errors_fp = fopen(code_compilation_errors_filename.c_str(), "r");
											if (compilation_errors_fp != NULL) {
												char buffer[BUFFER_SIZE];
												msg += "\nThe compilation errors are shown below:\n\n";
												while (fgets(buffer, BUFFER_SIZE, compilation_errors_fp) != NULL) {
													msg += buffer;
												}
											}
											fclose(compilation_errors_fp);
											// delete the compilation errors file
											if (access(code_compilation_errors_filename.c_str(), F_OK) == 0) {
												if (remove(code_compilation_errors_filename.c_str()) == 0) {
													; // file successfully deleted
												}
												else {
													cout << "ERROR: file containing the compilation errors could not be deleted!\n";
												}
											}
										}
										// compiled with or without warnings. proceed to execution phase.
										else {
											msg += "COMPILE_SUCCESS\n";
											FILE *compilation_errors_fp = fopen(code_compilation_errors_filename.c_str(), "r");
											
											// check the size of compilation errors file (warnings were there if its not empty)
											if (compilation_errors_fp != NULL) {           
												fseek (compilation_errors_fp, 0, SEEK_END); // goto end of file
												int size = ftell(compilation_errors_fp);
												
												// compilation was sucessful without any warnings
												if (size == 0) {
													msg += "(the program was compiled successfully without any warning)\n\n";
												}
												// else read the warnings from compilation errors file and send to client
												else {
													fseek(compilation_errors_fp, 0, SEEK_SET); // goto begin of file
													char buffer[BUFFER_SIZE];
													msg += "But the compilation gave following warnings:\n\n";
													while (fgets(buffer, BUFFER_SIZE, compilation_errors_fp) != NULL) {
														msg += buffer;
													}
													msg += "\n";
												}
												fclose(compilation_errors_fp);
												// delete the compilation errors file
												if (access(code_compilation_errors_filename.c_str(), F_OK) == 0) {
													if (remove(code_compilation_errors_filename.c_str()) == 0) {
														; // file successfully deleted
													}
													else {
														cout << "ERROR: file containing the compilation errors could not be deleted!\n";
													}
												}
											}

											// scan each line of code_input_filename and create a file containing only that line
											// redirect the input from the newly created files and run the executable for each file
											FILE *code_output_file_fp = fopen(code_output_filename.c_str(), "w");
											if (code_output_file_fp == NULL) {
												msg = "ERROR: could not create file to store output of the program in server's directory!\n";
											}
											FILE *code_input_file_fp = fopen(code_input_filename.c_str(), "r");
											if (code_input_file_fp == NULL) {
												msg = "ERROR: input file for the program not found in server's directory!\n";
												input_file_found = false;
											}															
											// proceed to EXECUTION PHASE
											else {
												msg += "EXECUTION PHASE STATUS:\n";
												char input_file_buffer[BUFFER_SIZE];
												string oneline_input_filename = "temp_" + code_input_filename;
												string one_testcase_output_filename = "temp_" + code_output_filename;
												
												// scan each line of input file and execute and get output 
												while (fgets(input_file_buffer, BUFFER_SIZE, code_input_file_fp) != NULL) {
													
													// check if line is blank
													bool is_blank_line = true;
													for (int i = 0; i < strlen(input_file_buffer); i++) {
														if (!(input_file_buffer[i] == ' ' || input_file_buffer[i] == '\n' || input_file_buffer[i] == '\t')) {
															is_blank_line = false;
															break;
														}
													}
													if (is_blank_line) {
														continue;
													}

													FILE *oneline_input_file_fp = fopen(oneline_input_filename.c_str(), "w");
													if (oneline_input_file_fp == NULL) {
														msg = "ERROR: cannot create temporary file to store input!\n";
													}
													else {
														fprintf(oneline_input_file_fp, "%s", input_file_buffer);
														fclose(oneline_input_file_fp);

														// run the executable. take input from oneline_input_filename. write output to oneline_output_filename (this is input redirection)
														string run_command = "timeout 1 ./" + code_executable_filename + " 0< " + oneline_input_filename + " 1> " + one_testcase_output_filename;
														
														// check for RUN_ERROR 
														int run_command_status = system(run_command.c_str());
														// if TLE
														if (run_command_status == 31744 || run_command_status == 36608) {
															code_time_limit_not_exceeded = false;
															// TLE on this testcase
															fprintf(code_output_file_fp, "%s", "TIME_LIMIT_EXCEEDED\n");
														}
														// if RUN_ERROR
														else if (run_command_status != 0) { 
															code_run_success = false;
															fprintf(code_output_file_fp, "%s", "RUN_ERROR\n");
														}
														
														// else RUN_SUCCESS. Get the output
														else if (run_command_status == 0) {
															get_system_command_result(run_command.c_str()); // stores the program output in oneline_output_filename
																												
															// access the testcase's output file and append result to main output file
															if (access(one_testcase_output_filename.c_str(), F_OK) == 0) {
																// RUN_SUCCESS on this testcase
																FILE *one_testcase_output_file_fp = fopen(one_testcase_output_filename.c_str(), "r");
																char one_testcase_output_filebuffer[BUFFER_SIZE];
																while (fgets(one_testcase_output_filebuffer, BUFFER_SIZE, one_testcase_output_file_fp) != NULL) {
																	fprintf(code_output_file_fp, "%s", one_testcase_output_filebuffer);
																}
																fclose(one_testcase_output_file_fp);
															}
														}
														// delete the temporary files
														if (access(oneline_input_filename.c_str(), F_OK) == 0) {
															if (remove(oneline_input_filename.c_str()) == 0) {
																;// file successfully deleted
															}
															else {
																cout << "ERROR: temporary input file could not be deleted!\n";
															}
														}
														if (access(one_testcase_output_filename.c_str(), F_OK) == 0) {
															if (remove(one_testcase_output_filename.c_str()) == 0) {
																;// file successfully deleted
															}
															else {
																cout << "ERROR: temporary output file could not be deleted!\n";
															}
														}
													}
												}
												if (code_input_file_fp) {
													fclose(code_input_file_fp);
												}
												if (code_output_file_fp) {
													fclose(code_output_file_fp);
												}

												if (!code_time_limit_not_exceeded) {
													msg += "TIME_LIMIT_EXCEEDED\n";
												}
												else if (!code_run_success) {
													msg += "RUN_ERROR\n";
												}
												else if (code_run_success && code_time_limit_not_exceeded) {
													msg += "RUN_SUCCESS\n";
												}
												
												// proceed to MATCHING PHASE (skip if input file or testcase file could not be found or run time error or tle occured)
												// scan the output file and match with testcase file
												if (code_run_success && code_time_limit_not_exceeded && input_file_found && access(code_output_filename.c_str(), F_OK) == 0 && access(code_testcase_filename.c_str(), F_OK) == 0) {
													FILE *code_output_fp = fopen(code_output_filename.c_str(), "r");
													FILE *code_testcase_fp = fopen(code_testcase_filename.c_str(), "r");
													char output_file_buffer[BUFFER_SIZE];
													char testcase_file_buffer[BUFFER_SIZE];
													bool output_finished = false;
													bool testcase_finished = false;
													bool output_matches_testcase = true;
													while (1) {
														// get a line from output file
														if (fgets(output_file_buffer, BUFFER_SIZE, code_output_fp) != NULL) {
															;
														}
														else {
															output_finished = true;
														}

														// get a line from testcase file
														if (fgets(testcase_file_buffer, BUFFER_SIZE, code_testcase_fp) != NULL) {
															;
														}
														else {
															testcase_finished = true;
														}

														if (testcase_finished && output_finished) {
															break;
														}
														else if (testcase_finished) {
															// remaining lines in output should all be blank
															while (fgets(output_file_buffer, BUFFER_SIZE, code_output_fp) != NULL) {
																bool is_blank_line = true;
																for (int i = 0; i < strlen(output_file_buffer); i++) {
																	if (!(output_file_buffer[i] == ' ' || output_file_buffer[i] == '\n' || output_file_buffer[i] == '\t' || testcase_file_buffer[i] == '\r')) {
																		is_blank_line = false;
																		break;
																	}
																}
																if (is_blank_line) {
																	continue;
																}
																else {
																	output_matches_testcase = false;
																	break;
																}
															}
															break;
														}
														else if (output_finished) {
															// remaining lines in testcase should all be blank
															while (fgets(testcase_file_buffer, BUFFER_SIZE, code_testcase_fp) != NULL) {
																bool is_blank_line = true;
																for (int i = 0; i < strlen(testcase_file_buffer); i++) {
																	if (!(testcase_file_buffer[i] == ' ' || testcase_file_buffer[i] == '\n' || testcase_file_buffer[i] == '\t' || testcase_file_buffer[i] == '\r')) {
																		is_blank_line = false;
																		break;
																	}
																}
																if (is_blank_line) {
																	continue;
																}
																else {
																	output_matches_testcase = false;
																	break;
																}
															}
															break;
														}

														// compare the 2 selected lines
														for (int i = 0; i < strlen(output_file_buffer); i++) {
															if (output_file_buffer[i] != testcase_file_buffer[i]) {
																if (output_file_buffer[i] == '\n' && testcase_file_buffer[i] == '\r') {
																	continue;
																}
																if (output_file_buffer[i] == '\n' && testcase_file_buffer[i] == '\0') {
																	continue;
																}
																output_matches_testcase = false; // mismatch found
																break;
															}
														}
														if (!output_matches_testcase) {
															break;
														}
													}
													msg += "\nMATCHING PHASE STATUS:\n";
													if (output_matches_testcase) {
														msg += "ACCEPTED\n";
													} 
													else {
														msg += "WRONG_ANSWER\n";
													}
										
													if (code_output_fp) {
														fclose(code_output_fp);
													}
													if (code_testcase_fp) {
														fclose(code_testcase_fp);
													}
												}
												else if (access(code_testcase_filename.c_str(), F_OK) != 0) {
													msg = "ERROR: testcase file for the program not found in server's directory!\n";
												}
												else if (access(code_output_filename.c_str(), F_OK) != 0) {
													msg = "ERROR: output file could not be generated for the program!\n";
												}
											}
											// delete the executable file 
											if (access(code_executable_filename.c_str(), F_OK) == 0) {
												if (remove(code_executable_filename.c_str()) == 0) {
													; // file successfully deleted
												}
												else {
													cout << "ERROR: the executable file for the program could not be deleted!\n";
												}
											}
										}	
										// finally, delete the program file
										if (remove(ci.filename_arg) == 0) {
											;// file successfully deleted
										}
										else {
											cout << "ERROR: The program file sent could not be deleted from server's directory!\n";
										}
									}
								}
								// file was not found in client's current directory
								else {
									msg += "ERROR: file not found in directory of client!\n";
								} 
							}
						}
					}				
					
					strcpy(send_buffer, msg.c_str());
                    if ((ci.is_valid && strcasecmp(ci.name, "QUIT") != 0) || !ci.is_valid) {
						cout << "Sending reply to client:\n" << send_buffer << endl;
                    	send(sd, send_buffer, BUFFER_SIZE, 0);
					}
				} 
			} 
		} 
	} 
		
	return 0; 
} 
