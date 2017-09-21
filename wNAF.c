#include <stdio.h>
//compute the wNAF of a big integer

void ec_GFp_nistp_recode_scalar_bits(unsigned char *sign,
                                     unsigned char *digit, unsigned char in)
{
    unsigned char s, d;

    s = ~((in >> 5) - 1);       /* sets all bits to MSB(in), 'in' seen as
                                 * 6-bit value */
    d = (1 << 6) - in - 1;
    d = (d & s) | (in & ~s);
    //d = 0 0 0 in[5]^in[4] ... in[5]^in[0]
    d = (d >> 1) + (d & 1);
    /**
     * d = 0 0 0 0 in[5]^in[4] ... in[5]^in[1]
     *   + 0 0 0 0           0 ... in[5]^in[0]
     *
     */
    *sign = s & 1;
    *digit = d;
}

unsigned char get_bit(unsigned char *scalar, int i, int n)
{
    if (i < 0 || i >= n) {
        return 0;
    } else {
        return (scalar[i >> 3] >> (i & 0x7)) & 1;
    }
}
//scalar is a n-bit integer
void compute_wnaf(unsigned char *scalar, int n,
          unsigned char *sign, unsigned char *digit)
{
    int i, j;
    unsigned char bits;

    for (i=n-1, j=0; i >= 0; i--) {
        if (i%5 == 0) {
            bits  = get_bit(scalar, i+4, n) << 5;
            bits |= get_bit(scalar, i+3, n) << 4;
            bits |= get_bit(scalar, i+2, n) << 3;
            bits |= get_bit(scalar, i+1, n) << 2;
            bits |= get_bit(scalar, i, n) << 1;
            bits |= get_bit(scalar, i-1, n);

            ec_GFp_nistp_recode_scalar_bits(sign+j, digit+j, bits);
            j++;
        }
    }
}
