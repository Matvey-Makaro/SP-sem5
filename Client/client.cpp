#pragma comment(lib, "ws2_32.lib")
#include "client.h"

#include <iostream>
#include <exception>

using namespace std;

#pragma warning(disable: 4996)

void Client::start()
{
  init();
  fillAddr();
  makeConnection();
  cout << "Connected!\n";
  mainLoop();
}

void Client::fillAddr()
{
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(1111);
  addr.sin_family = AF_INET;
}

void Client::makeConnection()
{
  connection = socket(AF_INET, SOCK_STREAM, NULL);
  if (connect(connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
    throw runtime_error("Error connect!");
}

void Client::mainLoop()
{
  while (true)
  {
    cout << "Enter your name: ";
    string name;
    getline(cin, name);
    sendPacketType(PT_ClientName);
    int nameSize = name.size();
    send(connection, reinterpret_cast<const char*>(&nameSize), sizeof(nameSize), NULL);
    send(connection, name.c_str(), nameSize, NULL);

    ServerAnswers answer = getServerAnswer();
    if (answer == SA_OK)
    {
      cout << "Login completed successfully." << endl;
      break;
    }
    else if (answer == SA_SUCH_NAME_TAKEN)
      cout << "This name is already taken." << endl;
    else if (answer == SA_ERROR_CLIENT_NAME_EXPECTED)
      throw runtime_error("Client name expected!");
    else throw runtime_error("Unknown error!");
  }
}

void Client::sendPacketType(PacketType packetType)
{
  send(connection, reinterpret_cast<const char*>(&packetType), sizeof(packetType), NULL);
}

PacketType Client::getPacketType()
{
  PacketType packetType;
  recv(connection, reinterpret_cast<char*>(&packetType), sizeof(packetType), NULL);
  return packetType;
}

void Client::sendAnswer(ClientAnswers answer)
{
  send(connection, reinterpret_cast<const char*>(&answer), sizeof(answer), NULL);
}

ServerAnswers Client::getServerAnswer()
{
  ServerAnswers answer;
  recv(connection, reinterpret_cast<char*>(&answer), sizeof(answer), NULL);
  return answer;
}

void Client::init()
{
  WSAData wsaData;
  WORD DLLVersion = MAKEWORD(2, 1);
  if (WSAStartup(DLLVersion, &wsaData) != 0)
    throw runtime_error("WSAStartup failed!");
}
