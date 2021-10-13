#include <stdio.h>
#include <stdint.h>

#include <sodium.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include "mac.h"




_Bool
file_opener(int32_t *originalFD, int32_t *editedFD, char *argv[], int32_t index){
    if((*originalFD=open(argv[index], O_RDONLY))<=0){
        perror("Error opening original");
        return 1;
    }
    if((*editedFD=open(argv[index+1], O_RDWR|O_CREAT, (S_IWUSR|S_IRUSR) | (S_IRGRP) | (S_IROTH)))<=0){
        perror("Error opening edited");
        return 1;
    }
    return 0;
}

_Bool
file_sizeSetUp(int32_t originalFD, int32_t editedFD, size_t *size, enum mode mode){

    struct stat messageStat;
    if((fstat(originalFD, &messageStat))<0){
        perror("the error is that");
        return 1;
    }
    *size=messageStat.st_size;

    size_t tam=((mode==encode)?*size+crypto_box_MACBYTES:*size-crypto_box_MACBYTES);
    ftruncate(editedFD, tam);

    return 0;
}

_Bool
file_mmapper(int32_t *originalFD, int32_t *editedFD, void **originalMemory, void **editedMemory, size_t size, enum mode mode){

    if((*originalMemory=(char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, *originalFD, (size_t)0))==(char *) MAP_FAILED){
        perror("Error mapping original memory");
        return 1;
    }
    int32_t sizeCal=size+((mode==encode)?crypto_box_MACBYTES:-crypto_box_MACBYTES);
    if((*editedMemory=(char*)mmap(NULL, sizeCal, PROT_READ|PROT_WRITE, MAP_SHARED, *editedFD, (size_t)0))==(char *)MAP_FAILED){
        perror("Error mapping newMem");
        return 1;
    }
    
    close(*originalFD);
    close(*editedFD);
    return 0;
}

_Bool 
mac(char *argv[], enum mode mode){

    FILE *publicKey, *secretKey;
    publicKey=fopen(argv[2], "rb");
    secretKey=fopen(argv[3], "rb");

    unsigned char pk[crypto_box_SECRETKEYBYTES+100];
    unsigned char sk[crypto_box_SECRETKEYBYTES+100];


    fread((void*)pk, sizeof(char), crypto_box_SECRETKEYBYTES, publicKey);
    fread((void*)sk, sizeof(char), crypto_box_SECRETKEYBYTES, secretKey);

    fclose(publicKey);
    fclose(secretKey);

    int32_t OG_fileFD=0;
    int32_t ED_fileFD=0;

    size_t fileSize;
    char *OG_fileMem=NULL;
    char *ED_fileMem=NULL;

    if(file_opener(&OG_fileFD, &ED_fileFD, argv, 4)){
        return 1;
    };
    if(file_sizeSetUp(OG_fileFD, ED_fileFD, &fileSize, mode)){
        return 2;
    };
    printf("file size: %ld\n", fileSize);
    if(file_mmapper(&OG_fileFD, &ED_fileFD, (void**) &OG_fileMem, (void**) &ED_fileMem, fileSize, mode)){
        return 3;
    };

    unsigned char nonce[crypto_box_NONCEBYTES]="";

    if(mode==encode){
        if(crypto_box_easy((void*)ED_fileMem, (void*)OG_fileMem, fileSize, nonce, pk, sk)){
            printf("error encrypting");
            return 4;
        }
    }else{
        if(crypto_box_open_easy((void*)ED_fileMem, (void*)OG_fileMem, fileSize, nonce, pk,  sk) != 0) {
            //printf("%s\n", OG_fileMem);
            printf("algo fallo en el decryptado\n");
            return 5;
        }
    }

    munmap(OG_fileMem, fileSize);
    munmap(ED_fileMem, fileSize+crypto_box_MACBYTES);

    return 1;
}

int asdfasdfas(int32_t argc, char *argv[]){
    
    //unsigned char senderSecKey[crypto_box_SECRETKEYBYTES], senderPubKey[crypto_box_PUBLICKEYBYTES], receiverSecKey[crypto_box_SECRETKEYBYTES], receiverPubKey[crypto_box_PUBLICKEYBYTES];

    enum mode action=(argv[1][0]=='e')?encode:decode;

    unsigned char pk[crypto_sign_PUBLICKEYBYTES];
    unsigned char sk[crypto_sign_SECRETKEYBYTES];
    unsigned char seed[crypto_box_SEEDBYTES]="";
    crypto_box_seed_keypair(pk, sk, seed);

    unsigned char rpk[crypto_sign_PUBLICKEYBYTES];
    unsigned char rsk[crypto_sign_SECRETKEYBYTES];
    seed[0]='a';
    crypto_box_seed_keypair(rpk, rsk, seed);

    int32_t OG_fileFD=0;
    int32_t ED_fileFD=0;

    size_t fileSize;
    char *OG_fileMem=NULL;
    char *ED_fileMem=NULL;

    if(file_opener(&OG_fileFD, &ED_fileFD, argv, 2)){
        return 1;
    };
    if(file_sizeSetUp(OG_fileFD, ED_fileFD, &fileSize, action)){
        return 2;
    };
    printf("file size: %ld\n", fileSize);
    if(file_mmapper(&OG_fileFD, &ED_fileFD, (void**) &OG_fileMem, (void**) &ED_fileMem, fileSize, action)){
        return 3;
    };

    //unsigned long long int messageEncLen=0;

    /*
    crypto_box_easy(ED_fileMem, OG_fileMem, messageStat.st_size, &messageEncLen, receiverPubKey, senderSecKey);
    */
    
    unsigned char nonce[crypto_box_NONCEBYTES]="";

    if(action==encode){
        if(crypto_box_easy((void*)ED_fileMem, (void*)OG_fileMem, fileSize, nonce, pk, rsk)){
            printf("error encrypting");
            return 4;
        }
    }else{
        if(crypto_box_open_easy((void*)ED_fileMem, (void*)OG_fileMem, fileSize, nonce, rpk,  sk) != 0) {
            //printf("%s\n", OG_fileMem);
            printf("algo fallo en el decryptado\n");
            return 5;
        }
    }


    printf("message OG:----------------------------------------------------------%p\n %s\n", OG_fileMem, OG_fileMem);
    printf("message ED:----------------------------------------------------------%p\n %s\n", ED_fileMem, ED_fileMem);

    munmap(OG_fileMem, fileSize);
    munmap(ED_fileMem, fileSize+crypto_box_MACBYTES);

    /*
    FILE *publicKey, *secretKey;
    publicKey=fopen("publicKey", "wb");
    secretKey=fopen("privateKey", "wb");
    fwrite((void*)pk, sizeof(char), crypto_sign_PUBLICKEYBYTES, publicKey);
    fwrite((void*)sk, sizeof(char), crypto_sign_SECRETKEYBYTES, secretKey);
    fclose(publicKey);
    fclose(secretKey);
    */
    return 0;
}


