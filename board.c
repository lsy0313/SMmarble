#include <stdio.h>
#include <stdlib.h> 
#include "board.h"

#define N_COINPOS      12
#define MAX_COIN       4


static int board_status[N_BOARD];
static int board_coin[N_BOARD];
static int board_SharkPosition;


// 보드 초기화  
int board_initBoard(void)
{
    int i; 
    for (i = 0; i < N_BOARD; i++)
    {
        board_status[i] = BOARDSTATUS_OK;
        board_coin[i] = 0;
    }
    
    for (i = 0; i < N_COINPOS; i++)
    {
        int flag = 0;
        while(flag == 0)
        {
               int allocPos = rand()%N_BOARD;
               if (board_coin[allocPos] == 0)
               {
                    board_coin[allocPos] = rand()%MAX_COIN + 1;
                    flag = 1;
               }
        }
    }
    return 0;
}

// 보드 출력  
int board_printBoardStatus(void)
{
    int i;
    
    printf("--------------------------BOARD STATUS--------------------------\n");
    for (i = 0; i < N_BOARD; i++)
    {
        printf("|");
        // 상어한테 먹혔으면  
        if (board_status[i] == BOARDSTATUS_NOK)
           printf("X");
        // 아직 안 먹혔으면  
        else
            printf("O");
        }
    printf("|\n");
    printf("----------------------------------------------------------------\n");
    
    return 0;
}

int board_getBoardStatus(int pos)
{
    return board_status[pos];
}

// 코인 획득  
int board_getBoardCoin(int pos)
{
    int coin = board_coin[pos];
    board_coin[pos] = 0;
    return coin;
}

// 상어 이동  
int board_stepShark(void) 
{
    int step = rand() % MAX_SHARKSTEP + 1;
    int shark_position = SHARK_INITPOS;
    int i;
    for (i += 1; i <= shark_position + step; i++)
    {
        if (i >= 0 && i < N_BOARD)
           board_status[i] = BOARDSTATUS_NOK;
    }
    
    shark_position += step;
    
    return shark_position; 
}

// player_status == LIVE / DIE 체크하는 부분 만들기  
