//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//
#define _CRT_SECURE_NO_WARNINGS

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"
#include "board.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"

typedef struct player {
    int energy;
    int position;
    char name[MAX_CHARNAME];
    int accumCredit;
    int flag_graduate;
} player_t;


//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;

static player_t* cur_player;
// static player_t cur_[MAX_PLAYER];


int* player_position;
char(*player_name)[MAX_CHARNAME];
int* player_coin;
int* player_status;
char player_statusString[3][MAX_CHARNAME] = { "LIVE", "DIE", "END" };


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

void opening()
{
    printf("****************************************************************\n");
    printf("***==========================================================***\n");
    printf("*****               ~~~~~~ SHARK GAME ~~~~~~               *****\n");
    printf("***==========================================================***\n");
    printf("****************************************************************\n");
}

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

// 플레이어 위치  
void printPlayerPosition(int player)
{
    int i;

    for (i = 0; i < N_BOARD; i++)
    {
        printf("|");
        if (player_position[player] == i)
            printf("%c", player_name[player][0]);
        else
            printf(" ");
    } printf("|\n");
}


// 플레이어 상태 (credit, energy, position) 
void printPlayerStatus(void)
{
    int i;

    printf("player status ---------------------------------------- \n");
    for (i = 0;i < player_nr;i++)
    {
        printf("%s : credit %i, energy %i, position %i\n",
            cur_player[i].name,
            cur_player[i].accumCredit,
            cur_player[i].energy,
            cur_player[i].position);

        printf("%s : pos %i, coin %i, status %s\n", player_name[i], player_position[i], player_coin[i], player_statusString[player_status[i]]);
        printPlayerPosition(i);
    } printf("------------------------------------------------------\n");
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
        cur_player[i].flag_graduate = 0;
    }
}

// 주사위 굴리기  
int rolldie(int player)
{
    char c;
    int die_result;
    printf("**************************************************************\n");
    printf(" Press any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);


    die_result = (rand() % MAX_DIE + 1);
    printf("::: Die result: %d :::\n", die_result);

    return die_result; // 6 이하 랜덤 정수
}


//action code when a player stays at a node
void actionNode(int player)
{
    void* boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    int type = smmObj_getNodeType(boardPtr);
    char* name = (char*)smmObj_getNodeName(boardPtr);
    void* gradePtr;

    switch (type)
    {
        //case lecture:
    case SMMNODE_TYPE_LECTURE:

        cur_player[player].accumCredit += smmObj_getNodeCredit(boardPtr);
        cur_player[player].energy -= smmObj_getNodeEnergy(boardPtr);

        // grade generation
        gradePtr = (void*)smmObj_genObject(name, smmObjType_grade, 0, smmObj_getNodeCredit(boardPtr), 0, 0);
        smmdb_addTail(LISTNO_OFFSET_GRADE + player, gradePtr);
        break;

    default:
        break;
    }
}


void goForward(int player, int step)
{
    void* boardPtr;
    cur_player[player].position += step;
    boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    printf("%s go to node %i (name: %s)\n", cur_player[player].name, cur_player[player].position, smmObj_getNodeName(boardPtr));
}

// 게임 종료  
int game_end(void)
{
    int i;
    int flag_end = 1;

    for (i = 0; i < player_nr; i++)
    {
        if (player_status[i] == PLAYERSTATUS_LIVE)
        {
            flag_end = 0; break;
        }
    }
    return flag_end;
}

// 살아있는 플레이어 표시 
int getAlivePlayer(void)
{
    int i;
    int cnt = 0;
    for (i = 0; i < player_nr; i++)
    {
        if (player_status[i] == PLAYERSTATUS_END)
            cnt++;
    }
    return cnt;
}

// 죽은 플레이어 표시  
void checkDie(void)
{
    int i;
    for (i = 0; i < player_nr; i++)
        if (board_getBoardStatus(player_position[i]) == BOARDSTATUS_NOK)
            player_status[i] = PLAYERSTATUS_DIE;
}

// 승자 구하기  
int getWinner(void)
{
    int i;
    int winner = 0;
    int max_coin = -1;

    for (i = 0; i < player_nr; i++)
    {
        if (player_coin[i] > max_coin)
        {
            max_coin = player_coin[i];
            winner = i;
        }
    }
    return winner;
}



int main(int argc, const char* argv[]) {

    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy;

    player_position = (int*)malloc(player_nr * sizeof(int));
    player_name = (char(*)[MAX_CHARNAME])malloc(player_nr * MAX_CHARNAME * sizeof(char));
    player_coin = (int*)malloc(player_nr * sizeof(int));
    player_status = (int*)malloc(player_nr * sizeof(int));

    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;

    int pos = 0;
    int turn = 0;

    srand(time(NULL));

    // 0. opening
    opening();

    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH, "r")) == NULL)  // 예외 처리  
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }

    printf("Reading board component......\n");
    while (fscanf(fp, "%s %i %i %i", &name, &type, &credit, &energy) == 4) //read a node parameter set
    {
        //store the parameter set
        void* boardObj = smmObj_genObject(name, smmObjType_board, type, credit, energy, 0);
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

    board_initBoard();

#if 0
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
    while () //read a festival card string
    {
        //store the parameter set
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);
#endif


    //2. Player configuration ---------------------------------------------------------------------------------

    do
    {
        //input player number to player_nr
        printf("Input player number: ");
        scanf("%d", &player_nr);
        fflush(stdin);
    } while (player_nr < 0 || player_nr > MAX_PLAYER);

    //플레이어 수만큼 메모리 할당 
    cur_player = (player_t*)malloc(player_nr * sizeof(player_t));
    generatePlayers(player_nr, initEnergy);



    //3. SM Marble game starts ---------------------------------------------------------------------------------
    do //is anybody graduated?
    {
        int step = 0;
        int coinResult;
        char c;
        int die_result;

        //4-1. initial printing
        if (player_status[turn] != PLAYERSTATUS_LIVE)
        {
            turn = (turn + 1) % player_nr; continue;
        }

        board_printBoardStatus();
        for (i = 0; i < player_nr; i++)
            printPlayerPosition(i);
        printPlayerStatus();

        //4-2. die rolling (if not in experiment)
        board_printBoardStatus();

        for (i = 0; i < player_nr; i++)
            printPlayerPosition(i);
        printPlayerStatus();
        die_result = rolldie(turn);

        printf("%s turn!! ", player_name[turn]);
        printf("Press any key to roll a die! \n");
        scanf("%d", &c);
        fflush(stdin);

        goForward(turn, die_result);

        //4-3. go forward
        player_position[turn] += step;
        if (player_position[turn] >= N_BOARD)
            player_position[turn] = N_BOARD - 1;

        if (player_position[turn] == N_BOARD - 1)
            player_status[turn] = PLAYERSTATUS_END;
        printf("***Die result : %d, %s moved to %d!***\n", step, player_name[turn], player_position[turn]);

        //4-4. take action at the destination node of the board
        actionNode(turn);
        coinResult = board_getBoardCoin(pos);
        player_coin[turn] += coinResult;
        printf("You gained %s coin!\n", coinResult);

        //4-5. next turn
        turn = (turn + 1) % player_nr;
    }while (game_end() == 0);
    free(cur_player);

    free(player_position);
    free(player_name);
    free(player_coin);
    free(player_status);

    system("PAUSE");
    return 0;
}


