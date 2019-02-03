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
	//Clean UP	
	free(test_1_return);
	sstring_destroy(test_1);

	// TEST 2
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
	return 0;
}
