#include "server.h"

#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
  Server server;
  server.start();

  system("pause");
  return 0;
}