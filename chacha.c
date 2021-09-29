#include <stdio.h>
#include <sodium.h>
#include <string.h>
#include <errno.h>

void printUint(uint32_t key[], int8_t size);
uint32_t* shuffle(uint32_t *matrix, uint32_t block);
void shuffleSide(uint32_t *matrix, _Bool side);
void genMat(uint32_t *matrix, uint32_t *key, uint32_t nuo[3]);
void printAsChars(uint32_t number, int8_t charNum);
static inline uint32_t addmod(uint32_t number1, uint32_t number2);
static inline uint32_t bitShift(uint32_t target, uint8_t bits);

enum OUTPUTMODE{
    std,
    file
};

enum OPENGEN{
    open,
    gen
};

int32_t
main(int32_t argc, char *argv[]){
    int32_t target=0;
    char *keyName;
    
    uint32_t *matrix=(uint32_t*)malloc(512/8);
    uint32_t key[8]={};
    _Bool openGenD=0;
    enum OPENGEN openGen=gen;
    _Bool fileOpened=0;
    _Bool outputModeD=0;
    enum OUTPUTMODE outputMode=std;
    
    
    FILE *cargo;
    FILE *output;
    /*
    flag suport
        k:
            selects encrypting key from document named as next argument
        g:
            generates new key and saves it with the name specified as next argument. cant be used at the same time as k
        o:
            saves the encrypted/decripted data to file named as the next argument
        
        the file specified with no flags before is taken as the input.
        
    */
   
    for(int8_t iter=1; iter<argc; iter++){
        if(argv[iter][0]=='-'){
            
            int8_t flags=strlen(argv[iter]);
            for(int8_t iterFlags=1; iterFlags<flags; iterFlags++){
                _Bool ctrl=0;
                switch(argv[iter][iterFlags]){
                    case 'k':{
                        if(openGenD){
                            return 1;
                        }else{
                            openGen=open;
                            openGenD=1;
                        }
                        FILE *keyDoc;
                        keyDoc=fopen(argv[++iter], "rb");
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
                            return 1;
                        }else{
                            openGen=gen;
                            openGenD=1;
                        }
                        randombytes_buf((void*)key, 32);
                        keyName=argv[++iter];
                        ctrl=1;
                        break;
                    }
                    case 'o':{
                        if(outputModeD){
                            return 2;
                        }else{
                            outputMode=file;
                            outputModeD=1;
                        }
                        output=fopen(argv[++iter], "wb");
                        ctrl=1;
                        break;
                    }
                }
                if(ctrl){
                    ctrl=0;
                    break;
                }
            }
        }else{ //only one file can be specified as input
            if(fileOpened){
                return 4;
            }else{
                fileOpened=1;
            }
            target=iter;
            //printf("file to operate in %s\n", argv[iter]);
            cargo=fopen(argv[iter], "rb");
        }
    }
    if(!fileOpened){
        printf("no file opened\n");
        return 3;
    }
    
    
    //generates random bytes if neither -k -g are present 
    strcpy((char*)(void*)matrix, "expand 32-byte k");
    if(!openGenD){
        randombytes_buf((void*)key, 32);
    }
    
    uint32_t nuo[3]={0,0,0};
    scanf("%d\n%d\n%d", nuo, nuo+1, nuo+2);
    
    genMat(matrix, key, nuo);
    uint32_t bn=0;
    uint32_t *shuffled;
    
    //creates the name of the .key file if flag -g not present
    if(!openGenD){
        char *dif=strchr(argv[target], '.');
        int32_t targetLen;
        if(!dif){
            targetLen=strlen(argv[target]);
        }else{
            targetLen=dif-argv[target];
        }
        keyName=(char*)malloc(targetLen+5);
        snprintf(keyName, targetLen+5, "%.*s.key", targetLen, argv[target]);
    }
    
    //save key to document active if -k not present
    if(openGen==gen){
        FILE *keySav=fopen(keyName, "wb");
        fwrite(key, 4, 8, keySav);
        fclose(keySav);
    }
    
    
    uint32_t buff=0;
    uint32_t encryptedBytes;
    
    int8_t readSize=0;
    int64_t iter=0;
    //read until the input is finished
    while(readSize=fread(&buff, 1, 4, cargo)){
        if(iter%16==0){
            shuffled=shuffle(matrix, iter/16);
        }
        encryptedBytes=shuffled[iter%16];
        //makes the xor 
        
        buff=encryptedBytes^buff;
        
        if(feof(cargo)){
            readSize--;
        }
        
        //conditional whether -o is present or not
        if(outputMode==std){
            printAsChars(buff, readSize);
        }else{
            fwrite(&buff, 1, readSize, output);
        }
        
        memset((void*)&buff, 0, 4);
        
        if(iter%16==15){
            free(shuffled);
        }
        iter++;
    }
    if(outputMode==std){
        printf("\n");
    }
    fclose(cargo);
    if(outputMode==file){
        fclose(output);
    }
    return 0;
}
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
genMat(uint32_t *matrix, uint32_t *key, uint32_t nuo[3]){
    memcpy((void*)&matrix[4], key, 32);
    memset((void*)&matrix[12], 0, 4);
    memcpy((void*)&matrix[13], nuo, 12);
}

//shuffles matrix 
uint32_t*
shuffle(uint32_t *matrix, uint32_t block){
    uint32_t *mat=(uint32_t*)malloc(64);
    memcpy((void*)mat, (void*)matrix, 64);
    mat[12]=block;
    
    
    //printUint(mat, 16);
    for(int8_t iter=0; iter<20; iter++){
        if(iter%2){
            shuffleSide(mat,1);
        }else{
            shuffleSide(mat,0);
        }
    }
    //printUint(mat, 16);
    return mat;
}

//runs algorithm chacha20 side decides whether it is row or column
void 
shuffleSide(uint32_t *matrix, _Bool side){
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
            a=&matrix[0+iter];
            b=&matrix[5+iter>7?4:5+iter];
            c=&matrix[10+iter>11?iter+10-4:iter+10];
            d=&matrix[15+iter>15?iter+15-4:15];
        }
    }
    //printUint(matrix, 16);
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
