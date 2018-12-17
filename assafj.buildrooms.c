#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef int bool;
#define true 1
#define false 0


void setRooms();
void setRoomType();
bool IsGraphFull();
void AddRandomConnection();
struct Room* GetRandomRoom();
bool CanAddConnectionFrom(struct Room* x);
bool ConnectionAlreadyExists(struct Room* x, struct Room* y);
void ConnectRoom(struct Room* x, struct Room* y);
bool IsSameRoom(struct Room* x, struct Room* y);
char folder[50];
void createFiles(struct Room* rooms[],char folder[]);

//array of all room names that will be used.
char* roomNames[10] = { "Ballroom", "Gameroom", "Patio", "Garage", "Hall", "Kitchen", "Library", "Lounge", "Study", "Cellar" };

//array to store struct rooms created
struct Room* rooms[7];

//struct Room that stores all components to be used 
struct Room
{
	int id;
	char* name;
	int numOutboundConnections;
	struct Room* outboundConnections[6];
	int  roomType;
};


//function to create files using all 7 structs.
void createFiles(struct Room* rooms[], char folder[])
{
	//enter correct directory
	chdir(folder);
	int i;
	
	for (i = 0; i < 7; i++)
	{
		FILE* file = fopen(rooms[i]->name, "w");
		fprintf(file, "ROOM NAME: %s\n", rooms[i]->name);

		int j;
		for (j = 0; j < rooms[i]->numOutboundConnections; j++)
		{
			fprintf(file, "CONNECTION %d: %s\n", (j + 1), rooms[i]->outboundConnections[j]->name);
		}

			if (rooms[i]->roomType == 1)
			{
				fprintf(file, "ROOM TYPE: START_ROOM");
			}
			else if (rooms[i]->roomType == 3)
			{
				fprintf(file, "ROOM TYPE: END_ROOM");
			}
			else
			{
				fprintf(file, "ROOM TYPE: MID_ROOM");
			}

		fclose(file);
	}

}
//function to create rooms
void setRooms() {
	
	//array to save numbers used already to prevent reuse
	int usedNumbers[7] = { -1 ,-1,-1,-1,-1,-1,-1 };
	int currentTotal = 0;
	int i;

	for (i = 0; i < 7; i++)
	{	
		//variable to keep track if random number is usable or not 
		bool number = false;
		while ( number == false)
		{	
			//generate a random number
			int randomNumber = (rand() % 10);
			int j;
			for (j = 0; j < 7;)
			{
				//if number has already been used, number remains false
				if (usedNumbers[j] == randomNumber)
				{
					number = false;
					j = 10;

				}
				//if number has not been used yet
				else if (j == 6)
				{
					number = true;
					
					//create a new struct Room
					struct Room* newRoom=(struct Room*)malloc(sizeof(struct Room));
					newRoom->id = currentTotal;
					newRoom->name = roomNames[randomNumber];
					newRoom->numOutboundConnections = 0;
					rooms[currentTotal] = newRoom;
					
					//add number to used number array to prevent reuse
					usedNumbers[currentTotal] = randomNumber;
					currentTotal++;
					j = 8;

				}

				j++;
			}		
		}

	}

}

//function to set room types using integers
void setRoomType() {

	int i;
	//set all room types to 2 for mid room
	for (i = 0; i < 7; i++)
	{
		rooms[i]->roomType = 2;
	}

	//integers to set start and end rooms
	int startRoom;
	int endRoom;

	//generate random numbers for start and end rooms
	startRoom = (rand() % 7);
	endRoom = (rand() % 7);
	
	//if they are both equal to each other, regenerate a new end room
	while (endRoom == startRoom)
	{
		endRoom = (rand() % 7);
	}
		
	//save new values
	rooms[startRoom]->roomType = 1;
	rooms[endRoom]->roomType = 3;


}


// Function that checks whether or not all rooms are fully connected.
// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
 bool IsGraphFull()
 {
 	int i;
	for (i = 0; i < 7; i++)
	{
		if ((rooms[i] -> numOutboundConnections) < 3)
 			return false;
 	}
 	return true;
}


//function to add random connections to rooms. This code is modified from 2.2 program outline material
void AddRandomConnection()
{
	struct Room* A; 
	struct Room* B;
	bool canConnect = true;
	bool bValid = false;
	

	while (canConnect)
	{
				
		A = GetRandomRoom();

		if (CanAddConnectionFrom(A) == true)
			break;
	}

	do
	{
		B = GetRandomRoom();
	} while (CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);
		
	//connect both rooms
	ConnectRoom(A, B);  
	
}

//function to get a random room for connections
struct Room* GetRandomRoom()
{
	int randomRoom = -1;
	randomRoom = (rand() % 7);
	return (rooms[randomRoom]);
}

//function that checks whether or not a room has less than 6 connections. 
bool CanAddConnectionFrom(struct Room* x)
{
	if (x->numOutboundConnections < 6)
		return true;
	else
		return false;

}

//checks to see whether or not a room is already connected to another room.
bool ConnectionAlreadyExists(struct Room* x, struct Room* y)
{
	int i;
	for (i = 0; i < (x->numOutboundConnections); i++)
	{
		//if this connection already exists, return true;
		if (x->outboundConnections[i] == y)
			return true;

	}
	return false;
}

//function to connect two rooms.
void ConnectRoom(struct Room* x, struct Room* y)
{
	//add room connections
	x->outboundConnections[x->numOutboundConnections] = y;
	y->outboundConnections[y->numOutboundConnections] = x;

	//increment total number of connections for each room
	x->numOutboundConnections++;
	y->numOutboundConnections++;
}


//function that checks if room name of two rooms is identical
bool IsSameRoom(struct Room* x, struct Room* y)
{
	if (x->name == y->name)
		return true;
	else
		return false;

}

int main(int argc, char* argv[]) {


	srand(time(NULL));
	setRooms();
	setRoomType();
	sprintf(folder, "assafj.rooms.%d", getpid());
	int result = mkdir(folder, 0755);		
	while (IsGraphFull() == false)
	{
		AddRandomConnection();
	}
	createFiles(rooms,folder);


	int j = 0;
	for (j=0; j<7; j++)
	{
	free (rooms[j]);
	}
	return 0;
}
