from sm2_util import sm2p192test, sm2p256test, sm2p256v1
# coding: utf-8

if __name__ == "__main__":
    #define the elliptic curve
    
    sk = Integer("128B2FA8 BD433C6C 068C8D80 3DFF7979 2A519A55 171B1B65 0C23661D 15897263", base=16)   
    #n_hex = hex(n)
    # create key pair (sk, pk)
    pk = sk * G
    xA, yA = pk.xy()

    M = ""
    ZA = ""

    (r,s) = sm2_sign(G, n, sk, M, ZA)
    is_valid = sm2_verify(G, n, pk, M, ZA, r, s)

    print "the signature is %s" % is_valid

# SM2 Ditial Signature Sign Algorithm
def sm2_sign(G, n, sk, M, ZA):
    print "begin to sign the message"
    #TODO: e should be equal to H(ZA || M)
    e = Integer("B524F552 CD82B8B0 28476E00 5C377FB1 9A87E6FC 682D48BB 5D42E3D9 B9EFFE76", base=16)
    
    while (True):
        #TODO: k should be generated from uniform distribution
        k = Integer("6CB28D99 385C175C 94F94E93 4817663F C176D925 DD72B727 260DBAAE 1FB2F96F", base=16)
    
        #compute [k]G
        rand_point = k*G
        x1, y1 = rand_point.xy()
    
        r = mod(e + x1.lift(), n).lift()
        if (r == 0) or (r+k == n):
            continue
        
        s = mod(inverse_mod(1+sk, n)*(k- r*sk), n).lift()
        if s == 0:
            continue

        break
    print "signature is (%s, %s) \n End" %(r,s)
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
    e = Integer("B524F552 CD82B8B0 28476E00 5C377FB1 9A87E6FC 682D48BB 5D42E3D9 B9EFFE76", base=16)

    t = mod(r+s, n).lift()
    if t == 0:
        return False
    
    (x1, y1) = (s*G + t*pk).xy()

    R = mod(e + x1.lift(), n).lift()
    if R == r:
        return True
    else:
        return False
