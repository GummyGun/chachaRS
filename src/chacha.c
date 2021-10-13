#include <stdio.h>
#include <sodium.h>
#include <string.h>
#include <errno.h>
#include "chacha.h"

static void chachaAlgotithm(uint32_t *matrixParam, _Bool side);
static inline uint32_t addmod(uint32_t number1, uint32_t number2);
static inline uint32_t bitShift(uint32_t target, uint8_t bits);

//prints bin data correctly to stdout 
void 
printAsChars(uint32_t number, int8_t charNum){
    char *tmp=(char*)(void*)&number;
    for(int8_t iter=0; iter<charNum; iter++){
        printf("%c",tmp[iter]);
    }
    
}

//debug function
void 
printUint(uint32_t key[], int8_t size){
    for(int32_t iter=0; iter<size; iter++){
        printf("%8x ", key[iter]);
    }
    printf("\n");
}

//fusses data into matrix
void 
genMat(uint32_t *matrixParam, uint32_t *key, uint32_t nuo[3]){
    memcpy((void*)&matrixParam[4], key, 32);
    memset((void*)&matrixParam[12], 0, 4);
    memcpy((void*)&matrixParam[13], nuo, 12);
}

//shuffles matrix 
uint32_t*
shuffleMatrix(uint32_t *matrixParam, uint32_t block){
    uint32_t *matrix=(uint32_t*)malloc(64);
    memcpy((void*)matrix, (void*)matrixParam, 64);
    matrix[12]=block;
    
    
    //printUint(matrix, 16);
    for(int8_t iter=0; iter<20; iter++){
        if(iter%2){
            chachaAlgotithm(matrix,1);
        }else{
            chachaAlgotithm(matrix,0);
        }
    }
    //printUint(matrix, 16);
    return matrix;
}

//runs algorithm chacha20 side decides whether it is row or column
static 
void 
chachaAlgotithm(uint32_t *matrixParam, _Bool side){
    uint32_t *a, *b, *c, *d;
    if(side){
        a=&matrixParam[0];
        b=&matrixParam[4];
        c=&matrixParam[8];
        d=&matrixParam[12];
    }else{
        a=&matrixParam[0];
        b=&matrixParam[5];
        c=&matrixParam[10];
        d=&matrixParam[15];
    }
    for(int8_t iter=0; iter<4; iter++){
        //printf("a-pre%x\nb-pre%x\nc-pre%x\nd-pre%x\n", *a, *b, *c, *d);
        *a=addmod(*b,*a);
        *d=*d^*a;
        *d=bitShift(*d, 16);
        *c=addmod(*d, *c);
        *b=*b^*c;
        *b=bitShift(*b, 12);
        *a=addmod(*b, *a);
        *a=*a^*d;
        *d=bitShift(*d, 8);
        *c=addmod(*d, *c);
        *b=*c^*b;
        //printf("a-post%x\nb-post%x\nc-post%x\nd-post%x\n", *a, *b, *c, *d);
        if(side){
            a++;
            b++;
            c++;
            d++;
        }else{
            a=&matrixParam[iter];
            b=&matrixParam[4+(1+iter)%4];
            c=&matrixParam[8+(2+iter)%4];
            d=&matrixParam[12+(3+iter)%4];
        }
    }
    //printUint(matrixParam, 16);
}

static inline
uint32_t
bitShift(uint32_t target, uint8_t bits){
    return (target<<bits)|(target>>(32-bits));
}

static inline
uint32_t
addmod(uint32_t number1, uint32_t number2){
    return number1+number2;
}
