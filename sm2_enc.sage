from sm2_util import *
from sage.rings.integer import Integer
from sage.misc.prandom import random

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
        K.extend( sm3_hash_sage(t)[: (klen - n*256)/8] )
        return K
        
def is_list_of_zeros(l):
    flag = True
    for e in l:
        if e != 0:
            flag = False
            break
    return flag

def hex_list(l):
    h = ""
    for b in l:
        h = h + "%c%c" %( hex((b>>4) & 0xf), hex(b & 0xf) )
    return h

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

        (x1, y1) = (k*G).xy()
        x1_bytes = field_to_bytes(x1, q)
        y1_bytes = field_to_bytes(y1, q)

        # x1 || y1
        C1 = []
        C1.extend(x1_bytes)
        C1.extend(y1_bytes)
        
        S = h*pk
        if S == 0*G:
            return
        
        (x2, y2) = (k*pk).xy()
        x2_bytes = field_to_bytes(x2, q)
        y2_bytes = field_to_bytes(y2, q)
        #x2 || y2
        x2y2 = []
        x2y2.extend(x2_bytes)
        x2y2.extend(y2_bytes)

        t = kdf(x2y2, klen)
        if is_list_of_zeros(t):
            continue
        print "x1: %s \n y1: %s" %(hex_list(x1_bytes), hex_list(y1_bytes))
        print "x2: %s \n y2: %s" %(hex_list(x2_bytes), hex_list(y2_bytes))
        print "t: %s" %(hex_list(t))

    	break
    #M xor t
    C2 = [ M[i].__xor__(t[i]) for i in range(len(M)) ]

    # C3 = Hash(x2 || M || y2)
    x2_bytes.extend(M)
    x2_bytes.extend(y2_bytes)
    C3 = sm3_hash_sage(x2_bytes)
    
    CC = []
    CC.extend(C1)
    CC.extend(C3)
    CC.extend(C2)
    print "C2 is %s \n C3 is %s\n ciphertext is %s\n" %( hex_list(C2), hex_list(C3), hex_list(CC).upper())

    return CC

def test():
    data = [(sm2p256test, Integer("4C62EEFD6ECFC2B95B92FD6C3D9575148AFA17425546D49018E5388D49DD7B4F", base=16),
                          Integer("1649AB77A00637BD5E2EFE283FBF353534AA7F7CB89463F208DDBC2920BB0DA0",base=16),
                          ("245C26FB68B1DDDDB12C4B6BF9F2B6D5FE60A383B0D18D1C4144ABF17F6252E7"
                            "76CB9264C2A7E88E52B19903FDC47378F605E36811F5C07423A24B84400F01B8"
                            "650053A89B41C418B0C3AAD00D886C00286467"
		                    "9C3D7360C30156FAB7C80A0276712DA9D8094A634B766D3A285E07480653426D")),
            (sm2p192test, Integer("384F3035 3073AEEC E7A16543 30A96204 D37982A3 E15B2CB5",base=16),
                          Integer("58892B80 7074F53F BF67288A 1DFAA1AC 313455FE 60355AFD",base=16),
                          ( "23FC680B124294DFDF34DBE76E0C38D883DE4D41FA0D4CF570CF14F20DAF0C4D"
                            "777F738D16B16824D31EEFB9DE31EE1F6AFB3BCEBD76F82B252CE5EB25B57996"
                            "86902B8CF2FD87536E55EF7603B09E7C610567DBD4854F51F4F00ADCC01CFE90"
                          ))]
    for (curve, pre_k, sk, ciphertext) in data:
        (q, C, n, h, G) = curve
        M = "encryption standard"
        M = [ord(c) for c in M]

        pk = sk*G
        pkx, pky = pk.xy()

        print "the message in hex: %s" % hex_list(M)
        print "the public key: \n x: %s \n y: %s" %( hex_list(field_to_bytes(pkx, q)) , 
                                                    hex_list(field_to_bytes(pky, q)))
    
    
        cc = sm2_do_enc(G, n, h, q, pk, M, len(M)*8, True, pre_k)

        print hex_list(cc).upper() == ciphertext
