from sm2_util import *
from sage.rings.integer import Integer
from sage.misc.prandom import random

def test():
    (C, n, h, G) = sm2p256test
    q = 256
    M = "encryption standard"
    M = [ord(c) for c in M]

    print "the message in hex: %s" % "".join([hex(c) for c in M])
    sk = Integer("1649AB77A00637BD5E2EFE283FBF353534AA7F7CB89463F208DDBC2920BB0DA0", base=16)
    pk = sk*G
    pre_k = Integer("4C62EEFD6ECFC2B95B92FD6C3D9575148AFA17425546D49018E5388D49DD7B4F", base=16)

    ciphertext = (
        "245C26FB68B1DDDDB12C4B6BF9F2B6D5FE60A383B0D18D1C4144ABF17F6252E7"
        "76CB9264C2A7E88E52B19903FDC47378F605E36811F5C07423A24B84400F01B8"
        "650053A89B41C418B0C3AAD00D886C00286467"
		"9C3D7360C30156FAB7C80A0276712DA9D8094A634B766D3A285E07480653426D")
    cc = sm2_do_enc(G, n, h, q, pk, M, len(M)*8, True, pre_k)
    print "".join([hex(c) for c in cc]) == ciphertext

def kdf(Z, klen):
    n = floor(klen/256)
    K = []
    for i in range(1, n+1):
        t = Z[:]
        t.extend(bytes(i))
        K.extend( sm3_hash_sage(t) )
    if (klen%256 == 0):
        return K
    else:
        t = Z[:]
        t.extend(bytes(n+1))
        return K.extend(sm3_hash_sage(t)[: (klen - n*256)/8])
        
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
        C1 = []
        C1.extend(field_to_bytes(x1, q)).extend(field_to_bytes(y1, q))
        
        S = h*pk
        if S == 0*G:
            """
            """
            #raise error
        
        (x2, y2) = (k*pk).xy()

        x2_bytes = field_to_bytes(x2, q)
        y2_bytes = field_to_bytes(y2, q)
        x2y2 = []
        x2y2.extend(x2_bytes)
        x2y2.extend(y2_bytes)
        
        t = kdf(x2y2, klen)
        if is_list_of_zeros(t):
            continue
        
        break
    C2 = [ M[i].__xor__(t[i]) for i in range(len(M)) ]
    C3 = sm3_hash_sage(x2_bytes.extend(M).extend(y2_bytes))

    return C1.extend(C3).extend(C2)

