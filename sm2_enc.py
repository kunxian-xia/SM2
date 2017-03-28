from sm2_util import sm2p256v1, sm2p256test, sm2p192test, kdf
from sage.misc.functional import log_b
from sage.functions.other import ceil, floor
from sage.misc.prandom import random
import pyximport 

pyximport.install()

from sm3 import sm3_hash_sage

if __name__ == "__main__":
    (C, n, h, G) = sm2p192test

#a \in F_q (q is a odd prime)
def bytes(a, q):
    t = ceil(log_b(q, 2))
    l = ceil(t/8)
    bs = []
    for i in range(l):
        bs.append(a & 0xff)
        a = a >> 8
    return bs

def is_list_of_zeros(l):
    flag = True
    for e in l:
        if e == 0:
            flag = False
            break
    return flag
#G: base point of order n, cofactor h over finite field Fq 
#encrypt message using pk 
#message is a list of byte
def sm2_do_enc(G, n, h, q, pk, M, klen, determ, predefined_k):
    while (True):
        #TODO: k should be uniformly distributed over [1, n-1]
        k = floor(random()*n)
        if k == 0:
            continue
        if determ:
            k = predefined_k
        C1 = k*G
        (x1, y1) = C1.xy()

        S = h*pk
        if S == 0*G:
            """
            """
            #raise error
        
        (x2, y2) = (k*pk).xy()

        x2_bytes = bytes(x2, q)
        y2_bytes = bytes(y2, q)
        x2y2 = []
        x2y2.extend(x2_bytes)
        x2y2.extend(y2_bytes)
        
        t = kdf(x2y2, klen)
        if is_list_of_zeros(t):
            continue
        
        break
    C2 = [ M[i].__xor__(t[i]) for i in range(len(M)) ]
    C3 = sm3_hash_sage(x2_bytes.extend(M).extend(y2_bytes))

    return (C1, C3, C2)

