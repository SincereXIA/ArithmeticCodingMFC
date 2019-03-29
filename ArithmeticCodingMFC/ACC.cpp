
#include<cstdio>
#include<stdlib.h>
#include<windows.h>

#include "stdafx.h"
#include "ACC.h"
using namespace::std;

/* TRANSLATION TABLES BETWEEN CHARACTERS AND SYMBOL INDEXES. */
int char_to_index[No_of_chars];         /* To index from character          */
unsigned char index_to_char[No_of_symbols + 1]; /* To character from index    */

/*   2^14 - 1           +            */
int cum_freq[No_of_symbols + 1];          /* Cumulative symbol frequencies    */

//用来存储编码值，是编码解码过程的桥梁。大小暂定１００，实际中可以修改
char code[Max_code_length];
int code_index = 0;
int decode_index = 0;

//buffer为八位缓冲区，暂时存放编码制
static int buffer;
//buffer中还有几个比特没有用到，初始值为8
static int bits_to_go;
//超过了EOF的字符，也是垃圾
static int garbage_bits;
int freq[No_of_symbols + 1];

void initACC() {
	code_index = 0;
	decode_index = 0;
	for (int i = 0; i < No_of_symbols; i++) {
		freq[i] = 0;
		cum_freq[i] = 0;
	}
}

void scanFreq(BYTE * source, long long length) {
	for (long long i = 0; i < length; i++) {
		BYTE b = *(source + i);
		freq[(b & 0x0f) + 1]++;
		freq[(b >> 4) + 1] ++;
	}
	freq[EOF_symbol] = 1;
}

void start_model() {

	int i;
	for (i = 0; i < No_of_chars; i++) {
		//为了便于查找
		char_to_index[i] = i + 1;
		index_to_char[i + 1] = i;
	}

	//累计频率cum_freq[i-1]=freq[i]+...+freq[257], cum_freq[257]=0;
	cum_freq[No_of_symbols] = 0;

	for (i = No_of_symbols; i > 0; i--) {
		cum_freq[i - 1] = cum_freq[i] + freq[i];
	}
	//这条语句是为了确保频率和的上线，这是后话，这里就注释掉
	//if (cum_freq[0] > Max_frequency);   /* Check counts within limit*/

}

//初始化缓冲区，便于开始接受编码值
void start_outputing_bits()
{
	buffer = 0;                                //缓冲区一开始为空
	bits_to_go = 8;
}

void output_bit(int bit)
{
	//为了写代码方便，编码数据是从右到左进入缓冲区的。记住这一点！
	buffer >>= 1;
	if (bit) buffer |= 0x80;
	bits_to_go -= 1;
	//当缓冲区满了的时候，就输出存起来
	if (bits_to_go == 0) {
		code[code_index] = buffer;
		code_index++;

		bits_to_go = 8; //重新恢复为8
	}
}

static void bit_plus_follow(int);   /* Routine that follows                    */
static code_value low, high;    /* Ends of the current code region          */
static long bits_to_follow;     /* Number of opposite bits to output after */


void start_encoding()
{
	for (int i = 0; i < 100; i++)code[i] = '\0';

	low = 0;                            /* Full code range.                 */
	high = Top_value;
	bits_to_follow = 0;                 /* No bits to follow           */
}

void encode_symbol(int symbol, int cum_freq[])
{
	long range;                 /* Size of the current code region          */
	range = (long)(high - low) + 1;

	high = low + (range*cum_freq[symbol - 1]) / cum_freq[0] - 1;  /* Narrow the code region  to that allotted to this */
	low = low + (range*cum_freq[symbol]) / cum_freq[0]; /* symbol.                  */

	for (;;)
	{                                  /* Loop to output bits.     */
		if (high < Half) {
			bit_plus_follow(0);                 /* Output 0 if in low half. */
		}
		else if (low >= Half) {                   /* Output 1 if in high half.*/
			bit_plus_follow(1);
			low -= Half;
			high -= Half;                       /* Subtract offset to top. */
		}
		else if (low >= First_qtr && high < Third_qtr) {  /* Output an opposite bit　later if in middle half. */
			bits_to_follow += 1;
			low -= First_qtr;                   /* Subtract offset to middle*/
			high -= First_qtr;
		}
		else break;                             /* Otherwise exit loop.     */
		low = 2 * low;
		high = 2 * high + 1;                        /* Scale up code range.     */
	}
}

/* FINISH ENCODING THE STREAM. */

void done_encoding()
{
	bits_to_follow += 1;                       /* Output two bits that      */
	if (low < First_qtr) bit_plus_follow(0);     /* select the quarter that   */
	else bit_plus_follow(1);                   /* the current code range    */
}
void done_outputing_bits()
{
	//编码最后的时候，当缓冲区没有满，则直接补充０
	code[code_index] = buffer >> bits_to_go;
	code_index++;
}
/* contains.                 */


static void bit_plus_follow(int bit)
{
	output_bit(bit);                           /* Output the bit.           */
	while (bits_to_follow > 0) {
		output_bit(!bit);                      /* Output bits_to_follow     */
		bits_to_follow -= 1;                   /* opposite bits. Set        */
	}                                          /* bits_to_follow to zero.   */
}




//解码

static code_value value;        /* Currently-seen code value                */

void start_inputing_bits()
{
	bits_to_go = 0;                             /* Buffer starts out with   */
	garbage_bits = 0;                           /* no bits in it.           */
}


int input_bit()
{
	int t;

	if (bits_to_go == 0) {
		buffer = code[decode_index];
		decode_index++;

		//    if (buffer==EOF) {
		if (decode_index > code_index) {
			garbage_bits += 1;                      /* Return arbitrary bits*/
			if (garbage_bits > Code_value_bits - 2) {   /* after eof, but check */
				fprintf(stderr, "Bad input file/n"); /* for too many such.   */
				// exit(-1);
			}
		}
		bits_to_go = 8;
	}
	//从左到右取出二进制位，因为存的时候是从右到左
	t = buffer & 1;                               /* Return the next bit from */
	buffer >>= 1;                               /* the bottom of the byte. */
	bits_to_go -= 1;
	return t;
}

void start_decoding()
{
	int i;
	value = 0;                                  /* Input bits to fill the   */
	for (i = 1; i <= Code_value_bits; i++) {      /* code value.              */
		value = 2 * value + input_bit();
	}


	low = 0;                                    /* Full code range.         */
	high = Top_value;
}


char decode_symbol(int cum_freq[])
{
	long range;                 /* Size of current code region              */
	int cum;                    /* Cumulative frequency calculated          */
	char symbol;                 /* Symbol decoded */
	range = (long)(high - low) + 1;
	cum = (((long)(value - low) + 1)*cum_freq[0] - 1) / range;    /* Find cum freq for value. */

	for (symbol = 1; cum_freq[symbol] > cum; symbol++); /* Then find symbol. */
	high = low + (range*cum_freq[symbol - 1]) / cum_freq[0] - 1;   /* Narrow the code region   *//* to that allotted to this */
	low = low + (range*cum_freq[symbol]) / cum_freq[0];

	for (;;) {                                  /* Loop to get rid of bits. */
		if (high < Half) {
			/* nothing */                       /* Expand low half.         */
		}
		else if (low >= Half) {                   /* Expand high half.        */
			value -= Half;
			low -= Half;                        /* Subtract offset to top. */
			high -= Half;
		}
		else if (low >= First_qtr && high < Third_qtr) {
			value -= First_qtr;
			low -= First_qtr;                   /* Subtract offset to middle*/
			high -= First_qtr;
		}
		else break;                             /* Otherwise exit loop.     */
		low = 2 * low;
		high = 2 * high + 1;                        /* Scale up code range.     */
		value = 2 * value + input_bit();            /* Move in next input blt. */
	}
	return symbol;
}

BYTE DecodeRS[1000];

void decode() {
	int i = 0;
	start_model();                              /* Set up other modules.    */
	start_inputing_bits();
	start_decoding();
	for (;;) {                                  /* Loop through characters. */
		int ch; int symbol;
		symbol = decode_symbol(cum_freq);       /* Decode next symbol.      */
		if (symbol == EOF_symbol) break;          /* Exit loop if EOF symbol. */
		ch = symbol - 1;
		DecodeRS[i] = ch;                       /* Write that character.    */
		symbol = decode_symbol(cum_freq);       /* Decode next symbol.      */
		if (symbol == EOF_symbol) break;
		ch = symbol - 1;						/* Exit loop if EOF symbol. */
		DecodeRS[i] |= ch << 4;
		i++;
	}
}

void encode(BYTE * source, int size) {
	initACC();
	scanFreq(source, size);
	start_model();                             /* Set up other modules.     */
	start_outputing_bits();
	start_encoding();
	for (int i = 0; i < size; i++) {
		BYTE b = *(source + i);
		encode_symbol(char_to_index[(b & 0x0f)], cum_freq);
		encode_symbol(char_to_index[b >> 4], cum_freq);
	}
	//将EOF编码进去，作为终止符
	encode_symbol(EOF_symbol, cum_freq);
	done_encoding();                           /* Send the last few bits.   */
	done_outputing_bits();
}