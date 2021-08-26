/*
NAME- Soumya Porel
ROLL- 20CS60R36
Assignement 4 (client program)
*/

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 2048

using namespace std;

int main(int argc, char **argv) {

    if (argc < 3) {
		cout << "ERROR: please provide all the command line arguments!\n";
		exit(0);
	}
    int portno = atoi(argv[2]); // get port number from command line
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM for TCP. protocol=0 for TCP
    if (sockfd < 0) {
        cout << "Error in creating socket\n";
        exit(1);
    }

    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        cout << "no such host!\n";
        exit(1);
    }

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr)); // copy server IP address
    serv_addr.sin_family = AF_INET; // for IPv4 family
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length); // copy server IP addresss
    serv_addr.sin_port = htons(portno); // defining port number
    
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { // initiating connect request to the server
        cout << "connection failed!\n";
        exit(1);
    }  
    cout << "Connected to server\n";

    char send_buffer[BUFFER_SIZE]; // client buffer to forward request to the server
    char recv_buffer[BUFFER_SIZE]; // client buffer to get reply from the server
    char final_msg[BUFFER_SIZE];
    bzero(send_buffer, BUFFER_SIZE);
    bzero(recv_buffer, BUFFER_SIZE);
    bzero(final_msg, BUFFER_SIZE);

    while (1) {

        cout << "\nPlease enter the message to the server: ";
        bzero(send_buffer, BUFFER_SIZE);
        fgets(send_buffer, BUFFER_SIZE, stdin);
        send(sockfd, send_buffer, BUFFER_SIZE, 0);

        // receive info on if the command is valid
        recv(sockfd, recv_buffer, BUFFER_SIZE, 0);

        if (strcmp("1", recv_buffer) == 0) {
            
            // receive info on the command name
            recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
            char command_name[BUFFER_SIZE];
            strcpy(command_name, recv_buffer);

            if (strcasecmp("RETR", command_name) == 0) {

                // receive info on the file name argument in command
                recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
                char command_filename_arg[BUFFER_SIZE];
                strcpy(command_filename_arg, recv_buffer);

                // stop operation if same filename is present in client's current directory
				if (access(command_filename_arg, F_OK) == 0) {
                    send(sockfd, "0", BUFFER_SIZE, 0); // send error code "0"
                }

                else {
                    send(sockfd, "1", BUFFER_SIZE, 0); // filename is not present in client's directory. send code "1"

                    char data_buffer[BUFFER_SIZE], char_num_blks[BUFFER_SIZE], char_num_last_blk[BUFFER_SIZE];
                
                    char check[BUFFER_SIZE];
                    recv(sockfd, check, BUFFER_SIZE, 0);
                    FILE *fp;
                    if (strcmp("1", check) == 0) {
                        
                        if ((fp = fopen(command_filename_arg, "w")) == NULL) {
                            // Error in creating file
                            // send info to server that required file was not created with code 0
                            send(sockfd, "0", BUFFER_SIZE, 0);
                        }

                        else {
                            // send info to server that required file was created with code 1
                            send(sockfd, "1", BUFFER_SIZE, 0);

                            recv(sockfd, char_num_blks, BUFFER_SIZE, 0);
                            int num_blks = atoi(char_num_blks);
                            for (int i = 0; i < num_blks; i++) {
                                recv(sockfd, data_buffer, BUFFER_SIZE, 0);
                                fwrite(data_buffer, sizeof(char), BUFFER_SIZE, fp);
                            }
                            recv(sockfd, char_num_last_blk, BUFFER_SIZE, 0);
                            int num_last_blk = atoi(char_num_last_blk);
                            if (num_last_blk > 0) {
                                recv(sockfd, data_buffer, BUFFER_SIZE, 0);
                                fwrite(data_buffer, sizeof(char), num_last_blk, fp);
                            }
                            fclose(fp);
                            strcpy(final_msg, "\nFile retrieved successfully in client's directory!\n");
                        }
                    }
                    // file was not found in current directory of server
                    // do nothing
                    else {
                        ;
                    }
                }
            }

            else if (strcasecmp("STOR", command_name) == 0) {

                // receive info of if the same filename already exists in server's directory
                recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
                // server will stop operation. do nothing in client.
                if (strcmp("0", recv_buffer) == 0) {
                    ;
                }
                else {
                    // receive info on the file name argument in command
                    recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
                    char command_filename_arg[BUFFER_SIZE];
                    strcpy(command_filename_arg, recv_buffer);

                    char data_buffer[BUFFER_SIZE], char_num_blks[BUFFER_SIZE], char_num_last_blk[BUFFER_SIZE];
                    
                    FILE *fp;
                    if ((fp = fopen(command_filename_arg, "r")) != NULL) {
                        // file is successfully being read to be sent to server
                        send(sockfd, "1", BUFFER_SIZE, 0);

                        // check if the required file was created at server
                        recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
                        if (strcmp("1", recv_buffer) == 0) {
                            
                            cout << "\nFile is being sent to server...\n";
                        
                            fseek(fp, 0, SEEK_END);
                            int lSize = ftell(fp);
                            rewind(fp);
                            int num_blks = lSize / BUFFER_SIZE;
                            int num_last_blk = lSize % BUFFER_SIZE;
                            sprintf(char_num_blks, "%d", num_blks);
                            send(sockfd, char_num_blks, BUFFER_SIZE, 0);

                            for (int i = 0; i < num_blks; i++) {
                                fread(data_buffer, sizeof(char), BUFFER_SIZE, fp);
                                send(sockfd, data_buffer, BUFFER_SIZE, 0);
                            }
                            sprintf(char_num_last_blk, "%d", num_last_blk);
                            send(sockfd, char_num_last_blk, BUFFER_SIZE, 0);
                            if (num_last_blk > 0) {
                                fread(data_buffer, sizeof(char), num_last_blk, fp);
                                send(sockfd, data_buffer, BUFFER_SIZE, 0);
                            }
                            fclose(fp);
                        }
                        // required file could not be created at server. do nothing in client.
                        else {
                            ;
                        }
                    }
                    // file not found in client's current directory.
                    // send code 0 to convey this error message
                    else {
                        send(sockfd, "0", BUFFER_SIZE, 0);
                    }
                }
            }

            else if (strcasecmp("CODEJUD", command_name) == 0) {

                // receive info of if the same filename already exists in server's directory
                recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
                // server will stop operation. do nothing in client.
                if (strcmp("0", recv_buffer) == 0) {
                    ;
                }
                else {
                    // receive info on the file name argument in command
                    recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
                    char command_filename_arg[BUFFER_SIZE];
                    strcpy(command_filename_arg, recv_buffer);

                    char data_buffer[BUFFER_SIZE], char_num_blks[BUFFER_SIZE], char_num_last_blk[BUFFER_SIZE];
                    
                    FILE *fp;
                    if ((fp = fopen(command_filename_arg, "r")) != NULL) {
                        // file is successfully being read to be sent to server
                        send(sockfd, "1", BUFFER_SIZE, 0);

                        // check if the required file was created at server
                        recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
                        if (strcmp("1", recv_buffer) == 0) {
                            
                            cout << "\nFile is being sent to server...\n";
                        
                            fseek(fp, 0, SEEK_END);
                            int lSize = ftell(fp);
                            rewind(fp);
                            int num_blks = lSize / BUFFER_SIZE;
                            int num_last_blk = lSize % BUFFER_SIZE;
                            sprintf(char_num_blks, "%d", num_blks);
                            send(sockfd, char_num_blks, BUFFER_SIZE, 0);

                            for (int i = 0; i < num_blks; i++) {
                                fread(data_buffer, sizeof(char), BUFFER_SIZE, fp);
                                send(sockfd, data_buffer, BUFFER_SIZE, 0);
                            }
                            sprintf(char_num_last_blk, "%d", num_last_blk);
                            send(sockfd, char_num_last_blk, BUFFER_SIZE, 0);
                            if (num_last_blk > 0) {
                                fread(data_buffer, sizeof(char), num_last_blk, fp);
                                send(sockfd, data_buffer, BUFFER_SIZE, 0);
                            }
                            fclose(fp);

                            // file sent to server
                        }
                        // required file could not be created at server. do nothing in client.
                        else {
                            ;
                        }
                    }
                    // file not found in client's current directory.
                    // send code 0 to convey this error message
                    else {
                        send(sockfd, "0", BUFFER_SIZE, 0);
                    }
                }
            }

            else if (strcasecmp("QUIT", command_name) == 0) {
                close(sockfd);
                cout << "Disconnected from server.\n";
                exit(1);
            }

        }

        int valread = recv(sockfd, recv_buffer, BUFFER_SIZE, 0);
        if (valread == 0) { 
            // server disconnected, exit!
            cout << "\nServer was terminated!\nClient exiting...\n";
            close(sockfd);
            exit(1);
		} 
        else if (valread < 0) {
                cout << "error on read\n";
                exit(1);
        }
        else {
            cout << "\nServer replied:\n" << recv_buffer;
            cout << final_msg;
            bzero(final_msg, BUFFER_SIZE);
            bzero(recv_buffer, BUFFER_SIZE);
        }
    }

    return 0;
}


