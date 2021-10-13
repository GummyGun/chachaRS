#include <stdio.h>
#include <sodium.h>
#include <string.h>
#include <errno.h>
#include "chacha.h"
#include "mac.h"

enum action{
    chacha, auth
};

int32_t
main(int32_t argc, char *argv[]){
    int32_t target=0;
    char *keyName;

    enum action action=chacha;
    
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
                    case 'e':{
                        if(argc!=6){
                            printf("%d\n", argc);
                            return 2;
                        }
                        _Bool state=0;
                        switch(argv[iter][iterFlags+1]){
                            case 'e':{
                                state=mac(argv, encode);
                                break;
                            }
                            case 'd':{
                                state=mac(argv, decode);
                                break;
                            }
                        }
                        return state;
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
            shuffled=shuffleMatrix(matrix, iter/16);
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

