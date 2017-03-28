#clib ./sm3

cdef extern from "sm3.h":
    void sm3_hash(const unsigned char *data, size_t data_len, unsigned char digest[32])

from libc.stdlib cimport malloc, free

def sm3_hash_sage(inlist):
    inlen = len(inlist)
    cdef unsigned char* input = <const unsigned char*> malloc(inlen * sizeof(unsigned char))
    cdef unsigned char* hash = <const unsigned char*> malloc(32)

    for i in range(inlen):
        input[i] = inlist[i]

    sm3_hash(input, inlen, hash)

    hash_value = [hash[i] for i in range(32)]

    free(input)
    free(hash)

    return hash_value
