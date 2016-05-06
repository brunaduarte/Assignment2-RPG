
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
#include <windows.h> //Allows the use of SetConsoleTitle
// Find types: h(ealth),s(trength),m(agic),g(old),w(eapon)
const char FindTypes[]={'h','m','g'};
const char RareTypes[]={'s','w'};

// The following arrays define the bad guys and 
// their battle properies - ordering matters!
// Baddie types : W(olf),O(gre),T(roll),D(ragon),H(ag)
const char Baddies[]={'W','O','T','D','H'};
// The following is the weapons' damage
const byte WeaponDamage[]={3,8,11,14}; //stamina cost: 1/1/2/3
#define ICE_SPELL_COST 15
#define FIRE_SPELL_COST 15
#define LIGHTNING_SPELL_COST 15
const byte FreezeSpellDamage[]={12,12,18,12,5};
const byte FireSpellDamage[]={12,18,12,12,5};
const byte LightningSpellDamage[]={12,12,12,18,5};
const byte BadGuyDamage[]={13,18,20,26,15};
const byte BadGuyExperience[]={30,50,55,70,40};
const byte BadGuyLife[]={75,100,90,110,80};

//base bonus for each class: mage/paladin/cavalier
//mage: +4int, -1str, -1def
//paladin: +2str, +1def
//cavalier: +15hp, +3str, -1int
const byte Classhealth[]={0,0,15};
const byte Classstrength[]={-1,2,3};
const byte Classint[]={4,0,-1};
const byte Classdefense[]={-1,1,0};

//levelup bonus for each class: mage/paladin/cavalier
//mage: +2int
//paladin: +1str, +1def
//cavalier: +3hp, +1str
const byte Bonushealth[]={0,0,3};
const byte Bonusstrength[]={0,1,1};
const byte Bonusint[]={2,0,0};
const byte Bonusdefense[]={0,1,0};

int GameStarted = 0;
tPlayer thePlayer;
tRealm theRealm;
int RealmLevel=0;
char ch;
FILE *SaveFile;
void delay(int len);
int flag;

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
	SetConsoleTitle("Forgotten Realms");
	HANDLE out=GetStdHandle(STD_OUTPUT_HANDLE);

    	system("cls"); 

	printString("=============================================");	
	printString("=           FORGOTTEN REALMS V1.2           =");
	printString("=============================================\n");	
    printString("A truly epic RPG-roguelike game");
	printString("(Please play full-screen!)\n");
	showHelp();		
	while(GameStarted == 0)
	{
		
		showGameMessage("\nPress S to start a new game \nPress L to load a previous game");
		ch = getUserInput();			
		
		if ( (ch == 'S') || (ch == 's') || (ch == 'L') || (ch == 'l') )
			GameStarted = 1;
	}
	
	if ( (ch == 'S') || (ch == 's') )
	{
		initRealm(&theRealm, RealmLevel);	
		initPlayer(&thePlayer,&theRealm);
	}
	if( (ch == 'L') || (ch == 'l') )
	{
		LoadPlayer(&thePlayer);
		LoadRealm(&theRealm);
		eputs("Game Loaded!\n");
	}
	delay(200);
	system("cls");
	showRealm(&theRealm,&thePlayer);
	showPlayer(&thePlayer);
	COORD position={0,33+4*RealmLevel};
	SetConsoleCursorPosition(out,position);
	//eputs("\n");

	while (1)
	{
		flag=1; //clean screen
		//ch = getUserInput();
		ch = getch();
		ch = ch | 32; // enforce lower case
		if(ch=='r'||ch=='R')
		{
			eputs("Are you sure? (Y/N)\n");
			char option=getUserInput();
			if(option=='Y'||option=='y')
			{
				GameStarted=0;
				return;
			}
		}
		else
		switch (ch) {
			case '.' : {
				SavePlayer(&thePlayer);
				SaveRealm(&theRealm);
				showGameMessage("Game Saved!");
				flag=0;
  				break;
			}
			case 'h' : {
				showHelp();
				flag=0;
				break;
			}
			case 'w' : {
				//showGameMessage("North");
				step('n',&thePlayer,&theRealm);
				break;
			}
			case 's' : {
				//showGameMessage("South");
				step('s',&thePlayer,&theRealm);
				break;

			}
			case 'd' : {
				//showGameMessage("East");
				step('e',&thePlayer,&theRealm);
				break;
			}
			case 'a' : {
				//showGameMessage("West");
				step('w',&thePlayer,&theRealm);
				break;
			}
			case 'e' : {				
				rest(&thePlayer);
				flag=0;
				break;
			}
		} // end switch
		showPlayer(&thePlayer);
		HANDLE out;
    	out=GetStdHandle(STD_OUTPUT_HANDLE);
    	COORD position={0,0}, position2={0,33+4*RealmLevel};
		SetConsoleCursorPosition(out,position);
		//system("cls");
		showRealm(&theRealm,&thePlayer);
	if (flag)
	{
		printf("                                                                                                    "); //clean line
		SetConsoleCursorPosition(out,position2);
	}
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
		flag=0;
		return;
	}
	Player->x = new_x;
	Player->y = new_y;
	int Consumed = 0;
	switch (AreaContents)
	{
		
		// const char Baddies[]={'W','O','T','B','H'};
		case 'W' :{
			showGameMessage("A giant Wolf tries to eat you!");
			Consumed = doChallenge(Player,0);
			system("pause");
			system("cls");
			break;
		}
		case 'O' :{
			showGameMessage("A smelly green Ogre appears before you!");
			Consumed = doChallenge(Player,1);
			system("pause");
			system("cls");
			break;
		}
		case 'T' :{
			showGameMessage("An evil Troll challenges you!");
			Consumed = doChallenge(Player,2);
			system("pause");
			system("cls");
			break;
		}
		case 'D' :{
			showGameMessage("A Red Dragon blocks your way!");
			Consumed = doChallenge(Player,3);
			system("pause");
			system("cls");
			break;
		}
		case 'H' :{
			showGameMessage("A withered hag cackles at you wickedly!");
			Consumed = doChallenge(Player,4);
			system("pause");
			system("cls");
			break;
		}
		case 'h' :{
			showGameMessage("You find an elixer of health! hp+30!");
			setHealth(Player,Player->health+30);
			Consumed = 1;		
			flag=0;
			break;
			
		}
		case 's' :{
			showGameMessage("You find a potion of strength! str+1!");
			Consumed = 1;
			Player->strength++;
			flag=0;
			break;
		}
		case 'g' :{
			showGameMessage("You find a shiny golden nugget!");
			Player->wealth+=range_random(10);			
			Consumed = 1;
			flag=0;
			break;
		}
		case 'm' :{
			showGameMessage("You find a magic charm! mana +20!");
			setMana(Player,Player->mana+20);						
			Consumed = 1;
			flag=0;
			break;
		}
		case 'w' :{
			Consumed = addWeapon(Player,range_random(MAX_WEAPONS-2)+1); //weapon 0 = punch (no weapon), so weapons go from 1 to 3
			flag=0;
			break;			
		}
		case 'X' : {
			// Player landed on the exit
			printString("A door! You exit into a new realm");
			RealmLevel++;
			setHealth(Player,Player->Maxhealth); // maximize health
			initRealm(&theRealm, RealmLevel);
			system("pause");
			system("cls");
		}
	}
	if (Consumed)
		Realm->map[new_y][new_x] = '.'; // remove any item that was found
}
int doChallenge(tPlayer *Player,int BadGuyIndex)
{
	int aux;
	char ch;
	int Damage;
	int BadGuyHealth = BadGuyLife[BadGuyIndex];
	printString("Press F to fight");
	ch = getUserInput() | 32; // get user input and force lower case
	//ch = getch() | 32;
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
			//ch = getch();
			switch (ch)
			{
				case 'i':
				case 'I':
				{
					if (Player->mana > ICE_SPELL_COST){
						printString("FREEZE!");
						Player->mana -= ICE_SPELL_COST;
						Damage = FreezeSpellDamage[BadGuyIndex]+Player->intelligence+range_random((Player->intelligence)/2); //int 12: dmg +6to12
						BadGuyHealth -= Damage;
						eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
						zap();
					}
					else
						eputs("You don't have sufficient mana!");
					break;
				}
				case 'f':
				case 'F':
				{
					if (Player->mana > FIRE_SPELL_COST){
						printString("BURN!");
						Player->mana -= FIRE_SPELL_COST;
						Damage = FireSpellDamage[BadGuyIndex]+Player->intelligence+range_random((Player->intelligence)/2); //int 12: dmg +6to12
						BadGuyHealth -= Damage;
						eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
						zap();
					}
					else
						eputs("You don't have sufficient mana!");
					break;
				}
				case 'l':
				case 'L':
				{
					if (Player->mana > LIGHTNING_SPELL_COST){
						printString("ZAP!");
						Player->mana -= LIGHTNING_SPELL_COST;
						Damage = LightningSpellDamage[BadGuyIndex]+Player->intelligence+range_random((Player->intelligence)/2); //int 12: dmg +6to12
						BadGuyHealth -= Damage;
						eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
						zap();
					}
					else
						eputs("You don't have sufficient mana!");
					break;
				}
				case '1':
				{
					if (Player->stamina >= Player->Weapon1){
						Damage = WeaponDamage[Player->Weapon1]+Player->strength/2+range_random((Player->strength)/2);  //str 12: dmg +6to12
						printString("Take that!");
						BadGuyHealth -= Damage;
						eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
						Player->stamina-=Player->Weapon1;
					}
					else{
						eputs("You are too tired!");
						Player->stamina++;
					}
					break;
				}
				case '2':
				{
					if (Player->stamina >= Player->Weapon2){
						Damage = WeaponDamage[Player->Weapon2]+Player->strength/2+range_random((Player->strength)/2);  //str 12: dmg +6to12
						printString("Take that!");
						BadGuyHealth -= Damage;
						eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
						Player->stamina-=Player->Weapon2;
					}
					else{
						eputs("You are too tired!");
						Player->stamina++;
					}
					break;
				}
				case 'p':
				case 'P':
				{
					if (Player->stamina >= 2){
						Damage = WeaponDamage[Player->Weapon1]+Player->strength/2+range_random((Player->strength)/2);  //str 12: dmg +6to12
						printString("Thump!");
						BadGuyHealth -= Damage;
						eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
						Player->stamina--;
					}
					else{
						eputs("You are too tired!");
						Player->stamina++;
					}
					break;
				}
				default: {
					printString("You fumble. Uh oh");
					setStamina(Player, Player->stamina+1);
				}
			}

			// Bad guy then gets a go 			
			if (BadGuyHealth <= 0)
				BadGuyHealth = 0;
			else
			{
				eputs("\nMonster's turn!\n");
				Damage = BadGuyDamage[BadGuyIndex]+range_random(5)-Player->defense;
				setHealth(Player,Player->health - Damage);
				eputs("you took "); printf("%d", Damage); eputs(" damage!\n\n");
				eputs("Health: you "); printf("%d", Player->health);
				eputs(", monster " );printf("%d", BadGuyHealth);
				eputs("\r\n");
			}
		}
		if (Player->health == 0)
		{ // You died
				printString("YOU DIED! Press CTRL+C to leave the game, loser.");
			while(1);
		}
		else
		{ // You won!
			aux = 10 + range_random(10);
			Player->wealth += aux;		
			eputs("You win! Their gold and experience are yours\n");
			printf("Gained %d gold pieces!\n", aux);
			setExperience(Player, BadGuyIndex);			
			return 1;
		}
		
	}
	else
	{
		showGameMessage("Our 'hero' chickens out");
		if(range_random(100)>75) //25% chance of being attacked before leaving battle
		{
			aux = range_random(8)+2*Player->level;
			setHealth(Player, (Player->health)-aux);
			printf("The monster attacked you, -%d hp\n", aux);
			if (Player->health == 0)
			{ // You died
				printString("YOU DIED! Press CTRL+C to leave the game, loser.");
				while(1);
			}
		}
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
			printString("a steel sword (atk=10)");
			break;
		}
		case 2:
		{	
			printString("a mighty axe (atk=12)");
			break;
		}
		case 3:
		{	
			printString("a bloody flail (atk=14)");
			break;
		}		
	}
	if ( (Player->Weapon1) && (Player->Weapon2) )
	{
		// The player has two weapons already.
		showPlayer(Player);
		printString("You already have two weapons");		
		printString("(1) drop Weapon1, (2) for Weapon2, (0) skip");
		c = getUserInput();
		//c = getch();
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
		case 1:return "Sword (atk10)";break;
		case 2:return "Axe (atk12)"; break;
		case 3:return "Flail (atk14)"; break;
	}
}

const char *getClassName(int index)
{
	switch (index)
	{
		case 0:return "Mage"; break;
		case 1:return "Paladin";break;
		case 2:return "Cavalier"; break;
	}
}

void setHealth(tPlayer *Player, int health)
{
	if (health > Player->Maxhealth)
		health = Player->Maxhealth;
	if (health < 0)
		health = 0;
	Player->health = health;
}	

void setMana(tPlayer *Player, int mana)
{
	if (mana > Player->Maxmana)
		mana = Player->Maxmana;
	if (mana < 0)
		mana = 0;
	Player->mana = mana;
}	

void setStamina(tPlayer *Player, int stamina)
{
	if (stamina > Player->Maxstamina)
		stamina = Player->Maxstamina;
	if (stamina < 0)
		stamina = 0;
	Player->stamina = stamina;
}	

void setExperience(tPlayer *Player, int BadGuyIndex)
{
	int aux;
	aux = BadGuyExperience[BadGuyIndex]+range_random(6);
	Player->experience += aux;
	printf("Gained %d experience!\n", aux);
	if(Player->experience >= 100*Player->level)
	{
		eputs("LEVEL UP!!!\n");
		Player->experience -= 100*Player->level;
		Player->level++;

		aux = 17+range_random(5)+Bonushealth[Player->player_class];
		Player->Maxhealth += aux;
		printf("+ %d hp!\n", aux);
		setHealth(Player,Player->Maxhealth);

		aux = 2+range_random(1)+Bonusstrength[Player->player_class];
		Player->strength += aux;
		printf("+ %d str!\n", aux);

		aux = 1+range_random(1)+Bonusdefense[Player->player_class];
		Player->defense += aux;
		printf("+ %d def!\n", aux);
	
		aux = 2+range_random(1)+Bonusint[Player->player_class];
		Player->intelligence += aux;
		printf("+ %d int!\n", aux);
		Player->Maxmana = 10*Player->intelligence;
		Player->mana = Player->Maxmana;
		
		aux = 3;
		Player->Maxstamina += aux;
		printf("+ %d stamina!\n\n", aux);
		Player->stamina = Player->Maxstamina;

		eputs("The player is fully restored!\n");
	}
}

void initPlayer(tPlayer *Player,tRealm *theRealm)
{
	int ValidOption=0;
	// get the player name
	int index=0;
	byte x,y;
	char ch=0;
	// Initialize the player's attributes
	eputs("Enter the player's name: ");
	while ( (index < MAX_NAME_LEN) && (ch != '\n') && (ch != '\r'))
	{
		//ch = getch();
		ch = getchar();
		if ( ch > '0' ) // strip conrol characters
		{
			Player->name[index++]=ch;
			//eputc(ch);
		}
	}
	eputs("Enter the player's class:\n");
	eputs("  (M)age (+Int -Str -Def)\n"); //class 0
	eputs("  (P)aladin (+Str +Def)\n");  //class 1
	eputs("  (C)avalier (+Hp +Str -Int)\n"); //class 2
	while(!ValidOption)
	{
		ch = getUserInput();
		//ch = getch();
		if((ch=='M')||(ch=='m'))
		{
			Player->player_class=0;
			ValidOption=1;
		}
		if((ch=='P')||(ch=='p'))
		{
			Player->player_class=1;
			ValidOption=1;
		}
		if((ch=='C')||(ch=='c'))
		{
			Player->player_class=2;
			ValidOption=1;
		}
	}
	Player->name[index]=0; // terminate the name
	Player->Maxhealth=120+Classhealth[Player->player_class];
	Player->health = Player->Maxhealth;
	Player->strength = 10+range_random(5)+Classstrength[Player->player_class];
	Player->defense = 10+Classdefense[Player->player_class];
	Player->Maxstamina = 20;
	Player->stamina = Player->Maxstamina;
	Player->intelligence = 10+range_random(5)+Classint[Player->player_class];
	Player->Maxmana = 10*Player->intelligence;
	Player->mana = Player->Maxmana;		
	Player->wealth=20+range_random(10);
	Player->level=1;
	Player->experience=0;
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
    COORD ptext={56,11},ctext={56,12},utext={56,13},utext1={56,14},utext2={56,15},utext3={56,16},utext4={56,17},utext5={56,18},utext6={56,19},utext7={56,20},utext8={56,21}, userscoresz={55,10};
	printBorder(24,13,userscoresz,15); //15 is the background color, 20 is the length and 13 is the heigth
    SetConsoleCursorPosition(out,ptext);
	printf("Player Board! \n");
	SetConsoleCursorPosition(out,ctext);
    printf("Name: %s",thePlayer->name);
	SetConsoleCursorPosition(out,utext);
    printf("Class: %s",getClassName(thePlayer->player_class));
	SetConsoleCursorPosition(out,utext1);
    printf("Level: %d   Exp: %d/%d",thePlayer->level, thePlayer->experience, (thePlayer->level)*100);
	SetConsoleCursorPosition(out,utext2);
    printf("HP: %d/%d",thePlayer->health, thePlayer->Maxhealth);
    SetConsoleCursorPosition(out,utext3);
    printf("Mana: %d/%d",thePlayer->mana, thePlayer->Maxmana);
	SetConsoleCursorPosition(out,utext4);
	printf("Stamina: %d/%d",thePlayer->stamina, thePlayer->Maxstamina);
    SetConsoleCursorPosition(out,utext5);
    printf("Str: %d   Int: %d",thePlayer->strength, thePlayer->intelligence);
    SetConsoleCursorPosition(out,utext6);
    printf("Def: %d   Gold: %d",thePlayer->defense, thePlayer->wealth);
    SetConsoleCursorPosition(out,utext7);
	printf("Weapon1: %s",getWeaponName(thePlayer->Weapon1));
	SetConsoleCursorPosition(out,utext8);
	printf("Weapon2: %s",getWeaponName(thePlayer->Weapon2));

 /*	printf("Name: %s\n", thePlayer->name);
   	printf("Health: %d/%d\n", thePlayer->health, thePlayer->Maxhealth);
   	printf("Strength: %d\n", thePlayer->strength);
   	printf("Stamina: %d\n", thePlayer->stamina);
   	printf("Mana: %d/%d\n", thePlayer->mana, thePlayer->Maxmana);
	printf("Defense: %d\n", thePlayer->defense);
	printf("Intelligence: %d\n", thePlayer->intelligence);
	printf("Experience: %d/%d\n", thePlayer->experience, (thePlayer->level)*100);
	printf("Level: %d\n", thePlayer->level);
   	printf("Wealth: %d\n", thePlayer->wealth);
   	printf("Location: %d, %d\n", thePlayer->x, thePlayer->y);
	printf("Weapon1: %s, ", getWeaponName(thePlayer->Weapon1));
	printf("Weapon2: %s\n", getWeaponName(thePlayer->Weapon2));
*/
}

void SavePlayer(tPlayer *thePlayer)
{
   	SaveFile = fopen("SavePlayer.txt", "w");

   	fprintf(SaveFile, "Name: %s\n", thePlayer->name);
	fprintf(SaveFile, "Class: %d\n", thePlayer->player_class);
   	fprintf(SaveFile, "MaxHealth: %d\n", thePlayer->Maxhealth);
   	fprintf(SaveFile, "Health: %d\n", thePlayer->health);
   	fprintf(SaveFile, "Strength: %d\n", thePlayer->strength);
	fprintf(SaveFile, "MaxStamina: %d\n", thePlayer->Maxstamina);
   	fprintf(SaveFile, "Stamina: %d\n", thePlayer->stamina);
   	fprintf(SaveFile, "MaxMana: %d\n", thePlayer->Maxmana);
   	fprintf(SaveFile, "Mana: %d\n", thePlayer->mana);
	fprintf(SaveFile, "Defense: %d\n", thePlayer->defense);
	fprintf(SaveFile, "Intelligence: %d\n", thePlayer->intelligence);
	fprintf(SaveFile, "Experience: %d\n", thePlayer->experience);
	fprintf(SaveFile, "Level: %d\n", thePlayer->level);
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
	fprintf(SaveFile, "%d\n", RealmLevel);
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
	fscanf(SaveFile, "%d", &theRealm->RealmLevel);
	RealmLevel=theRealm->RealmLevel;
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
	fscanf(SaveFile, "%d", &thePlayer->player_class);
	
   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->Maxhealth);
	
   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->health);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->strength);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->Maxstamina);
	
   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->stamina);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->Maxmana);

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
  	fscanf(SaveFile, "%d", &thePlayer->experience);

   	fscanf(SaveFile, "%s", aux); //moving the file position to the wanted location
	fseek (SaveFile , 1 , SEEK_CUR ); //moves through the "space" char
  	fscanf(SaveFile, "%d", &thePlayer->level);

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

void initRealm(tRealm *Realm, byte RealmLevel)
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
			else if (Rnd == 97) // put in some rare stuff
				Realm->map[y][x]=	RareTypes[range_random(sizeof(RareTypes)-1)];
			else if (Rnd >= 95) // put in some good stuff
				Realm->map[y][x]=	RareTypes[range_random(sizeof(RareTypes)-1)];
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
	
	printString("=============================================");	
	printString("=           FORGOTTEN REALMS V1.2           =");
	printString("=============================================\n\n");	
	
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
	printString("\r\n\nLegend");
	printString("(W)olf, (T)roll, (O)gre, (D)ragon, (H)ag, e(X)it");
	printString("(w)eapon, (g)old), (m)agic, (s)trength, (h)ealth, (*)rock");
	printString("Press H for help");
	printString("@=You");
}
void showHelp()
{
	printString("W,S,A,D : go North, South, West, East");
	printString(". : save game");
	printString("r : restart game (required to leave the game with Ctrl+C)");
	printString("e : rest");
}

void showGameMessage(char *Msg)
{
	printString(Msg);
	//printString("Ready");
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

void rest(tPlayer *Player)
{
	eputs("resting... (Please wait and don't press any button!)");
	delay(7000);
	eputs("\rYou feel rested!                                               ");
	setStamina(Player, Player->stamina+5);
	setHealth(Player, Player->health+3);
	setMana(Player, Player->mana+3);
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

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}
