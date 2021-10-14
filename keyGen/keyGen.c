#include <sodium.h>

int main(int argc, const char *argv[]){
    char secK[crypto_box_SECRETKEYBYTES], pubK[crypto_box_PUBLICKEYBYTES];
    crypto_box_keypair(pubK, secK);
    FILE *pub;
    char helper[100]="";
    snprintf(helper, 100, "%s.pub", argv[1]);
    pub=fopen(helper, "wb");
    fwrite(pubK, crypto_box_PUBLICKEYBYTES, 1, pub);
    fclose(pub);
    snprintf(helper, 100, "%s.sec", argv[1]);
    pub=fopen(helper, "wb");
    fwrite(secK, crypto_box_SECRETKEYBYTES, 1, pub);
    fclose(pub);
}
