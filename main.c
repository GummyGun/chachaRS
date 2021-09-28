#include <stdio.h>
#include <sodium.h>
#include <string.h>
#include <errno.h>

void printUint(uint32_t key[], int8_t size);
uint32_t* shuffle(uint32_t *matrix, uint32_t block);
void shuffleSide(uint32_t *matrix, _Bool side);
void genMat(uint32_t *matrix, uint32_t *key, uint32_t nuo[3]);
static inline uint32_t addmod(uint32_t number1, uint32_t number2);
static inline uint32_t bitShift(uint32_t target, uint8_t bits);

enum MODE{
    encode,
    decode
};

int 
main(int argc, char *argv[]){
    uint32_t *matrix=(uint32_t*)malloc(512/8);
    _Bool openGenD=0;
    uint32_t key[8]={};
    _Bool modeD=0;
    enum MODE mode=encode;
    FILE *cargo;
    
    for(int8_t iter=1; iter<argc; iter++){
        if(argv[iter][0]=='-'){
            int8_t flags=strlen(argv[iter]);
            
            for(int8_t iterFlags=1; iterFlags<flags; iterFlags++){
                _Bool ctrl=0;
                switch(argv[iter][iterFlags]){
                    case 'k':{
                        if(openGenD){
                            return -1;
                        }else{
                            openGenD=1;
                        }
                        FILE *keyDoc;
                        
                        keyDoc=fopen(argv[iter++], "rb");
                        if(errno!=0){
                            perror("Error openening key");
                        }else{
                            fread(key, 4, 8, keyDoc);
                            fclose(keyDoc);
                            ctrl=1;
                            break;
                        }
                    }
                    case 'g':{
                        if(openGenD){
                            return -1;
                        }else{
                            openGenD=1;
                        }
                        randombytes_buf((void*)key, 32);
                        
                        break;
                    }
                    /*
                    case 'd':{
                        if(modeD){
                            return 2;
                        }else{
                            mode=decode;
                            modeD=1;
                        }
                        mode=decode;
                        break;
                    }
                    
                    case 'e':{
                        if(modeD){
                            return 2;
                        }else{
                            mode=encode;
                            modeD=1;
                        }
                        break;
                    }
                    */
                }
                if(ctrl){
                    continue;
                }
            }
        }else{
            printf("file to operate in %s\n", argv[iter]);
            cargo=fopen(argv[iter], "rb");
        }
    }
    strcpy((char*)(void*)matrix, "expand 32-byte k");
    if(!openGenD){
        //randombytes_buf((void*)key, 32);
    }
    
    uint32_t nuo[3]={0,0,0};
    
    //scanf("%d\n%d\n%d", nuo, nuo+1, nuo+2);
    
    genMat(matrix, key, nuo);
    uint32_t bn=0;
    //printUint(matrix, 16);
    uint32_t *shuffled=shuffle(matrix, 0);
    //printUint(shuffled, 16);
    
    FILE *keySav=fopen("key.key", "wb");
    fwrite(key, 4, 8, keySav);
    fclose(keySav);
    
    uint32_t buff=0;
    uint32_t encryptedBytes;
    
    fread(&buff, 1, 1, cargo);
    
    for(int64_t iter=0; 0; iter++){
        printf("no ha tronado\n");
        if(iter/16==1){
            free(shuffled);
            shuffled=shuffle(matrix, iter/16);
        }
        encryptedBytes=shuffled[iter++];
        printf("%s", encryptedBytes^buff);
        memset((void*)&buff, 0, 4);
    }
    printf("\n");
    free(shuffled);
    fclose(cargo);
    return 0;
}

void flagG(){
    
}

void flagD(){
    
}

void 
printUint(uint32_t key[], int8_t size){
    for(int32_t iter=0; iter<size; iter++){
        printf("%8x ", key[iter]);
    }
    printf("\n");
}

void 
genMat(uint32_t *matrix, uint32_t *key, uint32_t nuo[3]){
    memcpy((void*)&matrix[4], key, 32);
    memset((void*)&matrix[12], 0, 4);
    memcpy((void*)&matrix[13], nuo, 12);
}

uint32_t*
shuffle(uint32_t *matrix, uint32_t block){
    uint32_t *mat=(uint32_t*)malloc(64);
    memcpy((void*)mat, (void*)matrix, 64);
    
    for(int8_t iter=0; iter<20; iter++){
        if(iter%2){
            shuffleSide(mat,1);
        }else{
            shuffleSide(mat,0);
        }
    }
    return mat;
}

void shuffleSide(uint32_t *matrix, _Bool side){
    uint32_t *a, *b, *c, *d;
    if(side){
        a=&matrix[0];
        b=&matrix[4];
        c=&matrix[8];
        d=&matrix[12];
    }else{
        a=&matrix[0];
        b=&matrix[5];
        c=&matrix[10];
        d=&matrix[15];
    }
    for(int8_t iter=0; iter<4; iter++){
        //printf("a-pre%x\nb-pre%x\nc-pre%x\nd-pre%x\n", *a, *b, *c, *d);
        *a=addmod(*b,*a);
        *d=*d^*a;
        *d=bitShift(*b, 16);
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
            a=&matrix[0+iter];
            b=&matrix[5+iter>7?4:5+iter];
            c=&matrix[10+iter>11?iter+10-4:iter+10];
            d=&matrix[15+iter>15?iter+15-4:15];
        }
    }
    //printUint(matrix, 16);
}


static inline uint32_t bitShift(uint32_t target, uint8_t bits){
    return (target<<bits)|(target>>(32-bits));
}

static inline uint32_t addmod(uint32_t number1, uint32_t number2){
    return number1+number2;
}
