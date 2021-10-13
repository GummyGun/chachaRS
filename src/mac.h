#ifndef __GUM_MAC__
#define __GUM_MAC__ 1

enum mode{
    encode, decode
};

_Bool file_mmapper(int32_t *originalFD, int32_t *editedFD, void **originalMemory, void **editedMemory, size_t size, enum mode mode);
_Bool file_sizeSetUp(int32_t originalFD, int32_t editedFD, size_t *size, enum mode mode);
_Bool file_opener(int32_t *originalFD, int32_t *editedFD, char *argv[], int32_t index);

_Bool mac(char *argv[], enum mode mode);

#endif
