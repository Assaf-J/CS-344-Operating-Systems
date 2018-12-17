/******************************************************************************************
*Name: Jennifer Assaf
*Date:11/30/2018
*
*Desrciption: This is the daemon file meant to be run in the background. It allows users to
*to decode an encrypted file using a key. The user must pass in a ciphertext, key, and correct
*port number
*******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


char* decrypt(char*, char*, int);
char getChar(int);
int getInt(char);
void recvChecker(int, char*, int);
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

//main function that sets up the port and allows for program functionality
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	//buffers that let user pass in up to 70001 lettes
	char buffer[70001];
	char keybuffer[70001];
	pid_t childPid;
	int keySize, inputSize;
	char handshake;
	char permission;

	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr, "USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	//loop that allows server to continue running 
	while (1)
	{
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//fork into new child to allow for more connections
		childPid = fork();

		switch (childPid)
		{
			//if fork was unsuccsful, exit
		case -1:
			perror("Fork Unsuccessful");
			exit(1);
			break;

			//if fork was successful
		case 0:
			//recieve handshake--check to see if connection is valid
			recv(establishedConnectionFD, &handshake, sizeof(char), 0);

			//if connection is decode-'d', then set permission to true, if not set to false
			if (handshake == 'd')
			{
				permission = 't';

			}
			else
			{
				permission = 'f';

			}
			//send permission status to otp_dec
			send(establishedConnectionFD, &permission, sizeof(char), 0);

			//if connection was valid, otp_dec will send size of both files, 
			//sizes will be save to inputSize and keysize
			recv(establishedConnectionFD, &inputSize, sizeof(int), 0);
			recv(establishedConnectionFD, &keySize, sizeof(int), 0);


			//check if entire input was received
			int recvBytes;
			//while loop to ensure that total received bytes for input is what we expect
			while (recvBytes != (inputSize * sizeof(char)))
			{
				//set total recvBytes to how many were recieved 
				recvBytes += recv(establishedConnectionFD, buffer, (inputSize * sizeof(char)), 0);
				if (recvBytes < 0)
				{
					fprintf(stderr, "otp_dec_d: Error reading input");
					exit(1);
				}
			}
			//while loop to ensure that total received bytes for key is what we expect
			int recvBytes2;
			while (recvBytes2 != (keySize * sizeof(char)))
			{
				recvBytes2 += recv(establishedConnectionFD, keybuffer, (keySize * sizeof(char)), 0);
				if(recvBytes2 < 0)
				{
					fprintf(stderr, "otp_dec_d: Error reading key");
					exit(1);
				}
			}


			//string useed to store decryption
			char* decryption = decrypt(buffer, keybuffer, inputSize);
			//send decryption to otp_dec
			send(establishedConnectionFD, decryption, inputSize * sizeof(char), 0);


			exit(0);

			break;

		default:

			break;
		}

		close(establishedConnectionFD); // Close the listening socket

	}
	close(listenSocketFD);
	return 0;

}

//functtion to decrypt input sent by otp_dec
//this function needs a string input, a key, and the inputs size
//in order to decrypt
char* decrypt(char* input, char* key, int size)
{
	//create a string variable using size of input
	char* cipher = calloc(sizeof(input), sizeof(char*));

	int i;
	for (i = 0; i < size - 1; i++)
	{
		//get letter inputs for input at i and key at i
		int letter1 = getInt(input[i]);

		int letter2 = getInt(key[i]);

		//calculate modulus
		int mod = ((letter1 - letter2) % 27);

		//if modulus is less than 0, add 27 so there are no negative numbers
		if (mod < 0)
		{
			mod += 27;
		}

		//get corresponding letter to mod number
		char cipherLetter = getChar(mod);

		//save letter to cipher string
		cipher[i] = cipherLetter;

	}
	//add newline at the end of the string 
	cipher[size - 1] = '\n';

	return cipher;
}

//function used to get corresponding letter for integer
int getInt(char let)
{
	//array of all capital letters including a space at the end
	char letters[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ " };

	//search array, if letter matches, send int location 
	//of letter
	int j;
	for (j = 0; j < 27; j++)
	{

		if (let == letters[j])
		{

			return j;

		}
	}

}

//function used to get corresponding integer from a letter
char getChar(int letterInt)
{
	//array of all capital letters with a space at the end
	char letters[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ " };

	//get letter at array location sent in
	return letters[letterInt];
}
