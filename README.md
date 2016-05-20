//===================================================================================================
//Realm.c
//===================================================================================================

 
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

#include "rlutil.h"

// Find types: h(ealth),s(trength),m(agic),g(old),w(eapon)
const char FindTypes[]={'h','m','g'};
const char RareTypes[]={'s','w','.','.'}; //50% chance of being nothing! decreases chance of generation to 0.5%

//Enemies Counter
int countWolf = 0;
int countOgre = 0;
int countTroll = 0;
int countDragon = 0;
int countHag = 0;

// The following arrays define the bad guys and 
// their battle properies - ordering matters!

// Baddie types : W(olf),O(gre),T(roll),D(ragon),H(ag)
const char Baddies[]={'W','O','T','D','H'};

// Bosses types : Fire elemental, Apocalypse
const char Bosses[]={'F','A'};
const int BossesFreezeSpellDamage[]={20,10};
const int BossesFireSpellDamage[]={-20,10};
const int BossesLightningSpellDamage[]={10,10};
const int BossesDamage[]={30,50};
const int BossesExperience[]={250, 700};
const int BossesLife[]={300,600};

// The following is the weapons' damage
const byte WeaponDamage[]={3,8,11,14,20}; //stamina cost: 1/1/2/3/4
#define ICE_SPELL_COST 15
#define FIRE_SPELL_COST 15
#define LIGHTNING_SPELL_COST 15
const int FreezeSpellDamage[]={12,12,18,12,5};
const int FireSpellDamage[]={12,18,12,12,5};
const int LightningSpellDamage[]={12,12,12,18,5};
const int BadGuyDamage[]={13,18,20,25,15};
const int BadGuyExperience[]={30,50,55,70,40};
const int BadGuyLife[]={75,100,90,110,80};

//base bonus for each class: mage/paladin/cavalier
//mage: +4int, -1str, -1def
//paladin: +2str, +1def
//cavalier: +15hp, +3str, -1int
const int Classhealth[]={0,0,15};
const int Classstrength[]={-1,2,3};
const int Classint[]={4,0,-1};
const int Classdefense[]={-1,1,0};

//levelup bonus for each class: mage/paladin/cavalier
//mage: +2int
//paladin: +1str, +1def
//cavalier: +3hp, +1str
const int Bonushealth[]={0,0,3};
const int Bonusstrength[]={0,1,1};
const int Bonusint[]={2,0,0};
const int Bonusdefense[]={0,1,0};

int GameStarted = 0;
tPlayer thePlayer;
tRealm theRealm;
int RealmLevel=0;
int RealmSizeX=20;
char ch;
FILE *SaveFile;
void delay(int len);
int flag;
int questNumber [] = {0,0,0};

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

	eputs("=============================================\n="); //printstring does \n automatically, eputs doesn't
	setColor(RED);	
	eputs("           FORGOTTEN REALMS V3.0           ");
	setColor(GREY);
	printString("=\n=============================================\n");	
	
	
    printString("A truly epic RPG-roguelike game");
	setColor(DARKGREY);
	printString("(Please play full-screen!)\n");
	setColor(GREY);
	
	
	showHelp();		
	while(GameStarted == 0)
	{
		
		showGameMessage("\nPress S to start a new game \nPress L to load a previous game");
		ch = getUserInput() | 32;			
		
		if ( (ch == 's') || (ch == 'l') )
			GameStarted = 1;
	}
	
	if ((ch == 's') )
	{
		initRealm(&theRealm, RealmLevel);	
		initPlayer(&thePlayer,&theRealm);
	}
	if((ch == 'l') )
	{
		LoadPlayer(&thePlayer);
		LoadRealm(&theRealm);
		eputs("Game Loaded!\n");
	}
	delay(200);
	system("cls");
	showRealm(&theRealm,&thePlayer);
	showPlayer(&thePlayer);
	COORD position={0,33};
	SetConsoleCursorPosition(out,position);
	//eputs("\n");

	while (1)
	{
		flag=1; //clean screen
		//ch = getUserInput();
		ch = getch();
		ch = ch | 32; // enforce lower case
		if(ch=='r')
		{
			eputs("Are you sure? (Y/N)\n");
			char option=getUserInput() | 32;
			if(option=='y')
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
			
			//Current quests
			case 'q' : {
				showQuests(&thePlayer);
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
    	COORD position={0,0}, position2={0,33};
		SetConsoleCursorPosition(out,position);
		//system("cls");
		showRealm(&theRealm,&thePlayer);
		printString(""); //newline
		
		if (flag)
		{
			clean_lines(4);
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
			if (new_y < 20-1)
				new_y++;
			break;
		}
		case 'e' :
		{
			if (new_x <  RealmSizeX-1)
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
		case 'F' :{
			showGameMessage("(BOSS) A Fire Elemental wants to melts you!");
			Consumed = doBossBattle(Player,0);
			system("pause");
			system("cls");
			break;
		}
		case 'A' :{
			showGameMessage("(FINAL BOSS) Apocalypse has come to bring TOTAL OBLIVION!!!");
			Consumed = doBossBattle(Player,1);
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
		
		//NPCS
		case 'Y':{
			showGameMessage("Hello, I am Yuki! Want to talk? (Y/N)");
			ch = getUserInput() | 32; // get user input and force lower case
			if(ch=='y')
				doQuest(Player,Realm,1);
			break;
		}
		case 'K':{
			showGameMessage("Kirito here! Want to talk? (Y/N)");
			ch = getUserInput() | 32; // get user input and force lower case
			if(ch=='y')
				doQuest(Player,Realm,2);
			break;
		}
		case 'L':{
			showGameMessage("Help Lu-chan! Want to talk? (Y/N)");
			ch = getUserInput() | 32; // get user input and force lower case
			if(ch=='y')
				doQuest(Player,Realm,3);
			break;
		}
		
		case 'h' :{
			showGameMessage("You find an potion of health! hp+30!");
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
			Player->wealth+=5+range_random(5);			
			Consumed = 1;
			flag=0;
			break;
		}
		case 'm' :{
			showGameMessage("You find a potion of mana! mana +20!");
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
			RealmSizeX += 4;
			if(RealmSizeX>MAP_WIDTH)
				RealmSizeX=MAP_WIDTH;
			setHealth(Player,Player->Maxhealth); // maximize health
			initRealm(&theRealm, RealmLevel);
			system("pause");
			system("cls");
			break;
		}
		case '$' :{
			showGameMessage("You find a merchant! Want to purchase something? (Y/N) ");
			ch = getUserInput() | 32; // get user input and force lower case
			if(ch=='y')
				merchant(Player);
			break;
		}
	}
	if (Consumed)
		Realm->map[new_y][new_x] = '.'; // remove any item that was found
}

int scanRealm (tRealm *theRealm) 
{
	
	int x,y,aux;
	
	countWolf = 0;
	countOgre = 0;
	countTroll = 0;
	countDragon = 0;
	countHag = 0;
	
	for(y=0;y<20;y++)
	{
		for(x=0;x<RealmSizeX;x++)
		{
			aux = theRealm->map[y][x];
			
			switch (aux)
			{			
				case 'W' :
				{
					countWolf++;
					break;					
				}
				
				case 'O':
				{
					countOgre++;
					break;
				}
				
				case 'T':
				{
					countTroll++;
					break;
				}
				
				case 'D':
				{
					countDragon++;
					break;
				}
				
				case 'H':
				{
					countHag++;
					break;
				}
			}
		}
	}
}

int doQuest (tPlayer *Player,tRealm *theRealm,int NPC) 
{
	
	int x,y;
	int Consumed = 0;
	
	int new_x, new_y;
	new_x = Player->x;
	new_y = Player->y;
	
	scanRealm(theRealm);
	
	switch (NPC) {
			
		//Yuki
		case 1:
		{
			if (countDragon == 0)
			{					
				printf("You are my saviour! Thank you very much!\n\n");
				printf("Gained 80 gold pieces!\n\n");
				Player->wealth += 80;
				Consumed = 1;
				questNumber[0] = 0;
				system("pause");
				system("cls");
			}	
			else
			{			
				printf("I am terrified of Dragons!\n");
				printf("Would help me and kill them? I can pay you back! (Y/N)\n");
				
				char yuki = getUserInput() | 32;
			
				if (yuki == 'y')
				{				
					printf("I will wait until you come back!\n\n");
					questNumber[0] = 1;
					system("pause");
					system("cls");
				}
			}
			break;
		}
		
		//Kirito
		case 2:
		{
			if (countWolf == 0 && countOgre == 0 && countTroll == 0 && countDragon == 0 && countHag == 0)
			{								
				printf("You actually killed all the Monsters! Congratulations!\n");
				printf("Here is your reward!\n\n");
				printf("Gained a Legendary Katana!\n\n");
				addWeapon(Player, 4);
				Consumed = 1;
				questNumber[1] = 0;
				system("pause");
				system("cls");
			}		
			else
			{	
				printf("I am bored, want to make a bet?\n");
				printf("I bet you can't kill all the Monsters in this level! If you win I will give you my weapon! (Y/N)\n");
					
				char kirito = getUserInput() | 32;
			
				if (kirito == 'y')
				{			
					printf("I will be waiting! Haha!\n\n");
					questNumber[1] = 1;
					system("pause");
					system("cls");
				}
			}
			break;
		}
		
		//Lupita
		case 3:
		{
			if (countTroll == 0 && countOgre == 0)
			{
				printf("You are the best! Thanks a million!\n");
				printf("Here is your gift!\n\n");
				printf("Received 120 Gold!\n\n");
				Player->wealth += 120;
				Consumed = 1;
				questNumber[2] = 0;
				system("pause");
				system("cls");
			}		
			else
			{
				printf("A group of Ogres and Trolls are following me. I am super sacared!\n");
				printf("Could you kill them, please!? (Y/N)\n");
						
				char lupita = getUserInput() | 32;
			
				if (lupita == 'y')
				{	
					printf("May the gods bless your journey!\n\n");
					questNumber[2] = 1;
					system("pause");
					system("cls");
				}
			}
			break;
		}
	}
	
	if (Consumed)
		theRealm->map[new_y][new_x] = '.'; // remove any item that was found

}

void showQuests(tPlayer *Player)
{ 
	if (questNumber[0] == 0 && questNumber[1] == 0 && questNumber[2] == 0)
		printf ("You don't have any active quests!\n");
	else
	{
		if(questNumber[0] == 1)
			printf ("Lend a hand to Yuki and kill the Dragons!\n");
		if(questNumber[1] == 1)
			printf ("Win Kirito's bet and kill all the monsters in this level!\n");
		if(questNumber[2] == 1)
			printf ("Save Lupita from the Ogres and Trolls!\n");
	}	
}

int doChallenge(tPlayer *Player,int BadGuyIndex)
{
	int aux;
	char ch;
	int Damage;
	int BadGuyHealth = BadGuyLife[BadGuyIndex]+10*RealmLevel;
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
						Damage = FreezeSpellDamage[BadGuyIndex]+(Player->intelligence)/2+range_random((Player->intelligence)/2); //int 12: dmg +6to12
						if(Damage<2) Damage=2;  //min damage
						if(Damage>50) Damage=50;  //max damage
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
						Damage = FireSpellDamage[BadGuyIndex]+(Player->intelligence)/2+range_random((Player->intelligence)/2); //int 12: dmg +6to12
						if(Damage<2) Damage=2;  //min damage
						if(Damage>50) Damage=50;  //max damage
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
						Damage = LightningSpellDamage[BadGuyIndex]+(Player->intelligence)/2+range_random((Player->intelligence)/2); //int 12: dmg +6to12
						if(Damage<2) Damage=2;  //min damage
						if(Damage>50) Damage=50;  //max damage
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
						if(Damage<2) Damage=2;  //min damage
						if(Damage>50) Damage=50;  //max damage
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
						if(Damage<2) Damage=2;  //min damage
						if(Damage>50) Damage=50;  //max damage
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
						if(Damage<2) Damage=2;  //min damage
						if(Damage>50) Damage=50;  //max damage
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
				Damage = BadGuyDamage[BadGuyIndex]+range_random(5)-Player->defense+RealmLevel*3;
				if(Damage<2) Damage=2;  //min damage
				if(Damage>50) Damage=50;  //max damage
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
		if(range_random(100)>75) //25% chance of losing life before leaving battle
		{
			Damage = range_random(8)+(2*Player->level)+RealmLevel;
			if(Damage<2) Damage=2;  //min damage
			if(Damage>50) Damage=50;  //max damage
			setHealth(Player, (Player->health)-Damage);
			printf("You stumble on a rock while fleeing, -%d hp\n", aux);
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
	eputs("You got ");
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
		case 4:
		{	
			printString("a legendary katana (atk=20)");
			break;
		}
	}
	if ( (Player->Weapon1) && (Player->Weapon2) )
	{
		// The player has two weapons already.
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
		case 4:return "Katana (atk20)"; break;
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
		x=range_random(RealmSizeX-1);
		y=range_random(20-1);		
	} while(theRealm->map[y][x] != '.');
	Player->x=x;
	Player->y=y;
}

void showPlayer(tPlayer *thePlayer)
{
	HANDLE out;
    out=GetStdHandle(STD_OUTPUT_HANDLE);
    COORD ptext={56,11},ctext={56,12},utext={56,13},utext1={56,14},utext2={56,15},utext3={56,16},utext4={56,17},utext5={56,18},utext6={56,19},utext7={56,20},utext8={56,21}, userscoresz={55,10};
	printBorder(25,13,userscoresz,15); //15 is the background color, 25 is the length and 13 is the heigth
    SetConsoleCursorPosition(out,ptext);
	setColor(RED);
	printf("     Player Board     \n");
	setColor(GREY);
	SetConsoleCursorPosition(out,ctext);
    printf("Name: %s",thePlayer->name);
	SetConsoleCursorPosition(out,utext);
    printf("Class: %s",getClassName(thePlayer->player_class));
	SetConsoleCursorPosition(out,utext1);
    printf("Level: %d  XP: %d/%d",thePlayer->level, thePlayer->experience, (thePlayer->level)*100);
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
	for(y=0;y<20;y++)
	{
		for(x=0;x<RealmSizeX;x++)
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
	RealmSizeX=20+RealmLevel*4;
	if(RealmSizeX>MAP_WIDTH)
		RealmSizeX=MAP_WIDTH;
	for(y=0;y<20;y++)
	{
		for(x=0;x<RealmSizeX;x++)
		{
			fscanf(SaveFile, "%d", &theRealm->map[y][x]);
			//moves through the "space" char or "\n"
			fseek(SaveFile , 1+ x/(RealmSizeX-1) , SEEK_CUR );
		}
	}
  	fclose(SaveFile);	
}

void LoadPlayer(tPlayer *thePlayer)
{
	char aux[20];
   	SaveFile = fopen("SavePlayer.txt", "r");
    //moving the file position to the wanted location
   	fscanf(SaveFile, "%s", aux); 
	//moves through the "space" char
	fseek (SaveFile , 1 , SEEK_CUR ); 
	fscanf(SaveFile, "%s", thePlayer->name);
	
	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR ); 
	fscanf(SaveFile, "%d", &thePlayer->player_class);
	
   	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR ); 
  	fscanf(SaveFile, "%d", &thePlayer->Maxhealth);
	
   	fscanf(SaveFile, "%s", aux);
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->health);

   	fscanf(SaveFile, "%s", aux);
	fseek (SaveFile , 1 , SEEK_CUR ); 
  	fscanf(SaveFile, "%d", &thePlayer->strength);

   	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->Maxstamina);
	
   	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR ); 
  	fscanf(SaveFile, "%d", &thePlayer->stamina);

   	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->Maxmana);

   	fscanf(SaveFile, "%s", aux);
	fseek (SaveFile , 1 , SEEK_CUR ); 
  	fscanf(SaveFile, "%d", &thePlayer->mana);

	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->defense);

	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->intelligence);

   	fscanf(SaveFile, "%s", aux);
	fseek (SaveFile , 1 , SEEK_CUR ); 
  	fscanf(SaveFile, "%d", &thePlayer->experience);

   	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->level);

   	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR ); 
  	fscanf(SaveFile, "%d", &thePlayer->wealth);

   	fscanf(SaveFile, "%s", aux);
	fseek (SaveFile , 1 , SEEK_CUR ); 
  	fscanf(SaveFile, "%d", &thePlayer->x);

   	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->y);

   	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->Weapon1);

	fscanf(SaveFile, "%s", aux); 
	fseek (SaveFile , 1 , SEEK_CUR );
  	fscanf(SaveFile, "%d", &thePlayer->Weapon2);

	fclose(SaveFile);
}

void initRealm(tRealm *Realm, byte RealmLevel)
{
	int x,y;
	int Rnd;
   	
	// clear the map to begin with
	for (y=0;y < 20; y++)
	{
		for (x=0; x < RealmSizeX; x++)
		{
			Rnd = range_random(100);
			
			if (Rnd >= 98) // put in some baddies
				Realm->map[y][x]=	Baddies[range_random(sizeof(Baddies)-1)];
			else if (Rnd == 97) // put in some rare stuff
				Realm->map[y][x]=	RareTypes[range_random(sizeof(RareTypes)-1)];
			else if (Rnd >= 95) // put in some good stuff
				Realm->map[y][x]=	FindTypes[range_random(sizeof(FindTypes)-1)];
			else if (Rnd >= 89) // put in some rocks
				Realm->map[y][x]='*'; 
			else // put in empty space
				Realm->map[y][x] ='.';
		}
	}
	
	One_monster(Realm); //generates at least one of the quest monsters
	
	//NPC Yuki
	do{
	   x = range_random(RealmSizeX-1);
	   y = range_random(20-1);
	}while(Realm->map[y][x]!='.');
	Realm->map[y][x]='Y';
	
	//First Boss -> Fire elemental
	if(RealmLevel==4)
	{
		do{
			x = range_random(RealmSizeX-1);
			y = range_random(20-1);
		}while(Realm->map[y][x]!='.');
		Realm->map[y][x]='F';
	}
	
	//Final Boss -> Apocalypse
	if(RealmLevel==9)
	{
		do{
			x = range_random(RealmSizeX-1);
			y = range_random(20-1);
		}while(Realm->map[y][x]!='.');
		Realm->map[y][x]='A';
	}
	
	//NPC Kirito
	do{
	    x = range_random(RealmSizeX-1);
	    y = range_random(20-1);
	}while(Realm->map[y][x]!='.');
	Realm->map[y][x]='K';
	
	//NPC Lupita
	do{
	   x = range_random(RealmSizeX-1);
	   y = range_random(20-1);
	}while(Realm->map[y][x]!='.');
	Realm->map[y][x]='L';
	
	
	// shop/store/merchant
	do{
	   x = range_random(RealmSizeX-1);
	   y = range_random(20-1);
	}while(Realm->map[y][x]!='.');
	Realm->map[y][x]='$';
	
	
	// finally put the exit to the next level in
	do{
	   x = range_random(RealmSizeX-1);
	   y = range_random(20-1);
	}while(Realm->map[y][x]!='.');
	Realm->map[y][x]='X';
}

void showRealm(tRealm *Realm,tPlayer *thePlayer)
{
	int x,y;
	int new_x, new_y;
	x = thePlayer->x;
	y = thePlayer->y;
	byte AreaContents = Realm->map[y][x];
	
	eputs("=============================================\n=");
	setColor(RED);	
	eputs("           FORGOTTEN REALMS V3.0           ");
	setColor(GREY);
	printString("=\n=============================================\n");
	
	printString("The Realm ");	
	for (y=0;y<20;y++)
	{
		for (x=0;x<RealmSizeX;x++)
		{
			
			if ( (x==thePlayer->x) && (y==thePlayer->y)) {
				setColor(LIGHTRED);
				eputc('@');
			}
			else{
				switch(Realm->map[y][x])
				{
					case 'L':
					case 'K':
					case 'Y':
					{
						setColor(LIGHTMAGENTA);
						eputc(Realm->map[y][x]);
						break;
					}
					case '$':
					{
						setColor(YELLOW);
						eputc(Realm->map[y][x]);
						break;
					}
					case 'X':
					{
						setColor(WHITE);
						eputc(Realm->map[y][x]);
						break;
					}
					case 'F':
					case 'A':
					{
						setColor(RED);
						eputc(Realm->map[y][x]);
						break;
					}
					default:
					{
						setColor(GREY);
						eputc(Realm->map[y][x]);
						break;
					}
				}
			}
		}
		setColor(GREY);
		eputs("\r\n");
	}
	setColor(RED); 
	printString("\r\n\nLegend");
	setColor(GREY);
	printString("(W)olf, (T)roll, (O)gre, (D)ragon, (H)ag, e(X)it, ($)merchant");
	printString("(w)eapon, (g)old nugget, (m)agic, (s)trength potion, (h)ealth");
	printString("(*)rock, @=You, (Y)uki (K)irito (L)upita = NPCs");
	setColor(DARKGREY);
	printString("Press H for help or Q to show your active quests");
	setColor(GREY);
	
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

void titleScreen()         //displays the title screen
{
	printf("Welcome to the Whispering Burrows!\n");
}

void printBorder(int _length,int _width,COORD _coordinates,int _color)     //border printing function
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

void merchant(tPlayer *Player)
{
	int exit=0;
	while(!exit)
	{
		system("cls");
		printString("=============================================");	
		printString("=           TRAVELLING MERCHANT             =");
		printString("=============================================\n");
		printf("Hello there!!\n\n");
		printf("Current gold: %d\n\n", Player->wealth);
		printf("  potion of (h)ealth (hp+30): $10\n\n");
		printf("  potion of (m)ana (mp+20): $10\n\n");
		printf("  potion of (s)trength: $40\n\n");
		printf("  ramdom (w)eapon: $25\n\n");
		printf("  (l)eave store\n\n");
		ch = getUserInput() | 32;
		switch(ch)
		{
			case 'h':
			{
				if(Player->wealth>=10)
				{
					printf("\nItem purchased!\n\n");
					setHealth(Player,Player->health+30);
					Player->wealth -=10;
					delay(700);
				}
				else
				{
					printf("\nYou dont have enough gold!");
					delay(700);
				}
				break;
			}
			case 'm':
			{
				if(Player->wealth>=10)
				{
					printf("\nItem purchased!\n\n");
					setMana(Player,Player->mana+30);
					Player->wealth -=10;
					delay(700);
				}
				else
				{
					printf("\nYou dont have enough gold!");
					delay(700);
				}
				break;
			}
			case 's':
			{
				if(Player->wealth>=40)
				{
					printf("\nItem purchased!\n\n");
					Player->strength ++;
					Player->wealth -=40;
					delay(700);
				}
				else
				{
					printf("\nYou dont have enough gold!");
					delay(700);
				}
				break;
			}
			case 'w':
			{
				if(Player->wealth>=25)
				{
					printf("\nItem purchased!\n\n");
					addWeapon(Player,range_random(MAX_WEAPONS-2)+1);
					Player->wealth -=25;
					delay(800);
				}
				else
				{
					printf("\nYou dont have enough gold!");
					delay(700);
				}
				break;
			}
			case 'l':
			{
				printf("\nBye!!\n\n");
				delay(800);	
				exit=1;
				system("cls");
				break;
			}
			default:
			{
				printf("\nInvalid option!");
				delay(700);	
				break;
			}
		}
	}
}

void clean_lines(int num)
{
	int i=0;
	for(i;i<num;i++)
		printString("                                                                                                 ");
}

void One_monster(tRealm *Realm)
{
	int x,y;
	scanRealm(Realm);
	if(countDragon==0)
	{
		do{
			x = range_random(RealmSizeX-1);
			y = range_random(20-1);
		}while(Realm->map[y][x]!='.');
		Realm->map[y][x]='D';
		countDragon++;
	}
	if(countTroll==0)
	{
		do{
			x = range_random(RealmSizeX-1);
			y = range_random(20-1);
		}while(Realm->map[y][x]!='.');
		Realm->map[y][x]='T';
		countTroll++;
	}
	if(countOgre==0)
	{
		do{
			x = range_random(RealmSizeX-1);
			y = range_random(20-1);
		}while(Realm->map[y][x]!='.');
		Realm->map[y][x]='O';
		countOgre++;
	}
}

int doBossBattle(tPlayer *Player,int BossIndex)
{
	int aux;
	char ch;
	int Damage;
	int BossHealth = BossesLife[BossIndex];
	printString("BOSS BATTLE!! (You cannot escape!)");
	printString("Choose action");
	while ( (Player->health > 0) && (BossHealth > 0) )
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
		ch = getUserInput() | 32;
		//ch = getch();
		switch (ch)
		{
			case 'i':
			{
				if (Player->mana > ICE_SPELL_COST){
					printString("FREEZE!");
					Player->mana -= ICE_SPELL_COST;
					Damage = BossesFreezeSpellDamage[BossIndex]+(Player->intelligence)/2+range_random((Player->intelligence)/2); //int 12: dmg +6to12
					if(Damage<2) Damage=2;  //min damage
					if(Damage>50) Damage=50;  //max damage
					BossHealth -= Damage;
					eputs("you dealed "); printf("%d", Damage); eputs(" damage!\n");
					zap();
				}
				else
					eputs("You don't have sufficient mana!");
				break;
			}
			case 'f':
			{
				if (Player->mana > FIRE_SPELL_COST){
					printString("BURN!");
					Player->mana -= FIRE_SPELL_COST;
					Damage = BossesFireSpellDamage[BossIndex]+(Player->intelligence)/2+range_random((Player->intelligence)/2); //int 12: dmg +6to12
					if(Damage<2) Damage=2;  //min damage
					if(Damage>50) Damage=50;  //max damage
					BossHealth -= Damage;
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
					Damage = BossesLightningSpellDamage[BossIndex]+(Player->intelligence)/2+range_random((Player->intelligence)/2); //int 12: dmg +6to12
					if(Damage<2) Damage=2;  //min damage
					if(Damage>50) Damage=50;  //max damage
					BossHealth -= Damage;
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
					if(Damage<2) Damage=2;  //min damage
					if(Damage>50) Damage=50;  //max damage
					BossHealth -= Damage;
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
					if(Damage<2) Damage=2;  //min damage
					if(Damage>50) Damage=50;  //max damage
					BossHealth -= Damage;
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
					if(Damage<2) Damage=2;  //min damage
					if(Damage>50) Damage=50;  //max damage
					BossHealth -= Damage;
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
		if (BossHealth <= 0)
			BossHealth = 0;
		else
		{
			eputs("\nBoss's turn!\n");
			Damage = BossesDamage[BossIndex]+range_random(10)-Player->defense;
			if(Damage<2) Damage=2;  //min damage
			if(Damage>50) Damage=50;  //max damage
			setHealth(Player,Player->health - Damage);
			eputs("you took "); printf("%d", Damage); eputs(" damage!\n\n");
			eputs("Health: you "); printf("%d", Player->health);
			eputs(", Boss " );printf("%d", BossHealth);
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
		aux = 40 + range_random(10);
		Player->wealth += aux;		
		eputs("You killed a BOSS! Congratulations!\n");
		printf("Gained %d gold pieces!\n", aux);
		setExperience(Player, BossIndex);			
		return 1;
	}
		
}

//===================================================================================================================
//Realm.h 
//===================================================================================================================

// realm.h
// Some game constants
#define MAP_WIDTH 40
#define MAP_HEIGHT 20
#define MAX_NAME_LEN 20
#define MAX_WEAPONS 5

#include<stdio.h>
#include<windows.h>

typedef unsigned char byte;
typedef struct {
	byte map[MAP_HEIGHT][MAP_WIDTH];
	byte RealmLevel;
} tRealm;
typedef struct {
	char name[MAX_NAME_LEN+1];
	byte player_class;
	int Maxhealth;
	int health;	
	int strength;
	int Maxstamina;
	int stamina;
	int Maxmana;
	int mana;
	int defense;
	int intelligence;
	int experience;
	int level;
	byte wealth;
	byte x,y;
	byte Weapon1;
	byte Weapon2;
} tPlayer;

// Function prototypes

void showHelp();
void showGameMessage(char *Msg);
char getUserInput();
void runGame(void);
void initRealm(tRealm *Realm, byte RealmLevel);
void showRealm(tRealm *Realm,tPlayer *thePlayer);
void SaveRealm(tRealm *Realm);
void LoadRealm(tRealm *Realm);
void initPlayer(tPlayer *Player,tRealm *Realm);
void showPlayer(tPlayer *thePlayer);
void SavePlayer(tPlayer *thePlayer);
void LoadPlayer(tPlayer *thePlayer);
void step(char Direction,tPlayer *Player,tRealm *Realm);
void setHealth(tPlayer *Player, int health);
int addWeapon(tPlayer *Player, int Weapon);
int doChallenge(tPlayer *Player, int BadGuyIndex);
const char *getWeaponName(int index);
void zap(void);
//NEW
void titleScreen(void);
void printBorder(int,int,COORD,int);
void setExperience(tPlayer *Player, int BadGuyIndex);
void setMana(tPlayer *Player, int mana);
void setStamina(tPlayer *Player, int stamina);
const char *getClassName(int index);
void rest(tPlayer *Player);
void delay(int milliseconds);
void merchant(tPlayer *Player);
int scanRealm (tRealm *theRealm);
int doQuest (tPlayer *Player,tRealm *theRealm,int NPC);
void showQuests(tPlayer *Player);
void clean_lines(int num);
void One_monster(tRealm *Realm);
int doBossBattle(tPlayer *Player, int BossIndex);


//===============================================================================================================
//rlutil.h //library to add colors
//===============================================================================================================

#pragma once
/**
 * File: rlutil.h
 *
 * About: Description
 * This file provides some useful utilities for console mode
 * roguelike game development with C and C++. It is aimed to
 * be cross-platform (at least Windows and Linux).
 *
 * About: Copyright
 * (C) 2010 Tapio Vierros
 *
 * About: Licensing
 * See <License>
 */


/// Define: RLUTIL_USE_ANSI
/// Define this to use ANSI escape sequences also on Windows
/// (defaults to using WinAPI instead).
#if 0
#define RLUTIL_USE_ANSI
#endif

/// Define: RLUTIL_STRING_T
/// Define/typedef this to your preference to override rlutil's string type.
///
/// Defaults to std::string with C++ and char* with C.
#if 0
#define RLUTIL_STRING_T char*
#endif

#ifndef RLUTIL_INLINE
	#ifdef _MSC_VER
		#define RLUTIL_INLINE __inline
	#else
		#define RLUTIL_INLINE static __inline__
	#endif
#endif

#ifdef __cplusplus
	/// Common C++ headers
	#include <iostream>
	#include <string>
	#include <cstdio> // for getch()
	/// Namespace forward declarations
	namespace rlutil {
		RLUTIL_INLINE void locate(int x, int y);
	}
#else
	#include <stdio.h> // for getch() / printf()
	#include <string.h> // for strlen()
	RLUTIL_INLINE void locate(int x, int y); // Forward declare for C to avoid warnings
#endif // __cplusplus

#ifdef _WIN32
	#include <windows.h>  // for WinAPI and Sleep()
	#define _NO_OLDNAMES  // for MinGW compatibility
	#include <conio.h>    // for getch() and kbhit()
	#define getch _getch
	#define kbhit _kbhit
#else
	#include <termios.h> // for getch() and kbhit()
	#include <unistd.h> // for getch(), kbhit() and (u)sleep()
	#include <sys/ioctl.h> // for getkey()
	#include <sys/types.h> // for kbhit()
	#include <sys/time.h> // for kbhit()

/// Function: getch
/// Get character without waiting for Return to be pressed.
/// Windows has this in conio.h
RLUTIL_INLINE int getch(void) {
	// Here be magic.
	struct termios oldt, newt;
	int ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

/// Function: kbhit
/// Determines if keyboard has been hit.
/// Windows has this in conio.h
RLUTIL_INLINE int kbhit(void) {
	// Here be dragons.
	static struct termios oldt, newt;
	int cnt = 0;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag    &= ~(ICANON | ECHO);
	newt.c_iflag     = 0; // input mode
	newt.c_oflag     = 0; // output mode
	newt.c_cc[VMIN]  = 1; // minimum time to wait
	newt.c_cc[VTIME] = 1; // minimum characters to wait for
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ioctl(0, FIONREAD, &cnt); // Read count
	struct timeval tv;
	tv.tv_sec  = 0;
	tv.tv_usec = 100;
	select(STDIN_FILENO+1, NULL, NULL, NULL, &tv); // A small time delay
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return cnt; // Return number of characters
}
#endif // _WIN32

#ifndef gotoxy
/// Function: gotoxy
/// Same as <rlutil.locate>.
RLUTIL_INLINE void gotoxy(int x, int y) {
	#ifdef __cplusplus
	rlutil::
	#endif
	locate(x,y);
}
#endif // gotoxy

#ifdef __cplusplus
/// Namespace: rlutil
/// In C++ all functions except <getch>, <kbhit> and <gotoxy> are arranged
/// under namespace rlutil. That is because some platforms have them defined
/// outside of rlutil.
namespace rlutil {
#endif

/**
 * Defs: Internal typedefs and macros
 * RLUTIL_STRING_T - String type depending on which one of C or C++ is used
 * RLUTIL_PRINT(str) - Printing macro independent of C/C++
 */

#ifdef __cplusplus
	#ifndef RLUTIL_STRING_T
		typedef std::string RLUTIL_STRING_T;
	#endif // RLUTIL_STRING_T

	#define RLUTIL_PRINT(st) do { std::cout << st; } while(false)
#else // __cplusplus
	#ifndef RLUTIL_STRING_T
		typedef const char* RLUTIL_STRING_T;
	#endif // RLUTIL_STRING_T

	#define RLUTIL_PRINT(st) printf("%s", st)
#endif // __cplusplus

/**
 * Enums: Color codes
 *
 * BLACK - Black
 * BLUE - Blue
 * GREEN - Green
 * CYAN - Cyan
 * RED - Red
 * MAGENTA - Magenta / purple
 * BROWN - Brown / dark yellow
 * GREY - Grey / dark white
 * DARKGREY - Dark grey / light black
 * LIGHTBLUE - Light blue
 * LIGHTGREEN - Light green
 * LIGHTCYAN - Light cyan
 * LIGHTRED - Light red
 * LIGHTMAGENTA - Light magenta / light purple
 * YELLOW - Yellow (bright)
 * WHITE - White (bright)
 */
enum {
	BLACK,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	GREY,
	DARKGREY,
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	YELLOW,
	WHITE
};

/**
 * Consts: ANSI escape strings
 *
 * ANSI_CLS                - Clears screen
 * ANSI_CONSOLE_TITLE_PRE  - Prefix for changing the window title, print the window title in between
 * ANSI_CONSOLE_TITLE_POST - Suffix for changing the window title, print the window title in between
 * ANSI_ATTRIBUTE_RESET    - Resets all attributes
 * ANSI_CURSOR_HIDE        - Hides the cursor
 * ANSI_CURSOR_SHOW        - Shows the cursor
 * ANSI_CURSOR_HOME        - Moves the cursor home (0,0)
 * ANSI_BLACK              - Black
 * ANSI_RED                - Red
 * ANSI_GREEN              - Green
 * ANSI_BROWN              - Brown / dark yellow
 * ANSI_BLUE               - Blue
 * ANSI_MAGENTA            - Magenta / purple
 * ANSI_CYAN               - Cyan
 * ANSI_GREY               - Grey / dark white
 * ANSI_DARKGREY           - Dark grey / light black
 * ANSI_LIGHTRED           - Light red
 * ANSI_LIGHTGREEN         - Light green
 * ANSI_YELLOW             - Yellow (bright)
 * ANSI_LIGHTBLUE          - Light blue
 * ANSI_LIGHTMAGENTA       - Light magenta / light purple
 * ANSI_LIGHTCYAN          - Light cyan
 * ANSI_WHITE              - White (bright)
 * ANSI_BACKGROUND_BLACK   - Black background
 * ANSI_BACKGROUND_RED     - Red background
 * ANSI_BACKGROUND_GREEN   - Green background
 * ANSI_BACKGROUND_YELLOW  - Yellow background
 * ANSI_BACKGROUND_BLUE    - Blue background
 * ANSI_BACKGROUND_MAGENTA - Magenta / purple background
 * ANSI_BACKGROUND_CYAN    - Cyan background
 * ANSI_BACKGROUND_WHITE   - White background
 */
const RLUTIL_STRING_T ANSI_CLS                = "\033[2J\033[3J";
const RLUTIL_STRING_T ANSI_CONSOLE_TITLE_PRE  = "\033]0;";
const RLUTIL_STRING_T ANSI_CONSOLE_TITLE_POST = "\007";
const RLUTIL_STRING_T ANSI_ATTRIBUTE_RESET    = "\033[0m";
const RLUTIL_STRING_T ANSI_CURSOR_HIDE        = "\033[?25l";
const RLUTIL_STRING_T ANSI_CURSOR_SHOW        = "\033[?25h";
const RLUTIL_STRING_T ANSI_CURSOR_HOME        = "\033[H";
const RLUTIL_STRING_T ANSI_BLACK              = "\033[22;30m";
const RLUTIL_STRING_T ANSI_RED                = "\033[22;31m";
const RLUTIL_STRING_T ANSI_GREEN              = "\033[22;32m";
const RLUTIL_STRING_T ANSI_BROWN              = "\033[22;33m";
const RLUTIL_STRING_T ANSI_BLUE               = "\033[22;34m";
const RLUTIL_STRING_T ANSI_MAGENTA            = "\033[22;35m";
const RLUTIL_STRING_T ANSI_CYAN               = "\033[22;36m";
const RLUTIL_STRING_T ANSI_GREY               = "\033[22;37m";
const RLUTIL_STRING_T ANSI_DARKGREY           = "\033[01;30m";
const RLUTIL_STRING_T ANSI_LIGHTRED           = "\033[01;31m";
const RLUTIL_STRING_T ANSI_LIGHTGREEN         = "\033[01;32m";
const RLUTIL_STRING_T ANSI_YELLOW             = "\033[01;33m";
const RLUTIL_STRING_T ANSI_LIGHTBLUE          = "\033[01;34m";
const RLUTIL_STRING_T ANSI_LIGHTMAGENTA       = "\033[01;35m";
const RLUTIL_STRING_T ANSI_LIGHTCYAN          = "\033[01;36m";
const RLUTIL_STRING_T ANSI_WHITE              = "\033[01;37m";
const RLUTIL_STRING_T ANSI_BACKGROUND_BLACK   = "\033[40m";
const RLUTIL_STRING_T ANSI_BACKGROUND_RED     = "\033[41m";
const RLUTIL_STRING_T ANSI_BACKGROUND_GREEN   = "\033[42m";
const RLUTIL_STRING_T ANSI_BACKGROUND_YELLOW  = "\033[43m";
const RLUTIL_STRING_T ANSI_BACKGROUND_BLUE    = "\033[44m";
const RLUTIL_STRING_T ANSI_BACKGROUND_MAGENTA = "\033[45m";
const RLUTIL_STRING_T ANSI_BACKGROUND_CYAN    = "\033[46m";
const RLUTIL_STRING_T ANSI_BACKGROUND_WHITE   = "\033[47m";
// Remaining colors not supported as background colors

/**
 * Enums: Key codes for keyhit()
 *
 * KEY_ESCAPE  - Escape
 * KEY_ENTER   - Enter
 * KEY_SPACE   - Space
 * KEY_INSERT  - Insert
 * KEY_HOME    - Home
 * KEY_END     - End
 * KEY_DELETE  - Delete
 * KEY_PGUP    - PageUp
 * KEY_PGDOWN  - PageDown
 * KEY_UP      - Up arrow
 * KEY_DOWN    - Down arrow
 * KEY_LEFT    - Left arrow
 * KEY_RIGHT   - Right arrow
 * KEY_F1      - F1
 * KEY_F2      - F2
 * KEY_F3      - F3
 * KEY_F4      - F4
 * KEY_F5      - F5
 * KEY_F6      - F6
 * KEY_F7      - F7
 * KEY_F8      - F8
 * KEY_F9      - F9
 * KEY_F10     - F10
 * KEY_F11     - F11
 * KEY_F12     - F12
 * KEY_NUMDEL  - Numpad del
 * KEY_NUMPAD0 - Numpad 0
 * KEY_NUMPAD1 - Numpad 1
 * KEY_NUMPAD2 - Numpad 2
 * KEY_NUMPAD3 - Numpad 3
 * KEY_NUMPAD4 - Numpad 4
 * KEY_NUMPAD5 - Numpad 5
 * KEY_NUMPAD6 - Numpad 6
 * KEY_NUMPAD7 - Numpad 7
 * KEY_NUMPAD8 - Numpad 8
 * KEY_NUMPAD9 - Numpad 9
 */
enum {
	KEY_ESCAPE  = 0,
	KEY_ENTER   = 1,
	KEY_SPACE   = 32,

	KEY_INSERT  = 2,
	KEY_HOME    = 3,
	KEY_PGUP    = 4,
	KEY_DELETE  = 5,
	KEY_END     = 6,
	KEY_PGDOWN  = 7,

	KEY_UP      = 14,
	KEY_DOWN    = 15,
	KEY_LEFT    = 16,
	KEY_RIGHT   = 17,

	KEY_F1      = 18,
	KEY_F2      = 19,
	KEY_F3      = 20,
	KEY_F4      = 21,
	KEY_F5      = 22,
	KEY_F6      = 23,
	KEY_F7      = 24,
	KEY_F8      = 25,
	KEY_F9      = 26,
	KEY_F10     = 27,
	KEY_F11     = 28,
	KEY_F12     = 29,

	KEY_NUMDEL  = 30,
	KEY_NUMPAD0 = 31,
	KEY_NUMPAD1 = 127,
	KEY_NUMPAD2 = 128,
	KEY_NUMPAD3 = 129,
	KEY_NUMPAD4 = 130,
	KEY_NUMPAD5 = 131,
	KEY_NUMPAD6 = 132,
	KEY_NUMPAD7 = 133,
	KEY_NUMPAD8 = 134,
	KEY_NUMPAD9 = 135
};

/// Function: getkey
/// Reads a key press (blocking) and returns a key code.
///
/// See <Key codes for keyhit()>
///
/// Note:
/// Only Arrows, Esc, Enter and Space are currently working properly.
RLUTIL_INLINE int getkey(void) {
	#ifndef _WIN32
	int cnt = kbhit(); // for ANSI escapes processing
	#endif
	int k = getch();
	switch(k) {
		case 0: {
			int kk;
			switch (kk = getch()) {
				case 71: return KEY_NUMPAD7;
				case 72: return KEY_NUMPAD8;
				case 73: return KEY_NUMPAD9;
				case 75: return KEY_NUMPAD4;
				case 77: return KEY_NUMPAD6;
				case 79: return KEY_NUMPAD1;
				case 80: return KEY_NUMPAD2;
				case 81: return KEY_NUMPAD3;
				case 82: return KEY_NUMPAD0;
				case 83: return KEY_NUMDEL;
				default: return kk-59+KEY_F1; // Function keys
			}}
		case 224: {
			int kk;
			switch (kk = getch()) {
				case 71: return KEY_HOME;
				case 72: return KEY_UP;
				case 73: return KEY_PGUP;
				case 75: return KEY_LEFT;
				case 77: return KEY_RIGHT;
				case 79: return KEY_END;
				case 80: return KEY_DOWN;
				case 81: return KEY_PGDOWN;
				case 82: return KEY_INSERT;
				case 83: return KEY_DELETE;
				default: return kk-123+KEY_F1; // Function keys
			}}
		case 13: return KEY_ENTER;
#ifdef _WIN32
		case 27: return KEY_ESCAPE;
#else // _WIN32
		case 155: // single-character CSI
		case 27: {
			// Process ANSI escape sequences
			if (cnt >= 3 && getch() == '[') {
				switch (k = getch()) {
					case 'A': return KEY_UP;
					case 'B': return KEY_DOWN;
					case 'C': return KEY_RIGHT;
					case 'D': return KEY_LEFT;
				}
			} else return KEY_ESCAPE;
		}
#endif // _WIN32
		default: return k;
	}
}

/// Function: nb_getch
/// Non-blocking getch(). Returns 0 if no key was pressed.
RLUTIL_INLINE int nb_getch(void) {
	if (kbhit()) return getch();
	else return 0;
}

/// Function: getANSIColor
/// Return ANSI color escape sequence for specified number 0-15.
///
/// See <Color Codes>
RLUTIL_INLINE RLUTIL_STRING_T getANSIColor(const int c) {
	switch (c) {
		case BLACK       : return ANSI_BLACK;
		case BLUE        : return ANSI_BLUE; // non-ANSI
		case GREEN       : return ANSI_GREEN;
		case CYAN        : return ANSI_CYAN; // non-ANSI
		case RED         : return ANSI_RED; // non-ANSI
		case MAGENTA     : return ANSI_MAGENTA;
		case BROWN       : return ANSI_BROWN;
		case GREY        : return ANSI_GREY;
		case DARKGREY    : return ANSI_DARKGREY;
		case LIGHTBLUE   : return ANSI_LIGHTBLUE; // non-ANSI
		case LIGHTGREEN  : return ANSI_LIGHTGREEN;
		case LIGHTCYAN   : return ANSI_LIGHTCYAN; // non-ANSI;
		case LIGHTRED    : return ANSI_LIGHTRED; // non-ANSI;
		case LIGHTMAGENTA: return ANSI_LIGHTMAGENTA;
		case YELLOW      : return ANSI_YELLOW; // non-ANSI
		case WHITE       : return ANSI_WHITE;
		default: return "";
	}
}

/// Function: getANSIBackgroundColor
/// Return ANSI background color escape sequence for specified number 0-15.
///
/// See <Color Codes>
RLUTIL_INLINE RLUTIL_STRING_T getANSIBackgroundColor(const int c) {
	switch (c) {
		case BLACK  : return ANSI_BACKGROUND_BLACK;
		case BLUE   : return ANSI_BACKGROUND_BLUE;
		case GREEN  : return ANSI_BACKGROUND_GREEN;
		case CYAN   : return ANSI_BACKGROUND_CYAN;
		case RED    : return ANSI_BACKGROUND_RED;
		case MAGENTA: return ANSI_BACKGROUND_MAGENTA;
		case BROWN  : return ANSI_BACKGROUND_YELLOW;
		case GREY   : return ANSI_BACKGROUND_WHITE;
		default: return "";
	}
}

/// Function: setColor
/// Change color specified by number (Windows / QBasic colors).
/// Don't change the background color
///
/// See <Color Codes>
RLUTIL_INLINE void setColor(int c) {
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(hConsole, &csbi);

	SetConsoleTextAttribute(hConsole, (csbi.wAttributes & 0xFFF0) | (WORD)c); // Foreground colors take up the least significant byte
#else
	RLUTIL_PRINT(getANSIColor(c));
#endif
}

/// Function: setBackgroundColor
/// Change background color specified by number (Windows / QBasic colors).
/// Don't change the foreground color
///
/// See <Color Codes>
RLUTIL_INLINE void setBackgroundColor(int c) {
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(hConsole, &csbi);

	SetConsoleTextAttribute(hConsole, (csbi.wAttributes & 0xFF0F) | (((WORD)c) << 4)); // Background colors take up the second-least significant byte
#else
	RLUTIL_PRINT(getANSIBackgroundColor(c));
#endif
}

/// Function: saveDefaultColor
/// Call once to preserve colors for use in resetColor()
/// on Windows without ANSI, no-op otherwise
///
/// See <Color Codes>
/// See <resetColor>
RLUTIL_INLINE int saveDefaultColor() {
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	static char initialized = 0; // bool
	static WORD attributes;

	if (!initialized) {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		attributes = csbi.wAttributes;
		initialized = 1;
	}
	return (int)attributes;
#else
	return -1;
#endif
}

/// Function: resetColor
/// Reset color to default
/// Requires a call to saveDefaultColor() to set the defaults
///
/// See <Color Codes>
/// See <setColor>
/// See <saveDefaultColor>
RLUTIL_INLINE void resetColor() {
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)saveDefaultColor());
#else
	RLUTIL_PRINT(ANSI_ATTRIBUTE_RESET);
#endif
}

/// Function: cls
/// Clears screen, resets all attributes and moves cursor home.
RLUTIL_INLINE void cls(void) {
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	// Based on https://msdn.microsoft.com/en-us/library/windows/desktop/ms682022%28v=vs.85%29.aspx
	const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	const COORD coordScreen = {0, 0};
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(hConsole, &csbi);
	const DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);

	GetConsoleScreenBufferInfo(hConsole, &csbi);
	FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);

	SetConsoleCursorPosition(hConsole, coordScreen);
#else
	RLUTIL_PRINT(ANSI_CLS);
	RLUTIL_PRINT(ANSI_CURSOR_HOME);
#endif
}

/// Function: locate
/// Sets the cursor position to 1-based x,y.
RLUTIL_INLINE void locate(int x, int y) {
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	COORD coord;
	// TODO: clamping/assert for x/y <= 0?
	coord.X = (SHORT)(x - 1);
	coord.Y = (SHORT)(y - 1); // Windows uses 0-based coordinates
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else // _WIN32 || USE_ANSI
	#ifdef __cplusplus
		RLUTIL_PRINT("\033[" << y << ";" << x << "H");
	#else // __cplusplus
		char buf[32];
		sprintf(buf, "\033[%d;%df", y, x);
		RLUTIL_PRINT(buf);
	#endif // __cplusplus
#endif // _WIN32 || USE_ANSI
}

/// Function: setString
/// Prints the supplied string without advancing the cursor
#ifdef __cplusplus
RLUTIL_INLINE void setString(const RLUTIL_STRING_T & str_) {
	const char * const str = str_.data();
	unsigned int len = str_.size();
#else // __cplusplus
RLUTIL_INLINE void setString(RLUTIL_STRING_T str) {
	unsigned int len = strlen(str);
#endif // __cplusplus
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD numberOfCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
	WriteConsoleOutputCharacter(hConsoleOutput, str, len, csbi.dwCursorPosition, &numberOfCharsWritten);
#else // _WIN32 || USE_ANSI
	RLUTIL_PRINT(str);
	#ifdef __cplusplus
		RLUTIL_PRINT("\033[" << len << 'D');
	#else // __cplusplus
		char buf[3 + 20 + 1]; // 20 = max length of 64-bit unsigned int when printed as dec
		sprintf(buf, "\033[%uD", len);
		RLUTIL_PRINT(buf);
	#endif // __cplusplus
#endif // _WIN32 || USE_ANSI
}

/// Function: setChar
/// Sets the character at the cursor without advancing the cursor
RLUTIL_INLINE void setChar(char ch) {
	const char buf[] = {ch, 0};
	setString(buf);
}

/// Function: setCursorVisibility
/// Shows/hides the cursor.
RLUTIL_INLINE void setCursorVisibility(char visible) {
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	HANDLE hConsoleOutput = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_CURSOR_INFO structCursorInfo;
	GetConsoleCursorInfo( hConsoleOutput, &structCursorInfo ); // Get current cursor size
	structCursorInfo.bVisible = (visible ? TRUE : FALSE);
	SetConsoleCursorInfo( hConsoleOutput, &structCursorInfo );
#else // _WIN32 || USE_ANSI
	RLUTIL_PRINT((visible ? ANSI_CURSOR_SHOW : ANSI_CURSOR_HIDE));
#endif // _WIN32 || USE_ANSI
}

/// Function: hidecursor
/// Hides the cursor.
RLUTIL_INLINE void hidecursor(void) {
	setCursorVisibility(0);
}

/// Function: showcursor
/// Shows the cursor.
RLUTIL_INLINE void showcursor(void) {
	setCursorVisibility(1);
}

/// Function: msleep
/// Waits given number of milliseconds before continuing.
RLUTIL_INLINE void msleep(unsigned int ms) {
#ifdef _WIN32
	Sleep(ms);
#else
	// usleep argument must be under 1 000 000
	if (ms > 1000) sleep(ms/1000000);
	usleep((ms % 1000000) * 1000);
#endif
}

/// Function: trows
/// Get the number of rows in the terminal window or -1 on error.
RLUTIL_INLINE int trows(void) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return -1;
	else
		return csbi.srWindow.Bottom - csbi.srWindow.Top + 1; // Window height
		// return csbi.dwSize.Y; // Buffer height
#else
#ifdef TIOCGSIZE
	struct ttysize ts;
	ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
	return ts.ts_lines;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	return ts.ws_row;
#else // TIOCGSIZE
	return -1;
#endif // TIOCGSIZE
#endif // _WIN32
}

/// Function: tcols
/// Get the number of columns in the terminal window or -1 on error.
RLUTIL_INLINE int tcols(void) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return -1;
	else
		return csbi.srWindow.Right - csbi.srWindow.Left + 1; // Window width
		// return csbi.dwSize.X; // Buffer width
#else
#ifdef TIOCGSIZE
	struct ttysize ts;
	ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
	return ts.ts_cols;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	return ts.ws_col;
#else // TIOCGSIZE
	return -1;
#endif // TIOCGSIZE
#endif // _WIN32
}

/// Function: anykey
/// Waits until a key is pressed.
/// In C++, it either takes no arguments
/// or a template-type-argument-deduced
/// argument.
/// In C, it takes a const char* representing
/// the message to be displayed, or NULL
/// for no message.
#ifdef __cplusplus
RLUTIL_INLINE void anykey() {
	getch();
}

template <class T> void anykey(const T& msg) {
	RLUTIL_PRINT(msg);
#else
RLUTIL_INLINE void anykey(RLUTIL_STRING_T msg) {
	if (msg)
		RLUTIL_PRINT(msg);
#endif // __cplusplus
	getch();
}

RLUTIL_INLINE void setConsoleTitle(RLUTIL_STRING_T title) {
	const char * true_title =
#ifdef __cplusplus
		title.c_str();
#else // __cplusplus
		title;
#endif // __cplusplus
#if defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
	SetConsoleTitleA(true_title);
#else
	RLUTIL_PRINT(ANSI_CONSOLE_TITLE_PRE);
	RLUTIL_PRINT(true_title);
	RLUTIL_PRINT(ANSI_CONSOLE_TITLE_POST);
#endif // defined(_WIN32) && !defined(RLUTIL_USE_ANSI)
}

// Classes are here at the end so that documentation is pretty.

#ifdef __cplusplus
/// Class: CursorHider
/// RAII OOP wrapper for <rlutil.hidecursor>.
/// Hides the cursor and shows it again
/// when the object goes out of scope.
struct CursorHider {
	CursorHider() { hidecursor(); }
	~CursorHider() { showcursor(); }
};

} // namespace rlutil
#endif