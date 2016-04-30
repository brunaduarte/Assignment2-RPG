
/*
Copyright (C) 2014  Frank Duignan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "console.h"
#include "realm.h"
#include <stdio.h>
#include <time.h>
#include <windows.h> //Alows the use of SetConsoleTitle
// Find types: h(ealth),s(trength),m(agic),g(old),w(eapon)
const char FindTypes[]={'h','s','m','g','w'};

// The following arrays define the bad guys and 
// their battle properies - ordering matters!
// Baddie types : O(gre),T(roll),D(ragon),H(ag)
const char Baddies[]={'O','T','D','H'};
// The following is the weapons' damage
const byte WeaponDamage[]={2,8,10,12};
#define ICE_SPELL_COST 10
#define FIRE_SPELL_COST 10
#define LIGHTNING_SPELL_COST 10
const byte FreezeSpellDamage[]={10,20,5,0};
const byte FireSpellDamage[]={20,10,5,0};
const byte LightningSpellDamage[]={15,10,25,0};
const byte BadGuyDamage[]={10,10,15,5};
int GameStarted = 0;
tPlayer thePlayer;
tRealm theRealm;
int RealmLevel=0;
FILE *SaveFile;
void delay(int len);

unsigned prbs()
{
	// This is an unverified 31 bit PRBS generator
	// It should be maximum length but this has not been verified 
	static unsigned long shift_register=0xa5a5a5a5;
	static int Initialized=0;
	// This is a mechanism that should pick a different prbs starting 
	// value each run (so long as you don't run in the same second)
	if (!Initialized)
	{
		shift_register=(unsigned long)time(NULL);
		Initialized = 1;
	}


	unsigned long new_bit=0;
	static int busy=0; // need to prevent re-entrancy here
	if (!busy)
	{
		busy=1;
		new_bit= ((shift_register & (1<<27))>>27) ^ ((shift_register & (1<<30))>>30);
		new_bit= ~new_bit;
		new_bit = new_bit & 1;
		shift_register=shift_register << 1;
		shift_register=shift_register | (new_bit);
		busy=0;
	}
	return shift_register & 0x7ffffff; // return 31 LSB's 
}


unsigned range_random(unsigned range)
{
	return prbs() % (range+1);
}
void runGame(void)
{
	char ch;
	
	SetConsoleTitle("Forgotten Realms");
	
	//titleScreen();
	//printString("MicroRealms on the LPC810.");

    	system("cls"); 

	printString("=============================================");	
	printString("=           FORGOTTEN REALMS V1.2           =");
	printString("=============================================");	

	showHelp();		
	while(GameStarted == 0)
	{
		
		showGameMessage("Press S to start a new game \nPress L to load a previous game");
		ch = getUserInput();			
		
		if ( (ch == 'S') || (ch == 's') || (ch == 'L') || (ch == 'l') )
			GameStarted = 1;
	}
	
	if ( (ch == 'S') || (ch == 's') )
	{
		initRealm(&theRealm);	
		initPlayer(&thePlayer,&theRealm);
	}
	if( (ch == 'L') || (ch == 'l') )
	{
		LoadPlayer(&thePlayer);
		LoadRealm(&theRealm);
		eputs("Game Loaded!\n");
	}
	
	showPlayer(&thePlayer);
	eputs("\n");
	showRealm(&theRealm,&thePlayer);
	showGameMessage("Press H for help");
	
	while (1)
	{
		ch = getUserInput();
		ch = ch | 32; // enforce lower case
		switch (ch) {
			case '.' : {
				SavePlayer(&thePlayer);
				SaveRealm(&theRealm);
				showGameMessage("Game Saved!");
  				break;
			}
			case 'h' : {
				showHelp();
				break;
			}
			case 'n' : {
				showGameMessage("North");
				step('n',&thePlayer,&theRealm);
				break;
			}
			case 's' : {
				showGameMessage("South");
				step('s',&thePlayer,&theRealm);
				break;

			}
			case 'e' : {
				showGameMessage("East");
				step('e',&thePlayer,&theRealm);
				break;
			}
			case 'w' : {
				showGameMessage("West");
				step('w',&thePlayer,&theRealm);
				break;
			}
			case '#' : {		
				if (thePlayer.wealth)		
				{
					showRealm(&theRealm,&thePlayer);
					//thePlayer.wealth--; //no one likes this
				}
				else
					showGameMessage("No gold!");
				break;
			}
			case 'p' : {				
				showPlayer(&thePlayer);
				break;
			}
		} // end switch
	} // end while
}
void step(char Direction,tPlayer *Player,tRealm *Realm) //Player walking
{
	int new_x, new_y;
	new_x = Player->x;
	new_y = Player->y;
	byte AreaContents;
	switch (Direction) {
		case 'n' :
		{
			if (new_y > 0)
				new_y--;
			break;
		}
		case 's' :
		{
			if (new_y < 20+RealmLevel*4-1)
				new_y++;
			break;
		}
		case 'e' :
		{
			if (new_x <  20+RealmLevel*4-1)
				new_x++;
			break;
		}
		case 'w' :
		{
			if (new_x > 0)
				new_x--;
			break;
		}		
	}
	AreaContents = Realm->map[new_y][new_x];
	if ( AreaContents == '*')
	{
		showGameMessage("A rock blocks your path.");
		return;
	}
	Player->x = new_x;
	Player->y = new_y;
	int Consumed = 0;
	switch (AreaContents)
	{
		
		// const char Baddies[]={'O','T','B','H'};
		case 'O' :{
			showGameMessage("A smelly green Ogre appears before you");
			Consumed = doChallenge(Player,0);
			break;
		}
		case 'T' :{
			showGameMessage("An evil troll challenges you");
			Consumed = doChallenge(Player,1);
			break;
		}
		case 'D' :{
			showGameMessage("A smouldering Dragon blocks your way !");
			Consumed = doChallenge(Player,2);
			break;
		}
		case 'H' :{
			showGameMessage("A withered hag cackles at you wickedly");
			Consumed = doChallenge(Player,3);
			break;
		}
		case 'h' :{
			showGameMessage("You find an elixer of health");
			setHealth(Player,Player->health+10);
			Consumed = 1;		
			break;
			
		}
		case 's' :{
			showGameMessage("You find a potion of strength");
			Consumed = 1;
			setStrength(Player,Player->strength+1);
			break;
		}
		case 'g' :{
			showGameMessage("You find a shiny golden nugget");
			Player->wealth++;			
			Consumed = 1;
			break;
		}
		case 'm' :{
			showGameMessage("You find a magic charm");
			Player->mana++;						
			Consumed = 1;
			break;
		}
		case 'w' :{
			Consumed = addWeapon(Player,range_random(MAX_WEAPONS-1)+1);
			showPlayer(Player);
			break;			
		}
		case 'X' : {
			// Player landed on the exit
			printString("A door! You exit into a new realm");
			RealmLevel++;
			setHealth(Player,100); // maximize health
			initRealm(&theRealm);
			showRealm(&theRealm,Player);
		}
	}
	if (Consumed)
		Realm->map[new_y][new_x] = '.'; // remove any item that was found
}
int doChallenge(tPlayer *Player,int BadGuyIndex)
{
	char ch;
	int Damage;
	int BadGuyHealth = 100;
	printString("Press F to fight");
	ch = getUserInput() | 32; // get user input and force lower case
	if (ch == 'f')
	{
		printString("Choose action");
		while ( (Player->health > 0) && (BadGuyHealth > 0) )
		{
			// Player takes turn first
			if (Player->mana > ICE_SPELL_COST)
				printString("(I)CE spell");
			if (Player->mana > FIRE_SPELL_COST)
				printString("(F)ire spell");
			if (Player->mana > LIGHTNING_SPELL_COST)
				printString("(L)ightning spell");
			if (Player->Weapon1)
			{
				eputs("(1)Use ");
				printString(getWeaponName(Player->Weapon1));
			}	
			if (Player->Weapon2)
			{
				eputs("(2)Use ");
				printString(getWeaponName(Player->Weapon2));
			}
			printString("(P)unch");
			ch = getUserInput();
			switch (ch)
			{
				case 'i':
				case 'I':
				{
					printString("FREEZE!");
					Player->mana -= ICE_SPELL_COST;
					Damage = FreezeSpellDamage[BadGuyIndex]+range_random(10);
					BadGuyHealth -= Damage;
					eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
					zap();
					break;
				}
				case 'f':
				case 'F':
				{
					printString("BURN!");
					Player->mana -= FIRE_SPELL_COST;
					Damage = FireSpellDamage[BadGuyIndex]+range_random(10);
					BadGuyHealth -= Damage;
					eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
					zap();
					break;
				}
				case 'l':
				case 'L':
				{
					printString("ZAP!");
					Player->mana -= LIGHTNING_SPELL_COST;
					Damage = LightningSpellDamage[BadGuyIndex]+range_random(10);
					BadGuyHealth -= Damage;
					eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
					zap();
					break;
				}
				case '1':
				{
					Damage = WeaponDamage[Player->Weapon1]+range_random(Player->strength);
					printString("Take that!");
					BadGuyHealth -= Damage;
					eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
					setStrength(Player,Player->strength-1);
					break;
				}
				case '2':
				{
					Damage = WeaponDamage[Player->Weapon2]+range_random(Player->strength);
					printString("Take that!");
					BadGuyHealth -= Damage;
					eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
					setStrength(Player,Player->strength-1);
					break;
				}
				case 'p':
				case 'P':
				{
					Damage = WeaponDamage[Player->Weapon1]+range_random(Player->strength);
					printString("Thump!");
					BadGuyHealth -= Damage;
					eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
					setStrength(Player,Player->strength-1);
					break;
				}
				default: {
					printString("You fumble. Uh oh");
				}
			}

			// Bad guy then gets a go 			
			if (BadGuyHealth <= 0)
				BadGuyHealth = 0;
			else
			{
				eputs("\nMonster's turn!\n");
				Damage = BadGuyDamage[BadGuyIndex]+range_random(4);
				setHealth(Player,Player->health - Damage);
				eputs("you took "); printf("%d", Damage); eputs(" damage!\n\n");
				eputs("Health: you "); printf("%d", Player->health);
				eputs(", monster " );printf("%d", BadGuyHealth);
				eputs("\r\n");
			}
		}
		if (Player->health == 0)
		{ // You died
			printString("You are dead. Press Reset to restart");
			while(1);
		}
		else
		{ // You won!
			Player->wealth = 50 + range_random(50);			
			showGameMessage("You win! Their gold is yours");			
			return 1;
		}
		
	}
	else
	{
		showGameMessage("Our 'hero' chickens out");
		return 0;
	}
}
int addWeapon(tPlayer *Player, int Weapon)
{
	char c;
	eputs("You stumble upon ");
	switch (Weapon)
	{
		case 1:
		{	
			printString("a mighty axe");
			break;
		}
		case 2:
		{	
			printString("a sword with mystical runes");
			break;
		}
		case 3:
		{	
			printString("a bloody flail");
			break;
		}		
		default:
			printHex(Weapon);
	}
	if ( (Player->Weapon1) && (Player->Weapon2) )
	{
		// The player has two weapons already.
		showPlayer(Player);
		printString("You already have two weapons");		
		printString("(1) drop Weapon1, (2) for Weapon2, (0) skip");
		c = getUserInput();
		switch(c)
		{
			eputc(c);
			case '0':{
				return 0; // don't pick up
			}
			case '1':{
				Player->Weapon1 = Weapon;
				break;
			}
			case '2':{
				Player->Weapon2 = Weapon;
				break;
			}
		}
	}
	else
	{
		if (!Player->Weapon1)
		{
			Player->Weapon1 = Weapon;	
		}
		else if (!Player->Weapon2)
		{
			Player->Weapon2 = Weapon;
		}
	}	
	return 1;
}
const char *getWeaponName(int index)
{
	switch (index)
	{
		case 0:return "Empty"; break;
		case 1:return "Axe";break;
		case 2:return "Sword"; break;
		case 3:return "Flail"; break;
	}
}

void setHealth(tPlayer *Player, int health)
{
	if (health > 100)
		health = 100;
	if (health < 0)
		health = 0;
	Player->health = health;
	
}	
void setStrength(tPlayer *Player, int strength)
{
	if (strength > 100)
		strength = 100;
	if (strength < 0)
		strength = 0;
	Player->strength = strength;
}

void setDefense(tPlayer *Player, int defense)
{
	if (defense > 100)
		defense = 100;
	if (defense < 0)
		defense = 0;
	Player->defense = defense;
}

void setIntelligence(tPlayer *Player, int intelligence)
{
	if (intelligence > 100)
		intelligence = 100;
	if (intelligence < 0)
		intelligence = 0;
	Player->intelligence = intelligence;
}

void initPlayer(tPlayer *Player,tRealm *theRealm)
{
	// get the player name
	int index=0;
	byte x,y;
	char ch=0;
	// Initialize the player's attributes
	eputs("Enter the player's name: ");
	while ( (index < MAX_NAME_LEN) && (ch != '\n') && (ch != '\r'))
	{
		ch = getchar();//getUserInput();
		if ( ch > '0' ) // strip conrol characters
		{
			Player->name[index++]=ch;
			eputc(ch);
		}
	}
	Player->name[index]=0; // terminate the name
	setHealth(Player,100);
	Player->strength = 50+range_random(50);
	Player->mana = 50+range_random(50);
	Player->defense = 50+range_random(50);	
	Player->intelligence = 50+range_random(50);		
	Player->wealth=10+range_random(10);
	Player->Weapon1 = 0;
	Player->Weapon2 = 0;
	

	
	// Initialize the player's location
	// Make sure the player does not land
	// on an occupied space to begin with
	do {
		x=range_random(20+RealmLevel*4);
		y=range_random(20+RealmLevel*4);
		
	} while(theRealm->map[y][x] != '.');
	Player->x=x;
	Player->y=y;
}
void showPlayer(tPlayer *thePlayer)
{

	HANDLE out;
    out=GetStdHandle(STD_OUTPUT_HANDLE);
    COORD ptext={56,30},ctext={56,31},utext={56,32},utext1={56,33},utext2={56,34},utext3={56,35},utext4={56,36},utext5={56,37}, utext6={56,38}, userscoresz={55,29};
	printBorder(20,11,userscoresz,15); //15 is the background color, 20 is the length and 10 is the heigth
    SetConsoleCursorPosition(out,ptext);
	printf("Player Board! \n");
	SetConsoleCursorPosition(out,ctext);
    printf("Name: %s",thePlayer->name);
    SetConsoleCursorPosition(out,utext);
    printf("HP: %d",thePlayer->health);
    SetConsoleCursorPosition(out,utext1);
    printf("Str: %d",thePlayer->strength);
	SetConsoleCursorPosition(out,utext2);
    printf("Mana: %d",thePlayer->mana);
    SetConsoleCursorPosition(out,utext3);
    printf("Int: %d",thePlayer->intelligence);
    SetConsoleCursorPosition(out,utext4);
    printf("Wealth: %d",thePlayer->wealth);
	SetConsoleCursorPosition(out,utext5);
	printf("Weapon1: %s",thePlayer->Weapon1);
	SetConsoleCursorPosition(out,utext6);
	printf("Weapon2: %s",thePlayer->Weapon2);
			
}

void SavePlayer(tPlayer *thePlayer)
{
   	SaveFile = fopen("SavePlayer.txt", "w");

   	fprintf(SaveFile, "Name: %s\n", thePlayer->name);
   	fprintf(SaveFile, "Health: %d\n", thePlayer->health);
   	fprintf(SaveFile, "Strength: %d\n", thePlayer->strength);
   	fprintf(SaveFile, "Stamina: %d\n", thePlayer->stamina);
   	fprintf(SaveFile, "Mana: %d\n", thePlayer->mana);
	fprintf(SaveFile, "Defense: %d\n", thePlayer->defense);
	fprintf(SaveFile, "Intelligence: %d\n", thePlayer->intelligence);
   	fprintf(SaveFile, "Wealth: %d\n", thePlayer->wealth);
   	fprintf(SaveFile, "Location: %d, %d\n", thePlayer->x, thePlayer->y);
	fprintf(SaveFile, "Weapon1: %d\n", thePlayer->Weapon1);
	fprintf(SaveFile, "Weapon2: %d\n", thePlayer->Weapon2);

  	fclose(SaveFile);			
}

void SaveRealm(tRealm *theRealm)
{
	int x,y;
   	SaveFile = fopen("SaveRealm.txt", "w");
	for(y=0;y<20+RealmLevel*4;y++)
	{
		for(x=0;x<20+RealmLevel*4;x++)
			fprintf(SaveFile, "%d ", theRealm->map[y][x]);
		fprintf(SaveFile, "\n");
	}
  	fclose(SaveFile);			
}

void LoadRealm(tRealm *theRealm)
{
	int x,y;
   	SaveFile = fopen("SaveRealm.txt", "r");
	for(y=0;y<20+RealmLevel*4;y++)
	{
		for(x=0;x<20+RealmLevel*4;x++)
		{
			fscanf(SaveFile, "%d", &theRealm->map[y][x]);
			fseek(SaveFile , 1+ x/(20+RealmLevel*4-1) , SEEK_CUR ); //moves through the "space" char or "\n"
		}
	}
  	fclose(SaveFile);	
}

void LoadPlayer(tPlayer *thePlayer)
{
	char aux[20];
   	SaveFile = fopen("SavePlayer.txt", "r");

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
	fscanf(SaveFile, "%s", thePlayer->name);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->health);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->strength);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->stamina);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->mana);

	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->defense);

	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->intelligence);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->wealth);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->x);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->y);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->Weapon1);

	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->Weapon2);

	fclose(SaveFile);
}

void initRealm(tRealm *Realm)
{
	int x,y;
	int Rnd;
	
	
   	
	// clear the map to begin with
	for (y=0;y < 20+RealmLevel*4; y++)
	{
		for (x=0; x < 20+RealmLevel*4; x++)
		{
			Rnd = range_random(100);
			
			if (Rnd >= 98) // put in some baddies
				Realm->map[y][x]=	Baddies[range_random(sizeof(Baddies)-1)];
			else if (Rnd >= 95) // put in some good stuff
				Realm->map[y][x]=	FindTypes[range_random(sizeof(FindTypes)-1)];
			else if (Rnd >= 90) // put in some rocks
				Realm->map[y][x]='*'; 
			else // put in empty space
				Realm->map[y][x] ='.';
		}
	}
	
	// finally put the exit to the next level in
	x = range_random(20+RealmLevel*4);
	y = range_random(20+RealmLevel*4);
	Realm->map[y][x]='X';
}
void showRealm(tRealm *Realm,tPlayer *thePlayer)
{
	int x,y;
	printString("The Realm:");	
	for (y=0;y<20+RealmLevel*4;y++)
	{
		for (x=0;x<20+RealmLevel*4;x++)
		{
			
			if ( (x==thePlayer->x) && (y==thePlayer->y))
				eputc('@');
			else
				eputc(Realm->map[y][x]);
		}
		eputs("\r\n");
	}
	printString("\r\nLegend");
	printString("(T)roll, (O)gre, (D)ragon, (H)ag, e(X)it");
	printString("(w)eapon, (g)old), (m)agic, (s)trength");
	printString("@=You");
}
void showHelp()
{
	printString("N,S,E,W : go North, South, East, West");
	printString("# : show map");
	printString(". : save game");
	printString("(H)elp");
	printString("(P)layer details\n");
}

void showGameMessage(char *Msg)
{
	printString(Msg);
	printString("Ready");
}
char getUserInput()
{
	char ch = 0;	
	ch = getchar();
	// need to flush out the input buffer
	int c;
	while ( (c = getchar()) != '\n' && c != EOF ) { }		
	return ch;
}

void zap()
{
	// do some special effect when someone uses a spell
}

void titleScreen()         ///displays the title screen
{
	printf("Welcome to the Whispering Burrows!\n");

}

void printBorder(int _length,int _width,COORD _coordinates,int _color)     ///border printing function
{
    int i,j;
    COORD zerozero={0,0},bordersz;
    SMALL_RECT _rect;
    CHAR_INFO _border[_length*_width];
    HANDLE out;
    out=GetStdHandle(STD_OUTPUT_HANDLE);
    bordersz.X=_length;
    bordersz.Y=_width;
    _rect.Left=_coordinates.X;
    _rect.Top=_coordinates.Y;
    _rect.Right=_rect.Left+_length;
    _rect.Bottom=_rect.Top+_width;
    for(i=0;i<_width;i++)
    {
        for(j=0;j<_length;j++)
        {
            if(i==0 || i==_width-1)
            {
                _border[j+_length*i].Char.AsciiChar=205;
                _border[j+_length*i].Attributes=_color;
                continue;
            }
            if(j==0 || j==_length-1)
            {
                _border[j+_length*i].Char.AsciiChar=186;
                _border[j+_length*i].Attributes=_color;
                continue;
            }
            _border[j+_length*i].Char.AsciiChar=' ';
            _border[j+_length*i].Attributes=_color;
        }
    }
    _border[0].Char.AsciiChar=201;
    _border[_length-1].Char.AsciiChar=187;
    _border[_length*_width - 1].Char.AsciiChar=188;
    _border[_length*(_width-1)].Char.AsciiChar=200;
    _border[0].Attributes=_color;
    _border[_length-1].Attributes=_color;
    _border[_length*_width - 1].Attributes=_color;
    _border[_length*(_width-1)].Attributes=_color;
    WriteConsoleOutput(out,_border,bordersz,zerozero,&_rect);
}

