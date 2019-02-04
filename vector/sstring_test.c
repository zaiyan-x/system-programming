/**
 * Vector Lab
 * CS 241 - Spring 2019
 */
 
#include "sstring.h"
#include "vector.h"
int main(int argc, char *argv[]) {
    // TODO create some tests
	// TEST 1
	puts("TEST1");
	char* input_1 = "Test 1 My day is awful because 241";
	sstring* test_1 = cstr_to_sstring(input_1);
	char* test_1_return = sstring_to_cstr(test_1);
	printf("%s\n", test_1_return);
	//Clean UP	
	free(test_1_return);
	sstring_destroy(test_1);

	// TEST 2
	puts("TEST2");
	sstring* input_2_first = cstr_to_sstring("My best friend is: ");
	sstring* input_2_second = cstr_to_sstring("Duoduo. And Duoduo is beautiful");
	sstring_append(input_2_first, input_2_second);
	char* test_2_return = sstring_to_cstr(input_2_first);
	printf("%s\n", test_2_return);
	//Clean UP
	sstring_destroy(input_2_first);
	sstring_destroy(input_2_second);
	free(test_2_return);
	
	// TEST 3
	puts("TEST3:Hello-World-I-am-you--friend.");
	char del = '-';
	sstring* input_3 = cstr_to_sstring("Hello-World-I-am-you--friend.");

	vector* test_3_return = sstring_split(input_3, del);
	size_t i;
	for (i = 0; i < vector_size(test_3_return); i++) {
		printf("%zu:%s\n", i, vector_get(test_3_return, i));
	}
	
	//Clean UP
	vector_destroy(test_3_return);
	sstring_destroy(input_3);
	
	// TEST 4
	puts("TEST4:-Input-in-pu----Input");
	sstring* input_4 = cstr_to_sstring("-Input-in-pu----Input");

	vector* test_4_return = sstring_split(input_4, del);
	for (i = 0; i < vector_size(test_4_return); i++) {
		printf("%zu:%s\n", i, vector_get(test_4_return, i));
	}
	
	//Clean UP
	vector_destroy(test_4_return);
	sstring_destroy(input_4);
	

	// TEST 5
	puts("TEST5:I love (), () is my love");
	sstring* input_5 = cstr_to_sstring("I love (), () is my love.");
	sstring_substitute(input_5, 9, "()", "duoduo");
	char* test_5_return = sstring_to_cstr(input_5);
	puts(test_5_return);
	free(test_5_return);
	sstring_destroy(input_5);
	
	// TEST 6
	puts("TEST6:I love ||||||, |||||| is my love.");
	sstring* input_6 = cstr_to_sstring("I love ||||||, |||||| is my love.");
	sstring_substitute(input_6, 0, "||||||", "food");
	sstring_substitute(input_6, 0, "||||||", "food");
	char* test_6_return = sstring_to_cstr(input_6);
	puts(test_6_return);
	free(test_6_return);
	sstring_destroy(input_6);
	

	// TEST 7
	puts("TEST7:I love foooooooooood, duoduo is my love.");
	sstring* input_7 = cstr_to_sstring("I love foooooooooood, duoduo is my love.");
	char* test_7_return = sstring_slice(input_7, 22, 28);
	puts(test_7_return);
	sstring_destroy(input_7);
	free(test_7_return);
}
