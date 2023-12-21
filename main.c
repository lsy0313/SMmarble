//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"


//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;


typedef struct player {
    int energy;
    int position;
    char name[MAX_CHARNAME];
    int accumCredit;
    int inExperiment; // is in experiment?
    int lectureTaken;
    char* grade;
    int flag_graduate;
} player_t;

static player_t* cur_player;


//function prototypes

int isGraduated(void); //check if any player is graduated
void generatePlayers(int n, int initEnergy); // generate a new player
char* getRandomGrade(void);
void takeLecture(int player); //take the lecture (insert a grade of the player)
void printGrades(int player); //print grade history of the player
void goForward(int player, int step, int initEnergy); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
int rolldie(int player);
void actionNode(int player);
int readFoodCard(void);
int readFestivalCard(void);
#if 0
float calcAverageGrade(int player); //calculate average grade of the player
void* findGrade(int player, char* lectureName); //find the grade from the player's grade history
#endif


int main(int argc, const char* argv[]) {

    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy;
    int turn = 0;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;

    srand(time(NULL));

    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH, "r")) == NULL)  // 예외 처리  
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }

    printf("Reading board component......\n");
    while (fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4) //read a node parameter set
    {
        //store the parameter set
        void* boardObj = (void*)smmObj_genObject(name, smmObjType_board, type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj);

        if (type == SMMNODE_TYPE_HOME)
            initEnergy = energy;
        board_nr++;
    }
    fclose(fp);
    printf("Total number of board nodes : %i\n", board_nr);
    printf("**************************************************************\n");

    for (i = 0;i < board_nr;i++)
    {
        void* boardObj = smmdb_getData(LISTNO_NODE, i);

        printf("node %i : %s, %i(%s), credit %i, energy %i\n",
            i, smmObj_getNodeName(boardObj),
            smmObj_getNodeType(boardObj), smmObj_getTypeName(smmObj_getNodeType(boardObj)),
            smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
    }


    //2. food card config 
    if ((fp = fopen(FOODFILEPATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }

    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %i", &name, &energy) == 2) //read a food parameter set
    {
        //store the parameter set
        void* foodObj = (void*)smmObj_genObject(name, smmObjType_card, type, credit, energy, 0);
        smmdb_addTail(LISTNO_FOODCARD, foodObj);
        food_nr++;
    }
    fclose(fp);
    printf("Total number of food cards : %i\n", food_nr);


    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }

    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", &name) == 1) //read a festival card string
    {
        void* festivalObj = (void*)smmObj_genObject(name, smmObjType_card, type, credit, energy, 0);
        smmdb_addTail(LISTNO_FESTCARD, festivalObj);
        festival_nr++;
        //store the parameter set
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);



    //2. Player configuration ---------------------------------------------------------------------------------
    int input;
    
    do
    {
        //input player number to player_nr
        printf("\n");
        printf("Input player number: ");
        input = scanf("%d", &player_nr);
        fflush(stdin);
    } while (player_nr < 0 || player_nr > MAX_PLAYER || input == 0);

    //플레이어 수만큼 메모리 할당 
    cur_player = (player_t*)malloc(player_nr * sizeof(player_t));
    generatePlayers(player_nr, initEnergy);



    //3. SM Marble game starts ---------------------------------------------------------------------------------
     
    while(1) {
          int die_result;
          int i;
                
        //4-1. initial printing
        printPlayerStatus();

        //4-2. die rolling (if not in experiment)
        if (cur_player[turn].inExperiment != 1) {
           die_result = rolldie(turn);
        
        //4-3. go forward
           goForward(turn, die_result, initEnergy);
           
           // is anybody graduated?
           if (isGraduated() == 0) break;
           
        //4-4. take action at the destination node of the board
           actionNode(turn);
        }
        else actionNode(turn);
        
        //4-5. next turn
        if (isGraduated() != 0) turn = (turn + 1) % player_nr;
     } 
  
    free(cur_player);
    
    system("PAUSE"); 
    return 0;
}

int isGraduated(void) {
    int i;
    
    for (i = 0; i < player_nr; i++) 
    {
        if (cur_player[i].flag_graduate)
           return 0;
        else 
           return 1;
    }
}
   
   
void generatePlayers(int n, int initEnergy) 
{
    int i;
    //n time loop 
    for (i = 0; i < n; i++)
    {
        // input name
        printf("Player %d's name: ", i + 1);   
        scanf("%s", cur_player[i].name);
        fflush(stdin);

        // set position
        cur_player[i].position = 0;

        // set energy
        cur_player[i].energy = initEnergy;
        cur_player[i].accumCredit = 0;
        cur_player[i].inExperiment = 0;
        cur_player[i].flag_graduate = 0;
    }
}
#if 0
 getRandomGrade(void) {
    smmObjGrade_e grades[] = {
        smmObjGrade_Ap, smmObjGrade_A0, smmObjGrade_Am,
        smmObjGrade_Bp, smmObjGrade_B0, smmObjGrade_Bm,
        smmObjGrade_Cp, smmObjGrade_C0, smmObjGrade_Cm
    };
    return grades[rand() % 9];
}
#endif

char* getRandomGrade() {
    char* grades[] = {"A+", "A0", "A-", "B+", "B0", "B-", "C+", "C0", "C-"};
    return grades[rand() % 9];
}


void takeLecture(int player)
{
    void* boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    int required_energy = smmObj_getNodeEnergy(boardPtr);
    
    // check having enough energy or haven't taken lecture
    if (cur_player[player].energy >= required_energy && !cur_player[player].lectureTaken) {
        
        int choice;
        
        printf("%s :: Credits: +%d, Energy : -%d ::\n", smmObj_getNodeName(boardPtr), smmObj_getNodeCredit(boardPtr), required_energy);
        printf("Do you want to take the lecture? (take: 1, drop: 2) ");
        scanf("%d", &choice);
        printf("\n");

        // take the lecture
        if (choice == 1) {  
            char* grade = getRandomGrade();// randomly return a grade
            printf("Your grade is: %s\n", grade);
            cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
            cur_player[player].energy -= required_energy;
            cur_player[player].lectureTaken = 1;
            cur_player[player].grade = grade;
            }
            
        // drop the lecture
        else if (choice == 2)   
            printf("You dropped %s.\n", smmObj_getNodeName(boardPtr));
        
        else 
         printf("Invalid choice.\n");
         }
    else 
        printf("You can't take this lecture.\n"); 
}

void printGrades(int player)
{
    int i;
    void* gradePtr;
    printf("[Grade Report] ");
    for (i = 0; i < smmdb_len(LISTNO_OFFSET_GRADE + player); i++)
    {
        #if 0
        gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        printf("%s : %i\n", smmObj_getNodeName(gradePtr), smmObj_getNodeGrade(gradePtr));
        #endif
        
        printf(" %s /", cur_player[player].grade);
    }
    printf("\n");
}

 
void goForward(int player, int step, int initEnergy)
{
    void* boardPtr;
    cur_player[player].position += step;
        
    // passing through Home Node
    if (cur_player[player].position > 15) {
       printf("\nNotice : %s passed through home. Gained %d energy\n\n", cur_player[player].name, initEnergy);
       cur_player[player].energy += initEnergy;   
       cur_player[player].position = cur_player[player].position - 15;
       
       // game ending
       if (cur_player[player].accumCredit >= GRADUATE_CREDIT) {
          printf("\n*************GAME OVER********************\n");
          printf("Congratulations!!! Player %s has graduated!\n", cur_player[player].name);
          cur_player[player].flag_graduate = 1; 
          }
       }                     
                                    
    boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    if (cur_player[player].flag_graduate != 1)
    printf(" %s go to node %i (name: %s)\n", cur_player[player].name, cur_player[player].position, smmObj_getNodeName(boardPtr));
}


void printPlayerStatus(void)
{
    int i;
    
    printf("\n");
    printf("player status ------------------------------------------- \n");
    for (i = 0;i < player_nr;i++)
    {
        printf("%s : credit %i, energy %i, position %i, in experiment: %s\n",
            cur_player[i].name,
            cur_player[i].accumCredit,
            cur_player[i].energy,
            cur_player[i].position,
            cur_player[i].inExperiment ? "Yes" : "No");

    } printf("---------------------------------------------------------\n");
    printf("**************************************************************\n");
}

 
int rolldie(int player)
{
    char c;
    int die_result;
    printf(" [ %s's turn ] Press any key to roll a die (press g to see grade): ", cur_player[player].name);
    c = getchar();
    if (c == 'g')
       printGrades(player);
    fflush(stdin);
    
    return (rand() % MAX_DIE + 1);
}

//action code when a player stays at a node
void actionNode(int player)
{
    void* boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    int type = smmObj_getNodeType(boardPtr);
    char* name = (char*)smmObj_getNodeName(boardPtr);
    void* gradePtr;
    int randomIndex;

    switch (type)
    {
        //case lecture:
        case SMMNODE_TYPE_LECTURE:
             printf("\n::: Lecture Node :::\n", smmObj_getNodeCredit(boardPtr));
             takeLecture(player);

             //grade generation;
             gradePtr = (void*)smmObj_genObject(name, smmObjType_grade, 0, smmObj_getNodeCredit(boardPtr), 0, 0);
             smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
             break;
        
        case SMMNODE_TYPE_RESTAURANT: 
             printf("\n::: Restaurant Node : Gained %d energy :::\n", smmObj_getNodeEnergy(boardPtr));
             cur_player[player].energy += smmObj_getNodeEnergy(boardPtr); 
             break;
             
        case SMMNODE_TYPE_LABORATORY: 
             // in experiment
             if (cur_player[player].inExperiment == 1) 
             {
                printf("\n::: Laboratory Node : Lost %d energy :::\n", smmObj_getNodeEnergy(boardPtr));
                cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr); 
                
                int dice = rolldie(player);
                int successThreshold = rand() % MAX_DIE + 1;
                // experiment success
                if (dice >= successThreshold) {   
                   printf("\n~~~Experiment Successed!~~~ (success threshold : %d < die result : %d)\n", successThreshold, dice); 
                   printf("Notice : You can move to the next node\n");
                   cur_player[player].inExperiment = 0; break;
                   }
                // experiment failed
                else {         
                   printf("\n~~~Experiment Failed!~~~ (success threshold : %d > die result : %d)\n", successThreshold, dice); 
                   printf("Notice : You can't move until the next turn\n");
                   cur_player[player].position = 8; break;
                   }     
             }
             // not in experiment
             else 
                  printf("\n::: Laboratory Node : You're not in experiment, so pass :::\n");
             break;
             
        case SMMNODE_TYPE_HOME: 
             printf("\n::: Home Node : Gained %d energy :::\n", smmObj_getNodeEnergy(boardPtr));
             cur_player[player].energy += smmObj_getNodeEnergy(boardPtr); 
             break;
             
        case SMMNODE_TYPE_EXPERIMENT: 
             printf("\n::: Experiment Node : Go to Laboratory Node :::\n", smmObj_getNodeEnergy(boardPtr));
             cur_player[player].inExperiment = 1;
             cur_player[player].position = 8; // move to Laboratory Node
             break;
             
        case SMMNODE_TYPE_FOODCHANCE:
             printf("\n::: Food Chance Node :::\n");
             cur_player[player].energy += readFoodCard();
             break;
             
        case SMMNODE_TYPE_FESTIVAL:
             printf("\n::: Festival Node :::\n");
             readFestivalCard();
             break;

    default:
        break;
    }
}


int readFoodCard(void) {
    FILE* fp = fopen(FOODFILEPATH, "r");;
    char food_name[MAX_CHARNAME]; 
    int food_energy;
    int i;
    // Create an index randomly
    int randomIndex = rand() % 14 + 1;
    
    // Move to randomIndex of file
    for (i = 0; i < randomIndex - 1; i++) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp) == NULL) {
            printf("Error reading file.\n");
            fclose(fp);
            return 1;
        }
    }
    // Read file
    if (fscanf(fp, "%s %d", food_name, &food_energy) == 2) {
       printf("You ate %s~! Gained %d energy\n", food_name, food_energy); 
       return food_energy;
       }
    else {
         printf("Error reading data from the file.\n");
         return 0;
         }
    fclose(fp);
}

int readFestivalCard(void) {
    FILE* fp = fopen(FESTFILEPATH, "r");
    char mission[MAX_CHARNAME]; 
    int i;
    
    // Create an index randomly
    int randomIndex = rand() % 5 + 1;
    
    // Move to randomIndex of file
    for (i = 0; i < randomIndex - 1; i++) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp) == NULL) {
            printf("Error reading file.\n");
            fclose(fp);
            return 1;
        }
    }
    // Read file
    if (fscanf(fp, "%s", mission) == 1) 
       printf("Mission: %s\n", mission); 
    else 
       printf("Error reading data from the file.\n");
         
    fclose(fp);
}
