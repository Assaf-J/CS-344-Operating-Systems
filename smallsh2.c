/*
Name: Jennifer Assaf
Date:11/14/2018
Description: This program is a C shell script. It has three built in commands,
exit, status, and cd. It also allows for non built in functions/
*/
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

typedef int bool;
#define true 1
#define false 0



//file variables, allow user to open files
char* inputFile;
char* outputFile;

//current location in parsed array
int currLoc = 0;
char* parsed[512];
char* token;
char inputs[512];


bool background = true;
bool inputFilePresent = false;
bool outputFilePresent = false;

void catchSIGSTOP(int signo);
void status(int signo);
void checkFiles(int fCheck);
void getFiles();
void clearMem();
bool getInput(bool output);
void processKiller(int pStatus);


//function to catch ctrl-Z. If background is on, this code was modified from lecture materia
void catchSIGSTOP(int signo) {

	if (background == true)
	{
		char* message = "Exiting foreground only mode.\n";
		write(STDOUT_FILENO, message, sizeof(message));
		background = false;


	}
	else
	{
		char* message = "Entering foreground only mode. (& is now ignored)\n";
		write(STDOUT_FILENO, message, sizeof(message));
		background = true;

	}
}
//status function taken from lecture material, prints exit status of process
void status(int signo)
{
	if (WIFEXITED(signo) != 0)
	{
		printf("exit value %d\n", WEXITSTATUS(signo));
		fflush(stdout);
	}
	else

		printf("terminated by signal %d\n", signo);
	fflush(stdout);

}
//function to check file status
void checkFiles(int fCheck)
{
	if (fCheck == -1)
	{
		printf("Error opening file\n");
		fflush(stdout);
		exit(1);
	}
}

//function to open files, if foreground only mode is set, it opens
//dev/null otherwise it opens what users have provided as input
//code on checking dup for files modified from geeksforgeeks.
void getFiles()
{
	//if foreground only mode is on. It opens dev/null
	if (background == false) {
		int fd = -5;
		fd = open("/dev/null", O_RDONLY);

		checkFiles(fd);
	}
	//checks if input file has been provided, it opens and check
	//if the file has opened correctly
	else if (inputFilePresent == true)
	{
		int fd = -5;
		fd = open(inputFile, O_RDONLY);
		//check file and dup status
		checkFiles(fd);
		checkFiles(dup2(fd, 0));
		//close file
		close(fd);
	}
	//if an outputfile has  been provided
	if (outputFilePresent == true)
	{
		int fd = -5;
		fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0755);
		checkFiles(fd);
		checkFiles(dup2(fd, 1));
		close(fd);
	}


}
//function to clear all memory from char * array
void clearMem()
{
	int i = 0;
	while (parsed[i] != NULL)
	{
		free(parsed[i]);
		i++;
	}
}

//Function to get user input using fgets. It checks if the user has entered a 
//<,>, & or $$. If not than it adds the input to the array.
//code to change integers to string modified from stack overflow
bool getInput(bool output)
{
	//reset global variable to 0
	currLoc = 0;
	//get input from user
	if (fgets(inputs, 512, stdin) != NULL)
	{

		//check is user is trying to get pid
		if (strstr(inputs, "$$") != NULL)
		{
			//variable to hold pid as a string
			char shellPid[20];
			sprintf(shellPid, "%d", getpid());

			fflush(stdout);
			//get lengths of both strings, set old to -3 to remove $$\n
			int oldLen = strlen(inputs) - 3;
			int pidLen = strlen(shellPid);
			int i;

			//create new string variable
			char* result = malloc(512);
			//copy old string
			strncpy(result, inputs, oldLen);
			//concatenate pid
			strcat(result, shellPid);
			//save to input variable
			strncpy(inputs, result, (512));


		}


		//get next string token
		token = strtok(inputs, " \n");

		//if token is empty, set output to false, and do not print
		while (token != NULL)
		{
			if (strncmp(token, "#", 1) == 0)
			{
				output = false;
			}
			//if user has entered in an inputfile
			else if (strcmp(token, "<") == 0)
			{
				token = strtok(NULL, " \n");
				//save inputfile 
				inputFile = token;
				//set boolean input as true
				inputFilePresent = true;
			}
			//if user has entered in an outputfile
			else if (strcmp(token, ">") == 0)
			{
				token = strtok(NULL, " \n");
				outputFile = token;
				outputFilePresent = true;

			}

			//if user wants to use foreground only mode
			else if (strcmp(token, "&") == 0)
			{
				//set boolean background to false
				background = false;
				break;
			}
			else {
				//save token and increment location
				parsed[currLoc] = strdup(token);
				currLoc++;
			}
			token = strtok(NULL, " \n");
		}
		parsed[currLoc] = NULL;
		return output;
	}
	//user did not input anything
	else
	{
		output = false;
	}
}

//function to check if processes have completed. It then sends the status to the status function
void processKiller(int pStatus)
{
	pid_t processesLeft;
	processesLeft = waitpid(-1, &pStatus, WNOHANG);
	while (processesLeft > 0) {
		printf("background process, %i, is done: ", processesLeft);
		fflush(stdout);
		status(pStatus);
		processesLeft = waitpid(-1, &pStatus, WNOHANG);
	}


}
//main funciton
int main() {

	//pid variables
	pid_t childPid = -5;
	pid_t mainPid = -5;

	//variable to keep track of status
	int pStatus = 0;

	//boolean that lets shell loop procees
	bool proceed = true;

	//sigactions - allow ctrl-c and ctrl -z functionality

	struct sigaction ignore_action = { 0 }, stop_action = { 0 }, default_action = { 0 };


	//action handlers-	//code modified from lecture material
	ignore_action.sa_handler = SIG_IGN;
	stop_action.sa_handler = catchSIGSTOP;
	default_action.sa_handler = SIG_DFL;

	sigfillset(&(ignore_action.sa_mask));
	sigaction(SIGINT, &ignore_action, NULL);

	sigfillset(&(stop_action.sa_mask));
	sigaction(SIGTSTP, &stop_action, NULL);


	//shell loop
	while (proceed == true) {

		mainPid = getpid();

		//set boolean to true
		background = true;
		//clear all saved files to prevent reuse
		inputFile = NULL;
		outputFile = NULL;
		inputFilePresent = false;
		outputFilePresent = false;

		//boolean to check for output
		bool output = true;

		printf(": ");       //Prompt and flushing to avoid errors
		fflush(stdin);
		fflush(stdout);

		//get user output
		output = getInput(output);

		//if user entered a comment or nothing at all
		if (output == false || parsed[0] == NULL)
		{
			continue;
		}
		else
		{
			//built in exit option that clears memory and exits shell
			if (strcmp(parsed[0], "exit") == 0)
			{

				proceed = false;
				clearMem();

				exit(0);
			}
			//built in cd option that lets users navigate directories
			else if (strcmp(parsed[0], "cd") == 0)
			{
				//user entered directory name
				if (parsed[1] != NULL) 
				{
					chdir(parsed[1]);
				}
				else
				{
					chdir(getenv("HOME"));

				}
				
			}
			//if user entered in status
			else if (strcmp(parsed[0], "status") == 0)
			{
				status(pStatus);
			}

			//user did not enter in a built in command, fork a new process
			else
			{
				childPid = fork();

				//switch code material for child processes modified from lecture material
				switch (childPid) {

					//if fork was unsuccessful
				case -1:
					perror("Fork Unsuccessful");
					pStatus = 1;
					break;
				case 0:
					//user is entering background only mode for new process
					if (background == true) {

						//set sigint to default to allow for termination of only child process
						sigaction(SIGINT, &default_action, NULL);
					}
					//open input and output files
					getFiles();

					//if commands were not found, set exit error
					//execvp will proceed automatically
					execvp(parsed[0], parsed);

					//if no execvp, then error
					printf("No such file or directory");
					fflush(stdout);
					exit(1);
					break;


					//parent processes
				default:

					if (background == true) {
						waitpid(childPid, &pStatus, 0);
					}
					else {
						printf("Background pid is %i\n", childPid);
						fflush(stdout);
						break;
					}
				}

			}
		}
		//clear all memories and processes
		clearMem();
		processKiller(pStatus);

	}

	return 0;
}