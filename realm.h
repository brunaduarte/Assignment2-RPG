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