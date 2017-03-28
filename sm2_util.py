from sage.rings.finite_rings.finite_field_constructor import GF
from sage.schemes.elliptic_curves.constructor import EllipticCurve
from sage.rings.integer import Integer
import pyximport 
pyximport.install()

from sm3 import sm3_hash_sage
# s is a bit string 

def bytes(i):
    i0 = i & 0xff
    i1 = (i >> 8) & 0xff
    i2 = (i >> 16) & 0xff
    i3 = (i >> 24) & 0xff
    return [i3, i2, i1, i0]

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

def EC_group_new(p, a_bin, b_bin, xG_bin, yG_bin, h):
    F = GF(p)
    a, b = F(a_bin), F(b_bin)
    C = EllipticCurve(F, [a,b])
    #print "the curve we are using is %s" %C

    # create base point
    xG, yG = F(xG_bin), F(yG_bin)
    G = C(xG, yG)
    n = G.order()
    #print "the base point we are using has order %s" %n
    return (C, n, h, G)

# curves recommended by SM2 Standard
# sm2p192test, sm2p256test, sm2b193test, sm2b257test
# sm2p256v1

sm2p192test = EC_group_new(
    p = Integer("BDB6F4FE3E8B1D9E0DA8C0D46F4C318CEFE4AFE3B6B8551F", base=16),
	a_bin = Integer("BB8E5E8FBC115E139FE6A814FE48AAA6F0ADA1AA5DF91985",base =16),
	b_bin = Integer("1854BEBDC31B21B7AEFC80AB0ECD10D5B1B3308E6DBF11C1", base=16),
	xG_bin = Integer("4AD5F7048DE709AD51236DE65E4D4B482C836DC6E4106640", base=16),
	yG_bin = Integer("02BB3A02D4AAADACAE24817A4CA3A1B014B5270432DB27D2", base=16),
    h = Integer(1)
)

sm2p256test = EC_group_new( 
    p = Integer("8542D69E 4C044F18 E8B92435 BF6FF7DE 45728391 5C45517D 722EDB8B 08F1DFC3", base=16),
    a_bin = Integer("787968B4 FA32C3FD 2417842E 73BBFEFF 2F3C848B 6831D7E0 EC65228B 3937E498", base=16),
    b_bin = Integer("63E4C6D3 B23B0C84 9CF84241 484BFE48 F61D59A5 B16BA06E 6E12D1DA 27C5249A", base=16),
    xG_bin = Integer("421DEBD6 1B62EAB6 746434EB C3CC315E 32220B3B ADD50BDC 4C4E6C14 7FEDD43D", base=16),
    yG_bin = Integer("0680512B CBB42C07 D47349D2 153B70C4 E5D7FDFC BFA36EA1 A85841B9 E46E09A2", base=16),
    h = Integer(1)
)

sm2p256v1 = EC_group_new(
    p = Integer("FFFFFFFE FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF 00000000 FFFFFFFF FFFFFFFF", base=16),
    a_bin = Integer("FFFFFFFE FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF 00000000 FFFFFFFF FFFFFFFC", base=16),
    b_bin = Integer("28E9FA9E 9D9F5E34 4D5A9E4B CF6509A7 F39789F5 15AB8F92 DDBCBD41 4D940E93", base=16),
    xG_bin = Integer("32C4AE2C 1F198119 5F990446 6A39C994 8FE30BBF F2660BE1 715A4589 334C74C7", base=16),
    yG_bin = Integer("BC3736A2 F4F6779C 59BDCEE3 6B692153 D0A9877C C62A4740 02DF32E5 2139F0A0", base=16),
    h = Integer(1)
)
