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

  SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
  bind(sListen, (SOCKADDR*)&addr, sizeofaddr);
  listen(sListen, SOMAXCONN);

  SOCKET newConnection;
  newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
  if (newConnection == 0)
  {
    cerr << "Error accept new connection!" << endl;
    return -2;
  }
  
  cout << "Client connected!\n";
  char msg[256] = "Hello, client!";
  send(newConnection, msg, sizeof(msg), NULL);


  system("pause");
  return 0;
}