#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <iostream>
using namespace std;

#pragma warning(disable: 4996)

int main(int argc, char* argv[])
{
  WSAData wsaData;
  WORD DLLVersion = MAKEWORD(2, 1);
  if (WSAStartup(DLLVersion, &wsaData) != 0)
  {
    cerr << "WSAStartup failed!" << endl;
    return -1;
  }

  SOCKADDR_IN addr;
  int sizeofaddr = sizeof(addr);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(1111);
  addr.sin_family = AF_INET;

  SOCKET connection = socket(AF_INET, SOCK_STREAM, NULL);
  if (connect(connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
  {
    cerr << "Error connect!" << endl;
      return 1;
  }

  cout << "Connected!\n";
  char msg[256];
  recv(connection, msg, sizeof(msg), NULL);
  system("pause");
  return 0;
}