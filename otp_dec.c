/******************************************************************************************
*Name: Jennifer Assaf
*Date:11/30/2018
*
*Desrciption: This is the client file meant to be run with otp_dec_d. It passes a key and 
*encoded file to otp_dec_d. It then receives and prints the decoded file.
*******************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include<sys/stat.h>


void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char decryption[70001];
	char handshake = 'd';
	char permission;

	if (argc < 3) { fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

																					  // Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

																											// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	//send char of which file this is, to be used to check for compatibility
	send(socketFD, &handshake, sizeof(char), 0);
	//recieve whether or not this file is correct
	recv(socketFD, &permission, sizeof(char), 0);
	//if permission is not true, connection is not valid, terminate program
	if (permission !='t')
	{
		fprintf(stderr,"This is not a valid connection");
		exit(1);

	}

	else
	{
		//open files and get total length of both key and input plaintext
		int key = open(argv[2], O_RDONLY);          
		int keySize = lseek(key, 0, SEEK_END);
		//close file
		close(key);

		int input = open(argv[1], O_RDONLY);         
		int inputSize = lseek(input, 0, SEEK_END);
		close(input);

		//check to see if file contains invalid characters
		//code for how to read files byte wise modified from stack overflow
		int file;
		if ((file = open(argv[1], O_RDWR)) >= 0)
		{
			char c;
			while (read(file, &c, 1) == 1)
			{
				//if file contains invalid characters, print to stderr, and exit program
				if (c != ' ' && c != '\n' && isupper(c) == 0)
				{
					fprintf(stderr, "File %s contains invalid characters", argv[1]);
					exit(1);
				}
			}

		}
		//if input is greater than key size, print to stderr and exit program
		if (inputSize > keySize)
		{
			fprintf(stderr, "Error: key '%s' is too small", argv[2]);
			exit(1);
		}

		//send sizes of both input and keysize
		send(socketFD, &inputSize, sizeof(int), 0);
		send(socketFD, &keySize, sizeof(int), 0);

		//open and read files and save to array
		//code for how to read files line by line modified from stack overflow
		FILE *fptr = fopen(argv[1], "r");
		char line[100];
		char* inputArray = calloc(inputSize, sizeof(char*));
		while (fscanf(fptr, "%79[^\n]\n", line) == 1) {
			strcat(inputArray, line);
			strcat(inputArray, "\n");
		}
		//close file
		fclose(fptr);
		
		//open and read file for key
		FILE *fptr2 = fopen(argv[2], "r");
		char line2[100];
		char* keyArray = calloc(keySize, sizeof(char*));
		while (fscanf(fptr2, "%79[^\n]\n", line2) == 1) {
			strcat(keyArray, line2);
			strcat(keyArray, "\n");
		}
		fclose(fptr2);

		//send input to otp_dec
		if (send(socketFD, inputArray, inputSize, 0) < 0)
		{
			fprintf(stderr, "could not send file %s", argv[1]);
			exit(1);
		}
		//send key to otp_dec
		if (send(socketFD, keyArray, keySize, 0) < 0)
		{
			fprintf(stderr, "could not send file %s", argv[2]);
			exit(1);
		}
		//free memory
		free(inputArray);
		free(keyArray);

		//recieve decrypted text
		recv(socketFD, decryption, inputSize * sizeof(char), 0);
		//print decryption
		printf("%s", decryption);
	
	}
		
	close(socketFD); // Close the socket
	return 0;
}


