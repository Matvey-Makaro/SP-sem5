#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <string>
using namespace std;

#pragma warning(disable: 4996)

enum PacketType
{
  PT_ClientName,
  PT_ReceiverName,
  PT_ChatMessage,
};

enum ServerAnswers
{
  SA_OK,
  SA_SUCH_NAME_TAKEN,
  SA_ERROR_CLIENT_NAME_EXPECTED,
  SA_NO_SUCH_NAME,
};

enum ClientAnswers
{
  CA_OK,
  CA_ERROR,
};

void sendPacketType(SOCKET socket, PacketType packetType)
{
  send(socket, reinterpret_cast<const char*>(&packetType), sizeof(packetType), NULL);
}

PacketType getPacketType(SOCKET socket)
{
  PacketType packetType;
  recv(socket, reinterpret_cast<char*>(&packetType), sizeof(packetType), NULL);
  return packetType;
}

void sendAnswer(SOCKET socket, ClientAnswers answer)
{
  send(socket, reinterpret_cast<const char*>(&answer), sizeof(answer), NULL);
}

ServerAnswers getServerAnswer(SOCKET socket)
{
  ServerAnswers answer;
  recv(socket, reinterpret_cast<char*>(&answer), sizeof(answer), NULL);
  return answer;
}

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

  while (true)
  {
    cout << "Enter your name: ";
    string name;
    getline(cin, name);
    sendPacketType(connection, PT_ClientName);
    int nameSize = name.size();
    send(connection, reinterpret_cast<const char*>(&nameSize), sizeof(nameSize), NULL);
    send(connection, name.c_str(), nameSize, NULL);

    ServerAnswers answer = getServerAnswer(connection);
    if (answer == SA_OK)
    {
      cout << "Login completed successfully." << endl;
      break;
    }
    else if (answer == SA_SUCH_NAME_TAKEN)
    {
      cout << "This name is already taken." << endl;
    }
    else if (answer == SA_ERROR_CLIENT_NAME_EXPECTED)
    {
      cerr << "Client name expected!" << endl;
      return -1;
    }
    else
    {
      cerr << "Unknown error." << endl;
      return -2;
    }
  }

  



  /*char msg[256];
  recv(connection, msg, sizeof(msg), NULL);
  cout << msg << endl;*/
  system("pause");
  return 0;
}