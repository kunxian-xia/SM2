/*
 * an implementation of the arithmetic of the prime field p224 
 * where p224 = 2^224 - 2^96 + 1
 *
 * addition, multiplication, multiply by constant
 */


//field element is represented as 
// a[0] + a[1]*2^56 + a[2]*2^112 + a[3]*2^168
typedef unsigned long uint64_t;
typedef uint64_t felem[4];
//p224 = (1 + 2^56 | 2^56 - 2^40 - 1 | 2^56 - 1 | 2^56 - 1) = (two56p1 | two56m40m1 | two56m1 | two56m1)


# if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
/* even with gcc, the typedef won't work for 32-bit platforms */
typedef __uint128_t uint128_t;  /* nonstandard; implemented by gcc on 64-bit
                               * platforms */
# else
#  error "Need GCC 3.1 or later to define type uint128_t"
# endif

//multiply two field element without reduction modulo p224 is a number
// c[0] + c[1]*2^56 + c[2]*2^112 + c[3]*2^168 + c[4]*2^224 + c[5]*2^280 + c[6]*2^336
typedef uint128_t widefelem[7];

static void felem_assign(felem out, const felem in) {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = in[3];
}
/* Get negative value: out = -in */
/* Assumes in[i] < 2^57 */
static void felem_neg(felem out, const felem in)
{
    static const limb two58p2 = (((limb) 1) << 58) + (((limb) 1) << 2);
    static const limb two58m2 = (((limb) 1) << 58) - (((limb) 1) << 2);
    static const limb two58m42m2 = (((limb) 1) << 58) -
        (((limb) 1) << 42) - (((limb) 1) << 2);

    /* Set to 0 mod 2^224-2^96+1 to ensure out > in */
    //out = 4*p - in 
    out[0] = two58p2 - in[0];
    out[1] = two58m42m2 - in[1];
    out[2] = two58m2 - in[2];
    out[3] = two58m2 - in[3];

    //Because in[i] < 2^57,  0 < out[i] < 2^58
}

//in[i] < 2^126
//out[0], out[1], out[2] < 2^56, out[3] < 2^56 + 2^16
static void felem_reduce(felem out, const widefelem in)
{
    uint128_t tmp[5];
    uint128_t two127p15 = (((uint128_t) 1) << 127) +
    (((uint128_t) 1) << 15);
    uint128_t two127m71m55 = (((uint128_t) 1) << 127) -
    (((uint128_t) 1) << 71) - (((uint128_t) 1) << 55);
    uint128_t two127m71 = (((uint128_t) 1) << 127) -
    (((uint128_t) 1) << 71);

    //p224 = (1 | -2^40 | 2^112), 2^15*p = (2^15 | -2^55 | 2^127)
    //(2^127 + 2^15 | 2^127 - 2^71 - 2^55 | 2^127 -2^71 )
    tmp[0] = in[0] + two127p15;
    tmp[1] = in[1] + two127m71m55;
    tmp[2] = in[2] + two127m71;
    tmp[3] = in[3];
    tmp[4] = in[4];

    //reduce in[6]*2^336: in[6]*2^112*(2^96 - 1) (mod p224)
    tmp[3] += (in[6] & 0xffff) << 40;
    tmp[4] += (in[6] >> 16); 
    tmp[2] -= in[6];

    //reduce in[5]*2^280 = 2^56*(2^96-1)*in[5] (mod p224)
    tmp[2] += (in[5] & 0xffff) << 40;
    tmp[3] += (in[5] >> 16);
    tmp[1] -= in[5];

    //reduce tmp[4]*2^224
    tmp[1] += (tmp[4] & 0xffff) << 40;
    tmp[2] += (tmp[4] >> 16);
    tmp[0] -= tmp[4];

    //now in = tmp[0] | tmp[1] | tmp[2] | tmp[3] (mod p224)

    //进位 2 -> 3 -> 4
    tmp[3] += tmp[2] >> 56;
    tmp[2] &= 0x00ffffffffffffff;

    tmp[4] = tmp[3] >> 56;
    tmp[3] &= 0x00ffffffffffffff;

    //tmp[2] < 2^56, tmp[3] < 2^56, tmp[4] < 2^72

    //reduce tmp[4]*2^224
    tmp[1] += (tmp[4] & 0xffff) << 40;
    tmp[2] += (tmp[4] >> 16);
    tmp[0] -= tmp[4];

    //进位 0 -> 1 -> 2 -> 3
    tmp[1] += tmp[0] >> 56;
    tmp[0] &= 0x00ffffffffffffff;

    tmp[2] += tmp[1] >> 56;
    tmp[1] &= 0x00ffffffffffffff;

    tmp[3] += tmp[2] >> 56;

    out[0] = (uint64_t) tmp[0];
    out[1] = (uint64_t) tmp[1];
    out[2] = (uint64_t) tmp[2];
    out[3] = (uint64_t) tmp[3];
}

struct nistp224_point {
    felem xyz[3];
};

//precomputed multiples of G
struct nistp224_pre_comp_st {
    nistp224_point g_pre_comp[2][16];
};

struct nistp224_point point_at_infty = {
    {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}
};

void point_assign(nistp224_point *out, const nistp224_point *in) {
    felem_assign(out->xyz[0], in->xyz[0]);
    felem_assign(out->xyz[1], in->xyz[1]);
    felem_assign(out->xyz[2], in->xyz[2]);
}
//compute multiples of G: 
//   b_0 * G + b_1 * 2^56*G + b_2*2^112*G + b_3*2^168*G
//b_0 *2^28*G + b_1* 2^84*G + b_2*2^140*G + b_3*2^192*G  
nistp224_pre_comp_st* precompute_mult(const nistp224_point *point) {
    int i, j;
    //compute G1, G2, G3
    //g_pre_comp[1][1<<i] = 2^28*g_pre_comp[0][1<<i]
    //g_pre_comp[0][1<<(i+1)] = 2^28*g_pre_comp[1][1<<i]
    nistp224_pre_comp_st *pre = malloc(sizeof(nistp224_pre_comp_st));
    if (pre == NULL) {

    }
    point_assign(pre->g_pre_comp[0][1], point);
    for (i=0; i < 3; i++) {
        point_double(pre->g_pre_comp[1][1<<i], pre->g_pre_comp[0][1<<i]);
        for (j=0; j < 27; j++) 
            point_double(pre->g_pre_comp[1][1<<i], pre->g_pre_comp[1][1<<i]);

        point_double(pre->g_pre_comp[0][1<<(i+1)], pre->g_pre_comp[1][1<<i]);
        for (j=0; j < 27; j++)
            point_double(pre->g_pre_comp[0][1<<(i+1)], pre->g_pre_comp[0][1<<(i+1)]);
    }

    point_double(pre->g_pre_comp[1][3], pre->g_pre_comp[0][3]);
    for (j=0; j < 27; j++) 
        point_double(pre->g_pre_comp[1][3], pre->g_pre_comp[1][3]);
    
    //compute b0*G + b1*G1 + b2*G2
    for (i=0; i < 2; i++) {
        point_add(pre->g_pre_comp[i][3], pre->g_pre_comp[i][1], pre->g_pre_comp[i][2]);
        point_add(pre->g_pre_comp[i][5], pre->g_pre_comp[i][1], pre->g_pre_comp[i][4]);
        point_add(pre->g_pre_comp[i][6], pre->g_pre_comp[i][2], pre->g_pre_comp[i][4]);
        point_add(pre->g_pre_comp[i][7], pre->g_pre_comp[i][1], pre->g_pre_comp[i][6]);

        for (j=1; j <= 7; j++) {
            point_add(pre->g_pre_comp[i][j+8], pre->g_pre_comp[i][j], pre->g_pre_comp[i][8]);
        }
    }
    
    return pre;
}

