#include "client.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
	try
	{
		Client client;
		client.start();
	}
	catch (const std::exception& ex)
	{
		cerr << ex.what() << endl;
	}

  system("pause");
  return 0;
}