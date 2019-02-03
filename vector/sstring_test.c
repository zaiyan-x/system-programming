/**
 * Vector Lab
 * CS 241 - Spring 2019
 */
 
#include "sstring.h"
#include "vector.h"
int main(int argc, char *argv[]) {
    // TODO create some tests
	// TEST 1
	char* input_1 = "Test 1 My day is awful because 241";
	sstring* test_1 = cstr_to_sstring(input_1);
	char* test_1_return = sstring_to_cstr(test_1);
	printf("%s\n", test_1_return);	

	// TEST 2
	sstring* input_2_first = cstr_to_sstring("My best friend is: ");
	sstring* input_2_second = cstr_to_sstring("Duoduo. And Duoduo is beautiful");
	sstring_append(input_2_first, input_2_second);
	char* test_2_return = sstring_to_cstr(input_2_first);
	printf("%s\n", test_2_return);




















	return 0;
}
