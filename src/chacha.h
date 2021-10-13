#ifndef __GUM_CHACHA20__
#define __GUM_CHACHA20__ 1


void printUint(uint32_t key[], int8_t size);
uint32_t* shuffleMatrix(uint32_t *matrixParam, uint32_t block);
void genMat(uint32_t *matrixParam, uint32_t *key, uint32_t nuo[3]);
void printAsChars(uint32_t number, int8_t charNum);

enum OUTPUTMODE{
    std,
    file
};

enum OPENGEN{
    open,
    gen
};

#endif
