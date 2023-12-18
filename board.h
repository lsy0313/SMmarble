
#define BOARDSTATUS_OK  1
#define BOARDSTATUS_NOK 0

#define N_BOARD        15


#define MAX_SHARKSTEP       6 
#define SHARK_INITPOS       -4

#define MAX_DIE   6

#define PLAYERSTATUS_LIVE   0
#define PLAYERSTATUS_DIE    1
#define PLAYERSTATUS_END    2


int board_initBoard(void);
int board_printBoardStatus(void);

int board_getBoardCoin(int pos);
int board_getSharkPosition(void);
