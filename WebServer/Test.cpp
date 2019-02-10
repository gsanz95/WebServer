#include "Test.h"
#include <iostream>

void Test::testClient()
{
	const Client* first = new Client(13000);
	const Client* second = new Client(13000);

	bool ret = *first == *second;
	std::cout << "These should be equal: ";
	printf("%s", ret ? "true" : "false");
	std::cout << std::endl;

	const Client* third = new Client(98992);
	ret = *first == *third;
	std::cout << "These should not be equal: ";
	printf("%s", ret ? "false" : "true");
	std::cout << std::endl;

}
