/******************************************************************************************
*Name: Jennifer Assaf
*Date:11/30/2018
*
*Desrciption: This is the key generator file that takes in a size from the user and creates 
*a key file of selected length. It also adds a newline character to the end of the file.
*******************************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char * argv[])
{

int total;
//seed random number generator
srand(time(NULL));

//if not enough arguments are provided, exit file
if (argc <= 1)
{
printf("Not enough arguments provided\n");
exit(1);
}

else{
	
	//set total equal to input provided
	total=atoi(argv[1]);

	int i;
	for (i = 0 ; i < total; i ++)
	{
		//generate a new random number between 0 and 27
		int  letter= rand() % 27;
		//add ascii value to get to 'A'
		letter=letter+65;
		char  ascii=(char)letter;
		//if ascii value is equal to [, set this to ' '
		if (ascii== '[')
		{
			ascii=' ';
		}
		
		//print random letters
		printf("%c", ascii);
	}	
}
//add newline
printf("\n");
return 0;
}
