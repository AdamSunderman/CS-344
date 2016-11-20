//Written by Adam Sunderman for OSU CS-344
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//Define program specific values
#define DIR_PERMISSIONS 0755			
#define MAX_GAME_DIR_NAME_LEN 30
#define NUM_ROOM_NAMES 10
#define MIN_NUM_CONNECTS 3
#define MAX_NUM_CONNECTS 6
#define MAX_NUM_ROOMS 7

//Floor 'map' of filenames representing rooms
struct FLOOR{
	int map[MAX_NUM_ROOMS];
	char start[30];
	char end[30];
	char mid1[30];
	char mid2[30];
	char mid3[30];
	char mid4[30];
	char mid5[30];

};

//Function Prototypes
char * mkGameDir();
struct FLOOR makeGameFiles(char *, char **);
void connectRooms(char *, struct FLOOR, char **);
void run_game(struct FLOOR, char *);
void enterRoom(char * , char *, char *);

//*****  MAIN  ******
int main(){
	//Create an array of room names for selection in game
	char * ROOM_NAMES[NUM_ROOM_NAMES];
  int i;
	for(i=0;i<NUM_ROOM_NAMES;i++){
		ROOM_NAMES[i] = malloc(128);
		if(ROOM_NAMES[i]==NULL){
			printf("Malloc failed!\n");
			exit(1);
		}
	}
	strcpy(ROOM_NAMES[0],"Alderaan");
	strcpy(ROOM_NAMES[1],"Bespin");
	strcpy(ROOM_NAMES[2],"Dagobah");
	strcpy(ROOM_NAMES[3],"Dantooine");
	strcpy(ROOM_NAMES[4],"Endor");
	strcpy(ROOM_NAMES[5],"Hoth");
	strcpy(ROOM_NAMES[6],"Kashyyyk");
	strcpy(ROOM_NAMES[7],"Sullust");
	strcpy(ROOM_NAMES[8],"Tatooine");
	strcpy(ROOM_NAMES[9],"Yavin");
	
  printf("\nLoading Game Files...");
	char * game_dir = mkGameDir();
	struct FLOOR game_floor = makeGameFiles(game_dir,ROOM_NAMES);
	connectRooms(game_dir, game_floor,ROOM_NAMES);
	run_game(game_floor,game_dir);
  int j;
	for(j=0;j<NUM_ROOM_NAMES;j++){
		free(ROOM_NAMES[j]);
	}
  free(game_dir);
  return 0;
}


//FUNCTION: mkGameDir()
//    Makes the directory for the game files in the current path, names the new directory 
//    'sunderad.rooms.[pid]', where [pid] is the current process id.
//INPUT: NONE
//OUTPUT: Returns a pointer to the name of the new directory
char * mkGameDir(){

  char * game_dir = malloc(128);
  char * my_name = "sunderad.rooms.";
  strcpy(game_dir, my_name);

  snprintf(game_dir, MAX_GAME_DIR_NAME_LEN, "%s%d", my_name, getpid());

  int worked = mkdir(game_dir, DIR_PERMISSIONS); 
  if (worked == -1){
    printf("Could not make game directory!\n");
    exit(0);
  }
  else{
   	return game_dir; 
  }
}

//FUNCTION: makeGameFiles(char *)
//    Creates randomly named game files in the game directory and stores the file paths 
//    to a struct that holds all the rooms, while designating start, middle and end rooms.
//INTPUT: 1)Game directory 2)Array oof room names
//OUTPUT: 1)Struct FLOOR with game room file paths  
struct FLOOR makeGameFiles(char * game_dir_x,char ** room_names){

	//create a new floorand a list of 7 unique random indices from 0 to 9 representing rooms
	struct FLOOR new_floor;
	int rooms_used[MAX_NUM_ROOMS]={-1,-1,-1,-1,-1,-1,-1}; 
	int count = MAX_NUM_ROOMS;
	int found = 0;
	while (count != 0){
		srand(time(NULL));
		int rand_room = (rand() % 10);
    int i;
		for (i=0;i < MAX_NUM_ROOMS;i++){
			if (rooms_used[i]==rand_room){
				found=1;
			}
		} 
		if (found != 1){
			rooms_used[MAX_NUM_ROOMS - count]=rand_room;
			count--;
		}
		else{
			found=0;
		}		
  }

  //Create a buffer for building up filenames etc.
  char * my_file_buffer = malloc(128);
  if (NULL==my_file_buffer){
    perror("Malloc failed!\n");
    exit(1);
  }

  //Use the random indices made above to make room files from the room names array in main
  //while saving the name each room to Floor struct 'new_floor'
  int j;
  for (j = 0; j < MAX_NUM_ROOMS; j++){
   	//build up filepaths and save
   	int new_room = rooms_used[j];
    snprintf(my_file_buffer, 128, "%s/%s", game_dir_x, room_names[new_room]);
    switch (j){
      case 0:
      	strcpy(new_floor.start, room_names[new_room]);
      case 1:
      	strcpy(new_floor.mid1, room_names[new_room]);
      case 2:
      	strcpy(new_floor.mid2, room_names[new_room]);
      case 3:
				strcpy(new_floor.mid3, room_names[new_room]);
      case 4:
				strcpy(new_floor.mid4, room_names[new_room]);
      case 5:
				strcpy(new_floor.mid5, room_names[new_room]);
      case 6:
      	strcpy(new_floor.end, room_names[new_room]);
    }

    //Write room names to files
    FILE * my_output = fopen(my_file_buffer, "w");
    if (my_output == NULL){
      perror("Could Not Open File!\n");
      exit(1);
    }
    else{
      fprintf(my_output, "ROOM NAME: %s\n", room_names[new_room]); 
    }
    fclose(my_output);
  }
  free(my_file_buffer);

  //save rooms names to the Struct FLOOR
  int k;
  for (k = 0;k < MAX_NUM_ROOMS;k++){
   	new_floor.map[k]=rooms_used[k];
  }
  return new_floor;
}

//FUNCTION: connectRooms(struct FLOOR)
//   Connects all game files together randomly using the files made above while
//   writing the resulting connections to each file.
//INPUT: 1)Game directory 2)Struct FLOOR with file paths of game rooms 3)Array of room names 
//OUTPUT: NONE
void connectRooms(char * my_dir, struct FLOOR the_floor,char ** room_names){

	//Create a list of random connection numbers and a 
	//list that tracks how many connections have been used
  //this how many connections each room will have
	int connections[MAX_NUM_ROOMS];
  int c;
	for (c = 0;c < MAX_NUM_ROOMS;c++){
		connections[c] = rand() % 4 + 3;
	}

	//Create a buffer for working on the current file.
	//and a buffer for any other connecting files
  char * my_file_buffer = malloc(128);
  char * temp = malloc(128);
  if (NULL==my_file_buffer || NULL==temp){
    perror("Malloc failed!\n");
    exit(1);
  }

  //Loop through the rooms picking random connections for each
  int i;
  for(i =0;i < MAX_NUM_ROOMS;i++){
    int used_index=0;
    int used_connects[6]={-1,-1,-1,-1,-1,-1}; 
    int the_room=the_floor.map[i];
    int duplicate=0;
    snprintf(my_file_buffer, 128, "%s/%s", my_dir, room_names[the_room]);

    //Check file length to see if any connections have already been made
    FILE * check_len = fopen(my_file_buffer, "r");
    int previous=0;
    int li;
		do{
			li = fgetc(check_len);
			if(li == 58){
				previous++;
      }
		}while(li != EOF);
		fclose(check_len);
    int to_make=connections[i] - previous + 1;
    if(to_make <= 0){
        continue;
    }

    //Loop through the current room making random connections while 
    //there are still connections to make
    int con=previous-1;
    while(to_make > con ){

      //Pick random numbers without duplicates
      int rand_conn=the_floor.map[(rand() % 7)];
      int j;
      for(j=0;j < 6;j++){
        if (used_connects[j]==rand_conn){
          duplicate=1;
        }
      }
      if(duplicate==1){
        duplicate=0;
        continue;
      }

      //Open the connecting file and write the new connection to both files
      used_connects[used_index]=rand_conn;
      used_index++;
    	snprintf(temp, 128, "%s/%s", my_dir, room_names[rand_conn]);
    	if (strcmp(temp, my_file_buffer)==0){
    		continue;
    	}
    	else{
    		FILE * parent = fopen(my_file_buffer, "a");
    		FILE * child = fopen(temp, "r");
      	if (parent == NULL || child == NULL){
         	perror("Could Not Open File!\n");
         	exit(1);
      	}

        // Check the number of connections in the connecting file
      	int ch;
				int child_length=0;
				do{
					ch = fgetc(child);
					if(ch == 58){
						child_length++;
          }
				}while(ch != EOF);

				if(child_length > 6){
					continue;
				}
				fclose(child);
				FILE * child_new = fopen(temp, "a");
      	fprintf(parent, "CONNECTION %d: %s\n", con+1, room_names[rand_conn]);
      	con++;
				fprintf(child_new, "CONNECTION %d: %s\n", child_length, room_names[the_room]);
				fclose(parent);
				fclose(child_new);
    	}
    }  	
  }

  //All files have their connections now print their room types
  int z;
  for(z=0;z < MAX_NUM_ROOMS;z++){
    snprintf(temp, 128, "%s/%s", my_dir, room_names[the_floor.map[z]]);
    FILE * setTypes = fopen(temp, "a");
    if(z==0){
    	fprintf(setTypes, "ROOM TYPE: START_ROOM");
    }
    else if (z==6){
    	fprintf(setTypes, "ROOM TYPE: END_ROOM");
    }
    else{
    	fprintf(setTypes, "ROOM TYPE: MID_ROOM");
    }
    fclose(setTypes);
  }  	
  
	free(my_file_buffer);
	free(temp);
}

//FUNCTION: run_game(struct FLOOR, Char *)
//   Main game logic. Moves player through the game by 'entering' rooms while
//   keeping track of the rooms visited and the number of moves.
//INPUT: 1)struct FLOOR with file paths of game rooms 2)name of game directory
//OUTPUT: NONE
void run_game(struct FLOOR game, char * my_dir){
  //Track rooms visited
  char * moves[50];
  int i;
  for(i=0;i<50;i++){
    moves[i] = malloc(128);
    if(moves[i]==NULL){
      printf("Malloc failed!\n");
      exit(1);
    }
  }
  //Track steps made, begin room, finish room and room player wants to move to
  int steps=0;
  char * begin=malloc(128);
	char * finish=malloc(128);
  char * move_to=malloc(128);
  if (NULL==move_to || NULL==begin || NULL==finish){
    perror("Malloc failed!\n");
    exit(1);
  }
  strcpy(begin, game.start);
  strcpy(finish, game.end);

  //Actual game loop
  //continue to enter rooms until in the finishing room
	while(strcmp(begin,finish)!=0){
    strcpy(moves[steps],begin);
    steps++;
    enterRoom(begin, my_dir, move_to);
    strcpy(begin,move_to);
  }
  //Game over
  strcpy(begin,move_to);
  steps++;
  printf("\n\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n\n",steps-1);
  int j;
  int k;
  for(j=1;j < steps-1;j++){
    printf("%s\n", moves[j]);
  }
  printf("%s\n\n", begin);
  for(k=0;k < 50;k++){
    if(moves[k]==NULL){
      continue;
    }
    free(moves[k]);
  }
  free(begin);
  free(finish);
  free(move_to);
}

//FUNCTION: enterRoom(Char *, Char *, Char *)
//   'Enters' rooms that the player wants to move to by reading in the file,
//   showing the room name, possible connections and then finally asking the next move. 
//INPUT: 1)current room 2)game directory 3) Where the player wants to move
//OUTPUT: NONE
void enterRoom(char * r, char * d, char * m){
  //Place to store room connections
  char * connects[6];
  int l;
  for(l=0;l<6;l++){
    connects[l] = malloc(128);
    if(connects[l]==NULL){
      printf("Malloc failed!\n");
      exit(1);
    }
  }
  //Memory for chopping up strings, reading files and user input
	char * token;
	char * my_file_buffer = malloc(128);
	char * temp = malloc(128);
  if(my_file_buffer==NULL || temp==NULL){
      printf("Malloc failed!\n");
      exit(1);
  }
  int made_connects=0;
  char * choice= malloc(128);
  if (NULL==my_file_buffer || NULL==temp){
  	perror("Malloc failed!\n");
    exit(1);
  }
  //Read and cut up file data into usable strings starting with room name
  snprintf(my_file_buffer, 128, "%s/%s", d, r);
  FILE * readRoom = fopen(my_file_buffer, "r");
  fgets(temp,128,readRoom);
  token=strtok(temp,":\n");
  token=strtok(NULL,":\n");
  token=token+1;
  printf("\n\nCURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS: ",token);
  fgets(temp,128,readRoom);
  token=strtok(temp,":\n");
  token=strtok(NULL,":\n");
  token=token+1;
  strcpy(connects[made_connects],token);
  made_connects++;
  printf("%s",token);
  //Print the rest of the connections stopping at the last line
  while(fgets(temp,128,readRoom)){
    token=strtok(temp,":\n");
    token=strtok(NULL,":\n");
    token=token+1;
    if(strcmp(token,"START_ROOM")==0 || strcmp(token,"MID_ROOM")==0 || strcmp(token,"END_ROOM")==0){
      printf(".\n");
    }
    else{
      printf(", %s",token); 
      strcpy(connects[made_connects],token);
      made_connects++;
    } 
  }
  //Let the user choose where to go next
  int status=0;
  while(status==0){
    printf("WHERE TO? >");
    scanf("%s",choice);
    int i;
    for(i=0;i < made_connects;i++){
      if(strcmp(connects[i],choice)==0){
        status=1;
        break;
      }
    }
    if(status==1){
      strcpy(m,choice);
      break;
    }
    printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
  }
  int k;
  for(k=0;k<6;k++){
    if(connects[k]==NULL){
      printf("Null ptr\n");
      continue;
    }
    free(connects[k]);
  }

  fclose(readRoom);
	free(my_file_buffer);
	free(temp);
  free(choice);
}

