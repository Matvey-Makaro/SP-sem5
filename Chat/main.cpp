#include "server.h"

#include <iostream>
#include <exception>
using namespace std;

int main(int argc, char* argv[])
{
	try
	{
		Server server;
		server.start();
	}
	catch (const std::exception& ex)
	{
		cerr << ex.what() << endl;
	}

  system("pause");
  return 0;
}