#define check(actual, expected, msg) \
	if(((actual)) != ((expected))) { puts(msg); failures++; }

int f1() {
	unsigned char i, j, k;
	int failures = 0;

	i = 2; j = 3; k = i + j;
	check(k, 5, "add failed: u8 + u8 -> u8");

	i = 253; j = 3; k = i + j;
	check(k, 0, "add failed: u8 + u8 -> u8");

	i = 5; j = 3; k = i - j;
	check(k, 2, "subtract failed: u8 - u8 -> u8");

	i = 0; j = 3; k = i - j;
	check(k, 253, "subtract failed: u8 - u8 -> u8");

	return failures;
}

int f2() {
	unsigned char i, j;
	int k;
	int failures = 0;

	i = 2; j = 3; k = i + j;
	check(k, 5, "add failed: u8 + u8 -> int");

	// Desmet-ism, not standard C89
	i = 253; j = 3; k = i + j;
	check(k, 0, "add failed: u8 + u8 -> int");

	i = 5; j = 3; k = i - j;
	check(k, 2, "subtract failed: u8 - u8 -> int");

	// Desmet-ism, not standard C89
	i = 0; j = 3; k = i - j;
	check(k, 253, "subtract failed: u8 - u8 -> int");

	return failures;
}

int f3() {
	signed char i, j, k;
	int failures = 0;

	i = 2; j = -3; k = i + j;
	check(k, -1, "add failed: i8 + i8 -> i8");

	i = 125; j = 3; k = i + j;
	check(k, -128, "add failed: i8 + i8 -> i8");

	i = -1; j = -3; k = i - j;
	check(k, 2, "subtract failed: i8 - i8 -> i8");

	i = -128; j = 3; k = i - j;
	check(k, 125, "subtract failed: i8 - i8 -> i8");

	return failures;
}

int f4() {
	signed char i, j;
	int k;
	int failures = 0;

	i = 2; j = -3; k = i + j;
	check(k, -1, "add failed: i8 + i8 -> int");

	// Desmet-ism, not standard C89
	i = 125; j = 3; k = i + j;
	check(k, -128, "add failed: i8 + i8 -> int");

	i = -1; j = -3; k = i - j;
	check(k, 2, "subtract failed: i8 - i8 -> int");

	// Desmet-ism, not standard C89
	i = -128; j = 3; k = i - j;
	check(k, 125, "subtract failed: i8 - i8 -> int");

	return failures;
}

int f5() {
	unsigned char i;
	signed char j;
	int k;
	int failures = 0;

	// Desmet-ism: u8-i8 math yields a u8 result that is then
	// widened to int.
	i = 2; j = -3; k = i + j;
	check(k, 255, "add failed: u8 + i8 -> int");

	i = 125; j = 3; k = i + j;
	check(k, 128, "add failed: u8 + i8 -> int");

	i = 1; j = -3; k = i - j;
	check(k, 4, "subtract failed: u8 - i8 -> int");

	i = -128; j = 3; k = i - j;
	check(k, 125, "subtract failed: u8 - i8 -> int");

	return failures;
}

int f6() {
	signed char i, j;
	long k;
	int failures = 0;

	i = 2; j = -3; k = i + j;
	check(k, -1, "add failed: i8 + i8 -> long");

	// Desmet-ism, not standard C89
	i = 125; j = 3; k = i + j;
	check(k, -128, "add failed: i8 + i8 -> long");

	i = -1; j = -3; k = i - j;
	check(k, 2, "subtract failed: i8 - i8 -> long");

	// Desmet-ism, not standard C89
	i = -128; j = 3; k = i - j;
	check(k, 125, "subtract failed: i8 - i8 -> long");

	return failures;
}

int f7() {
	signed char i;
	int j, k;
	int failures = 0;

	i = 2; j = -3; k = i + j;
	check(k, -1, "add failed: i8 + int -> int");

	i = 125; j = 3; k = i + j;
	check(k, 128, "add failed: i8 + int -> int");

	i = -1; j = -3; k = i - j;
	check(k, 2, "subtract failed: i8 - int -> int");

	i = -128; j = 3; k = i - j;
	check(k, -131, "subtract failed: i8 - int -> int");

	return failures;
}

int f8() {
	signed char i;
	unsigned int j, k;
	int failures = 0;

	i = -2; j = 3; k = i + j;
	check(k, 1, "add failed: i8 + unsigned -> unsigned");

	i = 125; j = 4; k = i + j;
	check(k, 129, "add failed: i8 + unsigned -> unsigned");

	i = 5; j = 3; k = i - j;
	check(k, 2, "subtract failed: i8 - unsigned -> unsigned");

	i = -1; j = 1; k = i - j;
	check(k, 65534, "subtract failed: i8 - unsigned -> unsigned");

	return failures;
}

int f9() {
	signed char i;
	long j, k;
	int failures = 0;

	i = 2; j = -3; k = i + j;
	check(k, -1, "add failed: i8 + long -> long");

	i = 125; j = 3; k = i + j;
	check(k, 128, "add failed: i8 + long -> long");

	i = -1; j = -3; k = i - j;
	check(k, 2, "subtract failed: i8 - long -> long");

	i = -128; j = 3; k = i - j;
	check(k, -131, "subtract failed: i8 - long -> long");

	return failures;
}

int main()
{
	int failures = 0;
	failures += f1();
	failures += f2();
	failures += f3();
	failures += f4();
	failures += f5();
	failures += f6();
	failures += f7();
	failures += f8();
	failures += f9();
	printf("tests finished, %d failures.\n", failures);
	return 0;
}
