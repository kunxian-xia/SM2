
/*
 * 原理介绍：
 * 
 * 在椭圆曲线群中，计算 -P 是很简单的；对于Weierstrass型曲线，若P = (x,y)，则-P = (x, -y)。
 * 
 * 因此，椭圆曲线点的多倍运算 k*P中，我们有必要要利用这种结构来减少预计算点的个数。
 * 
 * 比如，整数k = (1 00111 00010 11011)，二进制数k可以表达成如下的16进制数：
 *          k = (1 7 3 -5)
 * 
 * 很明显，11011可以向上进1, 然后变成一个负数， 即11011 = 100000-100-1
 * 于是，可以用ec_GFp_nistp_recode_scalar_bits函数来计算出。
 */

//in = a[5j+4] ... a[5j] a[5(j-1)+4]
void ec_GFp_nistp_recode_scalar_bits(unsigned char *sign,
                                     unsigned char *digit, unsigned char in)
{
    unsigned char s, d;

    s = ~((in >> 5) - 1);       /* sets all bits to MSB(in), 'in' seen as
                                 * 6-bit value */
    d = (1 << 6) - in - 1;
    d = (d & s) | (in & ~s);    /* d = (in[5]^in[4],...,in[5]^in[0]*/

    d = (d >> 1) + (d & 1);     /* d = (in[5]^in[4],...,in[5]^in[1]) 
                                 *                   + (in[5]^in[0]) */
    
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
