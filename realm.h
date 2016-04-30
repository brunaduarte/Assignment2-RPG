// realm.h
// Some game constants
#define MAP_WIDTH 30
#define MAP_HEIGHT 20
#define MAX_NAME_LEN 20
#define MAX_WEAPONS 4

#include<stdio.h>
#include<windows.h>

typedef unsigned char byte;
typedef struct {
	byte map[MAP_HEIGHT][MAP_WIDTH];
	
} tRealm;
typedef struct {
	char name[MAX_NAME_LEN+1];
	int health;	
	int strength;
	byte stamina;
	byte mana;
	int defense;
	int intelligence;
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
void initRealm(tRealm *Realm);
void showRealm(tRealm *Realm,tPlayer *thePlayer);
void SaveRealm(tRealm *Realm);
void LoadRealm(tRealm *Realm);
void initPlayer(tPlayer *Player,tRealm *Realm);
void showPlayer(tPlayer *thePlayer);
void SavePlayer(tPlayer *thePlayer);
void LoadPlayer(tPlayer *thePlayer);
void step(char Direction,tPlayer *Player,tRealm *Realm);
void setHealth(tPlayer *Player, int health);
void setStrength(tPlayer *Player, int strength);
void setDefense(tPlayer *Player, int defense);
void setIntelligence(tPlayer *Player, int intelligence);
int addWeapon(tPlayer *Player, int Weapon);
int doChallenge(tPlayer *Player, int BadGuyIndex);
const char *getWeaponName(int index);
void zap(void);
//NEW
void titleScreen(void);
void printBorder(int,int,COORD,int);

