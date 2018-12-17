#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h> 

//typedef declaration to allow for bool usage
typedef int bool;
#define true 1
#define false 0


void findCurDirect(char* directoryName);
void storeRooms(char* directory);
void printConnections(char*,char*);
void* getTime();
int getRoomNumbers(char* value);
char* printRooms(int value);
char* getStart();
void printRoomConnections(int roomNum);
bool checkRoomConnections(int location, int roomNum);
int playGame();
void *getTime();

//recreate struct room variable, but use an array of ints for connections
struct Room
{
	int id;
	char* name;
	int numOutboundConnections;
	int outboundConnections[6];
	int roomType;
};

//array to hold all struct variables
struct Room* rooms[7];
//array to hold steps in pathway
int pathway[100];

//initialize a mutex named lock
pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER;

//function to find current directory of files. This code has been modified from lecture material.
void findCurDirect(char* directoryName)
{
	int newestDirTime = -1; 
	char targetDirPrefix[32] = "assafj.rooms."; // file with name of rooms
	char newestDirName[256]; // Holds the name of the newest dir that contains prefix
	memset(newestDirName, '\0', sizeof(newestDirName));

	DIR* dirToCheck; // Holds the directory we're starting in
	struct dirent *fileInDir; // Holds the current subdir of the starting dir
	struct stat dirAttributes; // Holds information we've gained about subdir

	dirToCheck = opendir("."); 

	if (dirToCheck > 0) 
	{
		while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
		{
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) 
			{
				stat(fileInDir->d_name, &dirAttributes); 

				if ((int)dirAttributes.st_mtime > newestDirTime) 
				{
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}

	closedir(dirToCheck); 
strcpy(directoryName, newestDirName);

}

//function used to recreate a new array using information from the files.
//Code on how to read individual line segments from a file modified from stack overflow.
void  storeRooms(char* directory){
	
	//directory informtion
	DIR* dirToCheck;
	struct dirent *fileInDir; 

	//file that we are currently visting
	FILE* currFile;
	
	//variables used to hold current data
	char roomName[100];
	char data[300];
	char temp[300];
	char line[30];
	int total=0;
	//open directory sent to this function
	dirToCheck=opendir(directory);
	if (dirToCheck ==NULL)
	{
	printf("error");
	exit(1);
	}	

	// Check each entry in directory, skipping over "." and ".."
	while ((fileInDir = readdir(dirToCheck)) != NULL) 
    	{	if (!strcmp (fileInDir->d_name, "."))
            	continue;
       		if (!strcmp (fileInDir->d_name, ".."))
            	continue;
        	
		//create a new struct Room called newRoom	
		struct Room* newRoom = (struct Room*)malloc(sizeof(struct Room));
		
		//set newRoom to name of directory.
		newRoom->name = fileInDir->d_name;
		//save directory and file name to correct room name. This allows
		//the file to be created in the correct location in the directory.
		sprintf(roomName,"./%s/%s",directory,fileInDir->d_name);
		currFile=fopen(roomName,"r");
	
		//number of connections to be found.
		int numConnections=0;
		
		//while we are not at the end of the file
		while (feof(currFile) == 0) 
		{	
			//get the current line in a file
			fgets(data,300, currFile);
			//if line has "CONNECTION" in it.
			if (strstr(data, "CONNECTION")!=NULL)
			 {
				//save data into a temporary variable 
				sscanf(data, "%*s %*s %s", temp);
				//get room number from string temp
				int roomNum=getRoomNumbers(temp);
				//save room number to array in struct room newRoom
				newRoom->outboundConnections[numConnections]=roomNum;
				newRoom->numOutboundConnections++;
				numConnections++;
					
			}
			//if lne has room type in it
			else if(strstr(data, "ROOM TYPE")!= NULL)
			{
				//save data to a temp variable
				sscanf(data, "%*s %*s %s", temp);
				//compare strings to midroom, if yes, set to type 2
				if (strncmp ("MID_ROOM", temp,5)==0)
				newRoom->roomType=2;
				//if string contains start, set to 1
				else if  (strncmp ("START",temp,5)==0)
				newRoom->roomType=1;
				//if neither, set to 3, this must be the end room.
				else
				newRoom->roomType=3;
			}
		
		}
	//save to room array
	//increment total number of rooms in array
	rooms[total]=newRoom;
	total++;	
	
	}


}

//function used to get corresponding numbers to rooms.
//a char* value is sent, and is compared to existing rooms.
int getRoomNumbers(char* value){

	if (strcmp(value,"Ballroom")==0)
		return 1;
	else if (strcmp(value,"Gameroom")==0)
		return 2;
	else if (strcmp(value,"Patio")==0)
		return 3;
	else if (strcmp(value,"Garage")==0)
		return 4;
	else if (strcmp(value,"Hall")==0)
		return 5;
	else if (strcmp(value,"Kitchen")==0)
		return 6;
	else if (strcmp(value,"Library")==0)
		return 7;
	else if (strcmp(value,"Lounge")==0)
		return 8;
	else if (strcmp(value,"Study")==0)
		return 9;
	else if (strcmp(value,"Cellar")==0)
		return 10;
	//if no rooms, match return 11.
	else
		return 11;	

}

//function used to return names of rooms, from int values set in getRoomNumbers()
char* printRooms(int value)
{

	if (value==1)
		return "Ballroom";
	else if (value==2)
		return "Gameroom";
	else if (value==3)
		return "Patio";
	else if (value==4)
		return "Garage";
	else if (value==5)
		return "Hall";
	else if (value==6)
		return "Kitchen";
	else if (value==7)
		return "Library";
	else if (value==8)
		return "Lounge";
	else if (value==9)
		return "Study";
	else 
		return "Cellar" ;

}

//function to get the first room that users will start in.
char* getStart()
{
	int i =0;
	for (i=0; i < 7; i++)
	{	
		//if roomType int is 1, then this is the start room.
		if (rooms[i]->roomType==1)
		{	
		char*startRoom=rooms[i]->name;
		return startRoom;
		}
	}	
}

//function to print connections to users using roomNum in array
void printRoomConnections(int roomNum)
{
	printf("\nCURRENT LOCATION: %s\n",rooms[roomNum]->name);
	int i;
	printf("POSSIBLE LOCATIONS: ");

	for (i=0; i < rooms[roomNum]->numOutboundConnections; i++)
	{
		int roomConnect=rooms[roomNum]->outboundConnections[i];
		char* roomName=printRooms(roomConnect);
			if(i<(rooms[roomNum]->numOutboundConnections)-1)
			printf("%s, ", roomName);
			else
			printf("%s.\n",roomName);
	}
}


//function to check if connection is a valid connection to the current room. 
bool checkRoomConnections(int location, int roomNum)
{
	int i;
	for(i=0; i < rooms[location]->numOutboundConnections; i++)
	{
		if (roomNum==(rooms[location]->outboundConnections[i]))
		return true; 
	}
	return false;
}

//Main game play function. Gets first room and allows users to navigate through until and end
//room is reached. Also allows a thread to start to get local time of a user.
int playGame()
{	
	int steps=0;
	//variable to get starting location
	char* location=getStart();
	//current room name
	char roomName[30];
	bool end=false;	
	
	//while end is not reached, continue in this loop.
	while(end!=true)
	{
		//variable to keep track of current location		
		int currLoc;
		//variable to keep track of whether or not input is a valid room connection
		bool validRoom=false;
		int i;
		int roomNum;
		
		//get current location in rooms array.
		for (i=0; i < 7; i++)
		{
			if(strcmp(location,rooms[i]->name)==0)
			currLoc=i;
		}		
	
		//check if this is the ending room
		if ((rooms[currLoc]->roomType)==3)
		{
			end=true;
			break;
		}
		
		//if not ending room, print current room connections.
		printRoomConnections(currLoc);
	
		//while loop that continues if room entered is not a real connection.
		while(validRoom!=true)
			{
				printf("WHERE TO? >");
				scanf("%s", roomName);
				
				//if user enters time as input. Mutex code was modified from Piazza and 
				//lecture materials on Concurrency
				if(strcmp(roomName,"time")==0)
				{	
					//create a new thread using getTime function
					pthread_t thread;
					pthread_create(&thread,NULL,getTime,NULL);	
					
					//unlock mutex 
					pthread_mutex_unlock(&lock);
					//join threads
					pthread_join(thread, NULL);
					//relock mutex to enable its reuse later if called again.
					pthread_mutex_lock(&lock);
				}
				else
				{	
					//get corresponding room number for name entered
					roomNum=getRoomNumbers(roomName); 
					//save pathway
					pathway[steps]=roomNum;
					//check room validity
					validRoom = checkRoomConnections(currLoc,roomNum);
				
					if (validRoom==false)
						printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
					printRoomConnections(currLoc);

				}
			}
		//save new location and increment current number of steps used.
		location=roomName;
		steps++;		
	}

return steps;
}
//Void pointer function used to get local time and save to txt file.
//Source code modified from C++ reference guide on tm* time and stack overflow.
void *getTime(){ 

	//lock mutex, so this will proceed first.	
	pthread_mutex_lock(&lock);

	//open file
	FILE* timeFile;
	timeFile=fopen("currentTime.txt","w");

	//set time to local time
	time_t compTime=time(NULL);
	struct tm* myTime=localtime(&compTime);
	char fTime[100];

	//format time to proper format
	strftime(fTime, 200, "%l:%M%p, %A, %B %d, %Y", myTime);
	printf("\n%s\n\n",fTime);
	//save time to timeFile 
	fprintf(timeFile,"%s",fTime);
	fclose(timeFile);
	
	//unlock mutex
	pthread_mutex_unlock(&lock);

	return 0;
}

int main(){
	//lock mutex first to allow it to be woken up later 
	pthread_mutex_lock(&lock);
	
	//directory name where all files will be stored.
	char* directoryName = malloc(sizeof(char) * 100 );
	
	//get current directory
	findCurDirect(directoryName);
	
	//get all room information and generate them again.
	storeRooms(directoryName);
	
	//play the game and save number of steps
	int steps=playGame();	

	printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
	
	//print all steps taken to reach end of file.
	int i;
	for(i=0; i < steps; i++)
	{
		char* roomName=printRooms(pathway[i]);
		printf("%s\n",roomName);
	}

	//free dynamic memory used in file
	free (directoryName);

	int j = 0;
	for (j = 0; j < 7; j++)
	{
		free(rooms[j]);
	}	

	return 0;


}
