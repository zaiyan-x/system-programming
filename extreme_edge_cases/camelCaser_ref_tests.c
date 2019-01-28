/**
 * Extreme Edge Cases Lab
 * CS 241 - Spring 2019
 */
 
#include "camelCaser.h"
#include "camelCaser_ref_utils.h"
#include <unistd.h>

int main() {
    // Enter the string you want to test with the reference here
    /* char input[65];

    int j = 0;
    for(j=1;j<32;j++){
    	input[j*2-2]=j;
    	input[j*2-1]='.';
    }
    input[62] = 127;
    input[63] = '.';
    input[64] = 0; */

    /*char input[3];
    input[0] = 0;
    input[1] = '.';
    input[2] = 0;*/

/*    char input[121];
	//1
	input[0] = 97;
	input[1] = 46;
	input[2] = 1;
	input[3] = 32;
	input[4] = 33;
	//2
	input[5] = 97;
	input[6] = 46;
	input[7] = 32;
	input[8] = 1;
	input[9] = 33;
	//3
	input[10] = 97;
	input[11] = 1;
	input[12] = 46;
	input[13] = 32;
	input[14] = 33;
	//4
	input[15] = 97;
	input[16] = 1;
	input[17] = 32;
	input[18] = 46;
	input[19] = 33;
	//5
	input[20] = 97;
	input[21] = 32;
	input[22] = 46;
	input[23] = 1;
	input[24] = 33;
	//6
	input[25] = 97;
	input[26] = 32;
	input[27] = 1;
	input[28] = 46;
	input[29] = 33;
	//7
	input[30] = 46;
	input[31] = 97;
	input[32] = 1;
	input[33] = 32;
	input[34] = 33;
	//8
	input[35] = 46;
	input[36] = 97;
	input[37] = 32;
	input[38] = 1;
	input[39] = 33;
	//9
	input[40] = 46;
	input[41] = 1;
	input[42] = 97;
	input[43] = 32;
	input[44] = 33;
	//10
	input[45] = 46;
	input[46] = 1;
	input[47] = 32;
	input[48] = 97;
	input[49] = 33;
	//11
	input[50] = 46;
	input[51] = 32;
	input[52] = 97;
	input[53] = 1;
	input[54] = 33;
	//12
	input[55] = 46;
	input[56] = 32;
	input[57] = 1;
	input[58] = 97;
	input[59] = 33;
	//13
	input[60] = 1;
	input[61] = 97;
	input[62] = 46;
	input[63] = 32;
	input[64] = 33;	
	//14
	input[65] = 1;
	input[66] = 97;
	input[67] = 32;
	input[68] = 46;
	input[69] = 33;
	//15
	input[70] = 1;
	input[71] = 46;
	input[72] = 97;	
	input[73] = 32;
	input[74] = 33;	
	//16
	input[75] = 1;	
	input[76] = 46;
	input[77] = 32;
	input[78] = 97;
	input[79] = 33;
	//17
	input[80] = 1;
	input[81] = 32;
	input[82] = 97;
	input[83] = 46;
	input[84] = 33;
	//18
	input[85] = 1;
	input[86] = 32;
	input[87] = 46;
	input[88] = 97;
	input[89] = 33;
	//19
	input[90] = 32;
	input[91] = 97;
	input[92] = 46;
	input[93] = 1;
	input[94] = 33;
	//20
	input[95] = 32;
	input[96] = 97;
	input[97] = 1;
	input[98] = 46;
	input[99] = 33;
	//21
	input[100] = 32;
	input[101] = 46;
	input[102] = 97;
	input[103] = 1;
	input[104] = 33;
	//22
	input[105] = 32;
	input[106] = 46;	
	input[107] = 1;
	input[108] = 97;
	input[109] = 33;
	//23
	input[110] = 32;
	input[111] = 1;
	input[112] = 97;
	input[113] = 46;
	input[114] = 33;
	//24
	input[115] = 32;
	input[116] = 1;
	input[117] = 46;
	input[118] = 97;
	input[119] = 33;
	
	//END char
	input[120] = 0;*/
    char* input = "a.\001 !a. \001!a\001. !a\001 .!a .\001!a \001.!.a\001 !.a \001!.\001a !.\001 a!. a\001!. \001a!\001a. !\001a .!\001.a !\001. a!\001 a.!\001 .a! a.\001! a\001.! .a\001! .\001a! \001a.! \001.a!";
    // This function prints the reference implementation output on the terminal
    print_camelCaser(input);

    // Enter your expected output as you would in camelCaser_utils.c
    char **output = NULL;
    // This function compares the expected output you provided against
    // the output from the reference implementation
    // The function returns 1 if the outputs match, and 0 if the outputs
    // do not match
    return check_output(input, output);
}
