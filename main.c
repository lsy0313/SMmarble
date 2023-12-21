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
    int flag_graduate;
} player_t;

static player_t* cur_player;
// static player_t cur_[MAX_PLAYER];


#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
#endif


//function prototypes
#if 0
int isGraduated(void); //check if any player is graduated
void generatePlayers(int n, int initEnergy); //generate a new player
void printGrades(int player); //print grade history of the player
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char* lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char* lectureName); //find the grade from the player's grade history
void printGrades(int player); //print all the grade history of the player
#endif


// 성적 확인 
void printGrades(int player)
{
    int i;
    void* gradePtr;
    for (i = 0; i < smmdb_len(LISTNO_OFFSET_GRADE + player); i)
    {
        gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
        printf("%s : %i\n", smmObj_getNodeName(gradePtr), smmObj_getNodeGrade(gradePtr));
    }
}


// 플레이어 상태 (credit, energy, position) 
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

// 새로운 플레이어 생성 (이름 입력받기, 위치 및 에너지 할당) 
void generatePlayers(int n, int initEnergy) // generate a new player
{
    int i;
    //n time loop 
    for (i = 0; i < n; i++)
    {
        // input name
        printf("Player %d's name: ", i + 1); // 안내 문구  
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

// 주사위 굴리기  
int rolldie(int player)
{
    char c;
    int die_result;
    printf(" [ %s's turn ] Press any key to roll a die (press g to see grade): ", cur_player[player].name);
    c = getchar();
    fflush(stdin);
    
    return (rand() % MAX_DIE + 1); // 6 이하 랜덤 정수
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
             printf("\n::: Lecture Node : gained %d credits :::\n", smmObj_getNodeCredit(boardPtr));
             cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
             cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr);

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

int readFoodCard() {
    FILE* fp = fopen(FOODFILEPATH, "r");;
    char name[MAX_CHARNAME]; 
    int energy;
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
    if (fscanf(fp, "%s %d", name, &energy) == 2) {
       printf("You ate %s~! Gained %d energy\n", name, energy); 
       return energy;
       }
    else {
         printf("Error reading data from the file.\n");
         return 0;
         }
    fclose(fp);
}

int readFestivalCard() {
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

// 이동  
void goForward(int player, int step)
{
    void* boardPtr;
    cur_player[player].position += step;
    boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    printf(" %s go to node %i (name: %s)\n", cur_player[player].name, cur_player[player].position, smmObj_getNodeName(boardPtr));
}


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
        smmdb_addTail(LISTNO_NODE, foodObj);
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
        smmdb_addTail(LISTNO_NODE, festivalObj);
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
     //is anybody graduated?
    while(1) {
          
          int die_result;
      
        //4-1. initial printing
        printPlayerStatus();

        //4-2. die rolling (if not in experiment)
        if (cur_player[turn].inExperiment != 1) {
           die_result = rolldie(turn);
        
        //4-3. go forward
           goForward(turn, die_result);
           
        //4-4. take action at the destination node of the board
           actionNode(turn);
        }
        else actionNode(turn);
        
        //4-5. next turn
        turn = (turn + 1) % player_nr;
     } 
  
    free(cur_player);
    
    system("PAUSE"); 
    return 0;
}

   
