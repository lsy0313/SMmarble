//
//  smm_object.h
//  SMMarble object
//
//  Created by Juyeop Kim on 2023/11/05.
//

#ifndef smm_object_h
#define smm_object_h

#define SMMNODE_TYPE_LECTURE 0
#define SMMNODE_TYPE_RESTAURANT 1
#define SMMNODE_TYPE_LABORATORY 2
#define SMMNODE_TYPE_HOME 3
#define SMMNODE_TYPE_EXPERIMENT 4
#define SMMNODE_TYPE_FOODCHANCE 5
#define SMMNODE_TYPE_FESTIVAL 6

#define SMMNODE_TYPE_MAX 7


typedef enum smmObjType {
    smmObjType_board = 0,
    smmObjType_card,
    smmObjType_grade
} smmObjType_e;

typedef enum smmObjGrade {
    smmObjGrade_Ap = 0,
    smmObjGrade_A0,
    smmObjGrade_Am,
    smmObjGrade_Bp,
    smmObjGrade_B0,
    smmObjGrade_Bm,
    smmObjGrade_Cp,
    smmObjGrade_C0,
    smmObjGrade_Cm
} smmObjGrade_e;

/* node type :
    lecture,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
*/


/* grade :
    A+,
    A0,
    A-,
    B+,
    B0,
    B-,
    C+,
    C0,
    C-
*/



//object generation
int smmObj_genNode(char* name, int type, int credit, int energy);

//member retrieving


//element to string



#endif /* smm_object_h */