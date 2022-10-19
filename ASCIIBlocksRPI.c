#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 48
#define HEIGHT 20
#define MENU 10
#define LIHB 0xC3
#define LILB 0xB3
#define TIHB 0x5F
#define TILB 0x9B

int variableA;
int variableB;
bool solidity;
bool painting;
bool blockSolidStatus[256];
unsigned char textures[256];
unsigned char texturesName[32];
unsigned char playerX;
unsigned char playerY;
unsigned char playerSwapBlock;
unsigned char playerBlock;
unsigned char playerBlockName[32];
unsigned char playerUsername[32];
unsigned char playerCommand[32];
unsigned char level[WIDTH][HEIGHT];
unsigned char levelName[32];
unsigned char maximumDefinedBlock;
unsigned char stdinChar;
unsigned char *menuOptions[MENU];
FILE *filePointer;

void flushStdin(){
	stdinChar = 0;
	while(stdinChar != '\n' && stdinChar != EOF){
		stdinChar = getchar();
	}
	
	return;
}

void resetGame(){
	textures[0] = ' '; // Air
	textures[1] = '#'; // Log
	textures[2] = '%'; // Leaves
	textures[3] = '*'; // Bedrock
	textures[4] = '&'; // Grass
	textures[5] = '+'; // Stone
	textures[6] = '@'; // Wool
	textures[7] = '['; // Left Door
	textures[8] = ']'; // Right Door
	textures[9] = '='; // Entrance
	textures[10] = '_'; // Carpet
	textures[11] = '1'; // Portal 1
	textures[12] = '2'; // Portal 2
	textures[13] = '3'; // Portal 3
	textures[14] = '4'; // Portal 4
	textures[15] = '-'; // Path
	textures[16] = '~'; // Vines
	textures[17] = 'x'; // Barrier
	textures[254] = 'X'; // Player
	textures[255] = 'Y'; // Player 2
	
	for(variableA = 18; variableA <= 253; variableA++){
		textures[variableA] = '?';
	}
	for(variableA = 1; variableA <= 255; variableA++){
		blockSolidStatus[variableA] = (variableA >= 7 && variableA <= 16 || !variableA) ? false : true;
	}
	for(variableA = 0; variableA < HEIGHT; variableA++){
		for(variableB = 0; variableB < WIDTH; variableB++){
			level[variableB][variableA] = 0;
		}
	}
	
	playerX = 1;
	playerY = 1;
	playerBlock = 1;
	playerSwapBlock = 0;
	maximumDefinedBlock = 17;
	strcpy(playerUsername, "Max");
	strcpy(levelName, "Level");
	strcpy(texturesName, "ASCIIBlocks V5+");
	
	solidity = true;
	painting = false;
	
	return;
}

void loadLevel(){
	filePointer = fopen("LEVEL.AL2", "rb");
	
	if(filePointer){
		variableA = false;
		if(fgetc(filePointer) != LIHB) variableA = true;
		if(fgetc(filePointer) != LILB) variableA = true;
		if(fgetc(filePointer) != WIDTH) variableA = true;
		if(fgetc(filePointer) != HEIGHT) variableA = true;
		
		if(variableA){
			printf("The level LEVEL.AL2 is not a valid level file. Press the Enter key to continue playing.");
			fclose(filePointer);
			getchar();
			flushStdin();
			return;
		}
		
		resetGame();
		
		playerX = fgetc(filePointer);
		playerY = fgetc(filePointer);
		playerBlock = fgetc(filePointer);
		
		for(variableA = 0; variableA < 31; variableA++){
			playerUsername[variableA] = fgetc(filePointer);
		}
		for(variableA = 0; variableA < 31; variableA++){
			levelName[variableA] = fgetc(filePointer);
		}
		
		for(variableA = 0; variableA < HEIGHT; variableA++){
			for(variableB = 0; variableB < WIDTH; variableB++){
				level[variableB][variableA] = fgetc(filePointer);
			}
		}
		/* Read the level data from the file and store it into the game map */
		
		playerSwapBlock = level[playerX - 1][playerY - 1];
		
		printf("Loaded the current level with LEVEL.AL2. Press the Enter key to continue playing.");
	} else {
		printf("The level LEVEL.AL2 cannot be accessed or does not exist. Press the Enter key to continue playing.");
	}
	
	fclose(filePointer);
	getchar();
	flushStdin();
	
	return;
}

void saveLevel(){
	filePointer = fopen("LEVEL.AL2", "wb");
	fputc(LIHB, filePointer);
	fputc(LILB, filePointer);
	fputc(WIDTH, filePointer);
	fputc(HEIGHT, filePointer);
	fputc(playerX, filePointer);
	fputc(playerY, filePointer);
	fputc(playerBlock, filePointer);
	
	for(variableA = 0; variableA < 31; variableA++){
		fputc(playerUsername[variableA], filePointer);
	}
	for(variableA = 0; variableA < 31; variableA++){
		fputc(levelName[variableA], filePointer);
	}
	
	level[playerX - 1][playerY - 1] = playerSwapBlock;
	for(variableA = 0; variableA < HEIGHT; variableA++){
		for(variableB = 0; variableB < WIDTH; variableB++){
			fputc(level[variableB][variableA], filePointer);
		}
	}
	playerSwapBlock = level[playerX - 1][playerY - 1];
	fclose(filePointer);
	
	printf("Saved current level to LEVEL.AL2. Press the Enter key to continue playing.");
	getchar();
	flushStdin();
	
	return;
}

void loadTextures(){
	filePointer = fopen("TEXTURES.ABT", "rb");
	if(filePointer){
		variableA = false;
		if(fgetc(filePointer) != TIHB) variableA = true;
		if(fgetc(filePointer) != TILB) variableA = true;
		
		if(variableA){
			printf("The textures TEXTURES.ABT is not a valid textures file. Press the Enter key to continue playing.");
			fclose(filePointer);
			getchar();
			flushStdin();
			return;
		}
		
		maximumDefinedBlock = fgetc(filePointer);
		
		for(variableA = 0; variableA < 31; variableA++){
			texturesName[variableA] = fgetc(filePointer);
		}
		
		for(variableA = 0; variableA < 256; variableA++){
			textures[variableA] = fgetc(filePointer);
		}
		for(variableA = 0; variableA < 256; variableA++){
			blockSolidStatus[variableA] = fgetc(filePointer);
		}
		
		printf("Loaded the current textures with TEXTURES.ABT. Press the Enter key to continue playing.");
	} else {
		printf("The textures TEXTURES.ABT cannot be accessed or does not exist. Press the Enter key to continue playing.");
	}
	
	fclose(filePointer);
	getchar();
	flushStdin();
	return;
}

void saveTextures(){
	filePointer = fopen("TEXTURES.ABT", "wb");
	fputc(TIHB, filePointer);
    	fputc(TILB, filePointer);
	fputc(maximumDefinedBlock, filePointer);
	
	for(variableA = 0; variableA < 31; variableA++){
		fputc(texturesName[variableA], filePointer);
	}
	
	for(variableA = 0; variableA < 256; variableA++){
		fputc(textures[variableA], filePointer);
	}
	for(variableA = 0; variableA < 256; variableA++){
		fputc(blockSolidStatus[variableA], filePointer);
	}
	
	fclose(filePointer);
	
	printf("Saved current textures to TEXTURES.ABT. Press the Enter key to continue playing.");
	getchar();
	flushStdin();
	
	return;
}

void getCommand(){
	flushStdin();
	printf("Type \"help\" or \'h\' for the list of commands.\n");
	printf("> ");
	scanf("%31[^\n]s", &playerCommand);
	flushStdin();
	printf("\n");
	
	if(!strcmp(playerCommand, "help") || !strcmp(playerCommand, "h")){
		printf("Type the command without the square brackets or just type the shortcut in the square brackets.\n\n");
		
		printf("[h]elp - Display a help menu with all the listed commands.\n");
		printf("[s]et [b]lock - Set the selected block to be a certain block.\n");
		printf("[s]et [b]lock [t]exture - Set a block to a certain texture.\n");
		printf("[s]et [s]olid [s]tatus - Set a block to be either solid or non-solid.\n");
		printf("[s]et [s]kin - Set the skin of the player shown in-game.\n");
		printf("[s]et [u]sername - Set the username of the player shown in-game.\n");
		printf("[t]eleport - Teleport to the specified X and Y positions.\n");
		printf("[t]eleport [r]elative - Teleport to the specified X and Y positions relative to the player.\n");
		printf("[t]oggle [p]ainting - Toggle whether trying to place a block where a block already is will either remove the block or replace the block.\n");
		printf("[t]oggle [s]olidity - Toggle whether you can or cannot go through a solid block.\n");
		putchar('\n');
		
		printf("Press the Enter key to continue playing.");
		getchar();
		flushStdin();
		
		return;
	}
	
	if(!strcmp(playerCommand, "set block") || !strcmp(playerCommand, "sb")){
		printf("NOTE: This does not work like the \"[b]lock\" command in previous versions. You must enter a Block ID from 0-255.\n");
		printf("block id> ");
		scanf("%d", &variableA);
		flushStdin();
		
		if(variableA < 0 || variableA > 255){
			printf("Invalid Block ID. Press the Enter key to continue playing.");
			getchar();
			flushStdin();
		} else {
			playerBlock = (char) variableA;
		}
		
		return;
	}
	
	if(!strcmp(playerCommand, "set block texture") || !strcmp(playerCommand, "sbt")){
		printf("NOTE: Enter a Block ID from 0-255 to be textured. You can then use that block with the \"set block\" command.\n");
		printf("block id> ");
		scanf("%d", &variableA);
		flushStdin();
		
		if(variableA < 0 || variableA > 255){
			printf("Invalid Block ID. Press the Enter key to continue playing.");
			getchar();
			flushStdin();
		} else {
			printf("texture> ");
			textures[variableA] = getchar();
			flushStdin();
		}
		
		return;
	}
	
	if(!strcmp(playerCommand, "set solid status") || !strcmp(playerCommand, "sss")){
		printf("NOTE: Enter a Block ID from 0-255.\n");
		printf("block id> ");
		scanf("%d", &variableA);
		flushStdin();
		
		if(variableA < 0 || variableA > 255){
			printf("Invalid Block ID. Press the Enter key to continue playing.");
			getchar();
			flushStdin();
		} else {
			printf("The current solid status of Block ID %d is %s. Would you like to switch this block to a %s? (Y/?)", variableA, (blockSolidStatus[variableA]) ? "solid" : "non-solid", (!blockSolidStatus[variableA]) ? "solid" : "non-solid");
			if(getchar() == 'y'){
				blockSolidStatus[variableA] = !blockSolidStatus[variableA];
			}
			flushStdin();
		}
		
		return;
	}
	
	if(!strcmp(playerCommand, "set skin") || !strcmp(playerCommand, "ss")){
		printf("skin> ");
		textures[254] = getchar();
		flushStdin();
		
		return;
	}
	
	if(!strcmp(playerCommand, "set username") || !strcmp(playerCommand, "su")){
		printf("NOTE: Usernames can be a maximum of 31 characters.\n");
		printf("username> ");
		scanf("%31[^\n]s", &playerUsername);
		flushStdin();
		
		return;
	}
	
	if(!strcmp(playerCommand, "teleport") || !strcmp(playerCommand, "t")){
		printf("x position> ");
		scanf("%d", &variableA);
		flushStdin();
		printf("y position> ");
		scanf("%d", &variableB);
		flushStdin();
		
		if(variableA < 1 || variableA > WIDTH || variableB < 1 || variableB > HEIGHT) {
			printf("The X or Y position is invalid. Press the Enter key to continue playing.");
			getchar();
			flushStdin();
		} else {
			level[playerX - 1][playerY - 1] = playerSwapBlock;
			playerX = (char) variableA;
			playerY = (char) variableB;
			playerSwapBlock = level[playerX - 1][playerY - 1];
		}
		
		return;
	}
	
	if(!strcmp(playerCommand, "teleport relative") || !strcmp(playerCommand, "tr")){
		printf("x position> ");
		scanf("%d", &variableA);
		flushStdin();
		printf("y position> ");
		scanf("%d", &variableB);
		flushStdin();
		
		if(variableA + playerX < 1 || variableA + playerX > WIDTH || variableB + playerY < 1 || variableB + playerY > HEIGHT) {
			printf("The X or Y position adds up to an invalid value. Press the Enter key to continue playing.");
			getchar();
			flushStdin();
		} else {
			level[playerX - 1][playerY - 1] = playerSwapBlock;
			playerX += (char) variableA;
			playerY += (char) variableB;
			playerSwapBlock = level[playerX - 1][playerY - 1];
		}
		
		return;
	}
	
	if(!strcmp(playerCommand, "toggle painting") || !strcmp(playerCommand, "tp")){
		printf("Painting mode is %s. Would you like to %s it? (Y/?)", (painting) ? "enabled" : "disabled", (!painting) ? "enable" : "disable");
		if(getchar() == 'y'){
			painting = !painting;
		}
		flushStdin();
		
		return;
	}
	
	if(!strcmp(playerCommand, "toggle solidity") || !strcmp(playerCommand, "ts")){
		printf("Solidity is %s. Would you like to %s it? (Y/?)", (solidity) ? "enabled" : "disabled", (!solidity) ? "enable" : "disable");
		if(getchar() == 'y'){
			solidity = !solidity;
		}
		flushStdin();
		
		return;
	}
	
	if(!strcmp(playerCommand, "ab2themax") && playerX == 48 && playerY == 20){
		printf("ASCIIBlocks to the Max!");
		getchar();
		flushStdin();
		return;
	}
	
	printf("Invalid command. Press the Enter key to continue playing.");
	getchar();
	flushStdin();
	
	return;
}

void inventory(){
	flushStdin();
	system("clear");
	
	printf("Welcome to the ASCIIBlocks Inventory! Enter a Block ID from 0-255! The Block ID list is found below!\n\n");
	
	for(variableA = 0; variableA <= maximumDefinedBlock; variableA++){
		printf("%c - %d\n", textures[variableA], variableA);
	}
	
	printf("> ");
	scanf("%d", &variableA);
	flushStdin();
	
	if(variableA < 0 || variableA > 255){
		printf("Invalid Block ID. Press the Enter key to continue playing.");
		getchar();
		flushStdin();
	} else {
		playerBlock = (char) variableA;
	}
	
	return;
}

void menu(){
	variableA = 0;
	
	while(variableA != -1){
		system("clear");
		
		printf("Welcome to the ASCIIBlocks Menu! Use W and S to move your selection, and Space to select!\n\n");
		
		for(variableB = 0; variableB < MENU; variableB++){
			printf("%s%s\n", (abs(variableA) == variableB) ? "--> " : "    ", menuOptions[variableB]);
		}
		
		switch(getchar()){
			case 'w':
				if(variableA == 0){
					variableA = MENU - 1;
				} else {
					variableA--;
				}
				break;
			case 's':
				if(variableA == MENU - 1){
					variableA = 0;
				} else {
					variableA++;
				}
				break;
			
			case ' ':
				flushStdin();
				system("clear");
				
				switch(variableA){
					case 1:
						printf("Are you sure you want to reset the game? (Y/?)");
						if(getchar() == 'y') resetGame();
						flushStdin();
						break;
					case 2:
						printf("Skins are a single character that represents you on the game map.");
						printf("> ");
						textures[254] = getchar();
						flushStdin();
						break;
					case 3:
						printf("Usernames can be a maximum of 31 characters.\n");
						printf("> ");
						scanf("%31[^\n]s", &playerUsername);
						flushStdin();
						break;
					case 4:
						printf("Level Names can be a maximum of 31 characters.\n");
						printf("> ");
						scanf("%31[^\n]s", &levelName);
						flushStdin();
						break;
					case 5:
						loadLevel();
						break;
					case 6:
						saveLevel();
						break;
					case 7:
						loadTextures();
						break;
					case 8:
						saveTextures();
						break;
					case 9:
						printf("This feature is not currently implemented into the game. Press the Enter key to continue playing.");
						getchar();
						flushStdin();
						break;
					default:
						break;
				}
				variableA = -1;
				break;
		}
	}
	
	return;
}	

void blockName(){
	switch(playerBlock){
		case 0:
			strcpy(playerBlockName, "Air");
			break;
		case 1:
			strcpy(playerBlockName, "Log");
			break;
		case 2:
			strcpy(playerBlockName, "Leaves");
			break;
		case 3:
			strcpy(playerBlockName, "Bedrock");
			break;
		case 4:
			strcpy(playerBlockName, "Grass");
			break;
		case 5:
			strcpy(playerBlockName, "Stone");
			break;
		case 6:
			strcpy(playerBlockName, "Wool");
			break;
		case 7:
			strcpy(playerBlockName, "Left Door");
			break;
		case 8:
			strcpy(playerBlockName, "Right Door");
			break;
		case 9:
			strcpy(playerBlockName, "Entrance");
			break;
		case 10:
			strcpy(playerBlockName, "Carpet");
			break;
		case 11:
			strcpy(playerBlockName, "Portal 1");
			break;
		case 12:
			strcpy(playerBlockName, "Portal 2");
			break;
		case 13:
			strcpy(playerBlockName, "Portal 3");
			break;
		case 14:
			strcpy(playerBlockName, "Portal 4");
			break;
		case 15:
			strcpy(playerBlockName, "Path");
			break;
		case 16:
			strcpy(playerBlockName, "Vines");
			break;
		case 17:
			strcpy(playerBlockName, "Barrier");
			break;
		case 254:
			strcpy(playerBlockName, "Player");
			break;
		case 255:
			strcpy(playerBlockName, "Player 2");
			break;
		default:
			strcpy(playerBlockName, "Unnamed");
			break;
	}
	
	return;
}

int main(){
	menuOptions[0] = "Back";
	menuOptions[1] = "Reset Game";
	menuOptions[2] = "Change Skin";
	menuOptions[3] = "Change Username";
	menuOptions[4] = "Change Level Name";
	menuOptions[5] = "Load Level";
	menuOptions[6] = "Save Level";
	menuOptions[7] = "Load Texture Pack";
	menuOptions[8] = "Save Texture Pack";
	menuOptions[9] = "Convert ABL to AL2 Level";
	
	resetGame();
	
	while(true){
		system("clear");
		
		level[playerX - 1][playerY - 1] = 254;
		
		blockName();
		
		putchar('#');
		for(variableA = 0; variableA < WIDTH; variableA++){
			putchar('-');
		}
		putchar('#');
		printf("  Welcome to ASCIIBlocks Raspberry Pi Edition!\n");
		for(variableA = 0; variableA < HEIGHT; variableA++){
			putchar('|');
			for(variableB = 0; variableB < WIDTH; variableB++){
				putchar(textures[level[variableB][variableA]]);
			}
			putchar('|');
			switch(variableA){
				case 0:
					printf("  Version 5.0.1 by Hayden!");
					break;
				case 2:
					printf("  Use WASD to move,");
					break;
				case 3:
					printf("  IJKL to place and delete blocks,");
					break;
				case 4:
					printf("  E to open the inventory!");
					break;
				case 6:
					printf("  Use X to quit,");
					break;
				case 7:
					printf("  T to type commands,");
					break;
				case 8:
					printf("  Q to open the menu!");
					break;
				case 10:
					printf("  X: %hhu", playerX);
					break;
				case 11:
					printf("  Y: %hhu", playerY);
					break;
				case 12:
					printf("  Block: %s (%c)%s", playerBlockName, textures[playerBlock], (painting) ? " (Painting mode is enabled)" : "");
					break;
				case 13:
					printf("  Block ID: %hhu", playerBlock);
					break;
				case 14:
					printf("  Block Solid Status: %s%s", (blockSolidStatus[playerBlock]) ? "Solid" : "Non-solid", (!solidity) ? " (Solidity is disabled)" : "");
					break;
				case 15:
					printf("  Username: %s (%c)", playerUsername, textures[254]);
					break;
				case 16:
					printf("  Level Name: %s", levelName);
					break;
				case 17:
					printf("  Textures: %s", texturesName);
					break;
				default:
					break;
			}
			putchar('\n');
		}
		putchar('#');
		for(variableA = 0; variableA < WIDTH; variableA++){
			putchar('-');
		}
		putchar('#');
		putchar('\n');
		putchar('\n');
		
		switch(getchar()){
			case 'w':
				if(playerY > 1 && (!blockSolidStatus[level[playerX - 1][playerY - 1 - 1]] || !solidity)){
					level[playerX - 1][playerY - 1] = playerSwapBlock;
					playerY--;
					playerSwapBlock = level[playerX - 1][playerY - 1];
				}
				break;
			case 'a':
				if(playerX > 1 && (!blockSolidStatus[level[playerX - 1 - 1][playerY - 1]] || !solidity)){
					level[playerX - 1][playerY - 1] = playerSwapBlock;
					playerX--;
					playerSwapBlock = level[playerX - 1][playerY - 1];
				}
				break;
			case 's':
				if(playerY < HEIGHT && (!blockSolidStatus[level[playerX - 1][playerY - 1 + 1]] || !solidity)){
					level[playerX - 1][playerY - 1] = playerSwapBlock;
					playerY++;
					playerSwapBlock = level[playerX - 1][playerY - 1];
				}
				break;
			case 'd':
				if(playerX < WIDTH && (!blockSolidStatus[level[playerX - 1 + 1][playerY - 1]] || !solidity)){
					level[playerX - 1][playerY - 1] = playerSwapBlock;
					playerX++;
					playerSwapBlock = level[playerX - 1][playerY - 1];
				}
				break;
			
			case 'i':
				if(playerY > 1){
					level[playerX - 1][playerY - 1 - 1] = (level[playerX - 1][playerY - 1 - 1] && !painting) ? 0 : playerBlock;
				}
				break;
			case 'j':
				if(playerX > 1){
					level[playerX - 1 - 1][playerY - 1] = (level[playerX - 1 - 1][playerY - 1] && !painting) ? 0 : playerBlock;
				}
				break;
			case 'k':
				if(playerY < HEIGHT){
					level[playerX - 1][playerY - 1 + 1] = (level[playerX - 1][playerY - 1 + 1] && !painting) ? 0 : playerBlock;
				}
				break;
			case 'l':
				if(playerX < WIDTH){
					level[playerX - 1 + 1][playerY - 1] = (level[playerX - 1 + 1][playerY - 1] && !painting) ? 0 : playerBlock;
				}
				break;
				
			case 'e':
				inventory();
				break;
			case 'x':
				flushStdin();
				printf("Are you sure you want to quit? (Y/?)");
				if(getchar() == 'y') exit(0);
				flushStdin();
				break;
			case 't':
				getCommand();
				break;
			case 'q':
				menu();
				break;
			
			default:
				break;
		}
	}
}