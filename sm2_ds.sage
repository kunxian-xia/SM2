# coding: utf-8
from sm2_util import *

def test():
    (q, C, n, h, G) = sm2p256test   
    sk = Integer("128B2FA8 BD433C6C 068C8D80 3DFF7979 2A519A55 171B1B65 0C23661D 15897263", base=16) 
    pre_k = Integer("6CB28D99 385C175C 94F94E93 4817663F C176D925 DD72B727 260DBAAE 1FB2F96F", base=16)
    a = Integer("787968B4 FA32C3FD 2417842E 73BBFEFF 2F3C848B 6831D7E0 EC65228B 3937E498", base=16)
    b = Integer("63E4C6D3 B23B0C84 9CF84241 484BFE48 F61D59A5 B16BA06E 6E12D1DA 27C5249A", base=16)
    M = "message digest"
    IDA = "ALICE123@YAHOO.COM"
    ENTLA = [0x0, 0x0, 0x9, 0x0]

    M = [ord(c) for c in M]
    IDA = [ord(c) for c in IDA]
    pk = sk * G
    xA, yA = pk.xy()
    xA_bytes = field_to_bytes(xA, q)
    yA_bytes = field_to_bytes(yA, q)

    ZA = ENTLA[:]
    ZA.extend(IDA)
    ZA.extend( Integer_to_bytes(a, 256) )
    ZA.extend( Integer_to_bytes(b, 256) )
    ZA.extend(xA_bytes)
    ZA.extend(yA_bytes)
    ZA = sm3_hash_sage(ZA)
    
    print "ZA: %s" %( hex_list(ZA) )
    (r,s) = sm2_sign(G, n, sk, M, ZA, True, pre_k)
    is_valid = sm2_verify(G, n, pk, M, ZA, r, s)

    print "the signature is %s" % is_valid

# SM2 Ditial Signature Sign Algorithm
def sm2_sign(G, n, sk, M, ZA, determ, pre_k):
    print "begin to sign the message"

    #TODO: e = Hash(ZA || M)
    Mbar = ZA[:]
    Mbar.extend(M)
    e = sm3_hash_sage(Mbar)
    e_int = bytes_to_Integer(e)

    print "e: %s" %(hex_list(e).upper())
    
    while (True):
        #TODO: k should be generated from uniform distribution
        k = floor(random()*n)
        if k == 0:
            continue
        if determ:
            k = pre_k
    
        # [k]G
        rand_point = k*G
        x1, y1 = rand_point.xy()
        x1= x1.lift()
        y1 = y1.lift()

        r = mod(e_int + x1, n).lift()
        if (r == 0) or (r+k == n):
            continue
        
        s = mod(inverse_mod(1+sk, n)*(k- r*sk), n).lift()
        if s == 0:
            continue

        print "x1: %s\n y1: %s\n" %( hex(x1), hex(y1) )
        print "r : %s\n s : %s" %( hex(r), hex(s))
        break
    return (r, s)

def sm2_verify(G, n, pk, M, ZA, r, s):
    """
    """
    print "begin to verify the signature"

    if not ( (1 <= r) and (r <= (n-1)) ):
        return False
    
    if not ( (1 <= s) and (s <= (n-1)) ):
        return False
    
    ##TODO: e should be equal to H(ZA || M)
    Mbar = ZA[:]
    Mbar.extend(M)
    e = sm3_hash_sage(Mbar)
    e_int = bytes_to_Integer(e)

    t = mod(r+s, n).lift()
    if t == 0:
        return False
    
    (x1, y1) = (s*G + t*pk).xy()

    R = mod(e_int + x1.lift(), n).lift()
    if R == r:
        return True
    else:
        return False
