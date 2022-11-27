#pragma comment(lib, "ws2_32.lib")

#include "server.h"

#include <iostream>
#include <exception>
#include <memory>
using namespace std;

#pragma warning(disable: 4996)

void Server::start()
{
  init();
  fillAddr();
  sizeofaddr = sizeof(addr);
  createListener();

  SOCKET newConnection;
  while (true)
  {
    newConnection = accept(sListener, (SOCKADDR*)&addr, &sizeofaddr);
    if (newConnection == INVALID_SOCKET)
    {
      cerr << "Error accept new connection! " << WSAGetLastError() << endl;
      continue;
    }


    threads.emplace_back([this, newConnection]() {
      clientHandler(newConnection);
      });

  }



  cout << "Client connected!\n";
  char msg[256] = "Hello, client!";
  send(newConnection, msg, sizeof(msg), NULL);

  for (auto& t : threads)
    t.join();
}

void Server::init()
{
  WSAData wsaData;
  WORD DLLVersion = MAKEWORD(2, 1);
  if (WSAStartup(DLLVersion, &wsaData) != 0)
    throw std::runtime_error("WSAStartup failed!");
}

void Server::fillAddr()
{
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(1111);
  addr.sin_family = AF_INET;
}

void Server::createListener()
{
  sListener = socket(AF_INET, SOCK_STREAM, NULL);
  if (sListener == INVALID_SOCKET)
    throw runtime_error("Socket creation failed! " + std::to_string(WSAGetLastError()));

  bind(sListener, (SOCKADDR*)&addr, sizeofaddr);
  listen(sListener, SOMAXCONN);
}

void Server::clientNameHandler(SOCKET currConnection)
{
  PacketType packetType = getPacketType(currConnection);
  if (packetType != PT_ClientName)
  {
    sendAnswer(currConnection, SA_ERROR_CLIENT_NAME_EXPECTED);
    throw runtime_error("Client name expected for SOCKET: " + to_string(currConnection));
  }

  
  while (true)
  {
    std::string name = getStringFromClient(currConnection);

    {
      lock_guard<mutex> g(nameToSocketMutex);
      if (nameToSocket.count(name) == 0)
      {
        nameToSocket[name] = currConnection;
        sendAnswer(currConnection, SA_OK);
        cout << name << " connected.\n";
        break;
      }
    }
    
    sendAnswer(currConnection, SA_SUCH_NAME_TAKEN);
  }
}

string Server::receiverNameHandler(SOCKET currConnection)
{
  while (true)
  {
    string name = getStringFromClient(currConnection);

    {
      lock_guard<mutex> g(nameToSocketMutex);
      if (nameToSocket.count(name) != 0)
      {
        sendAnswer(currConnection, SA_OK);
        return name;
      }
    }

    sendAnswer(currConnection, SA_NO_SUCH_NAME);
  }
}

void Server::chatMessageHandler(SOCKET currConnection, const string& sender)
{
  string message = getStringFromClient(currConnection);
  sendPacketType(currConnection, PT_MessageWithReveiverName);
  const int messageSize = message.size();
  send(currConnection, reinterpret_cast<const char*>(&messageSize), sizeof(messageSize), NULL);
  send(currConnection, message.c_str(), messageSize, NULL);
  const int receiverSize = sender.size();
  send(currConnection, reinterpret_cast<const char*>(&receiverSize), sizeof(receiverSize), NULL);
  send(currConnection, sender.c_str(), receiverSize, NULL);
  
  ClientAnswers answer = getClientAnswer(currConnection);
  if (answer != CA_OK)
    cerr << "The message from" << sender << " did not reach." << endl;
}

void Server::clientHandler(SOCKET currConnection)
{
  std::cout << "Client handler!" << std::endl;

  clientNameHandler(currConnection);

  PacketType packetType = getPacketType(currConnection);
  string receiverName;
  switch (packetType)
  {
  case PT_ReceiverName:
    receiverName = receiverNameHandler(currConnection);
    break;
  case PT_ChatMessage:
    chatMessageHandler(currConnection, receiverName);
    break;
  default:
    cerr << "Unknown packet type" << endl;
    // return -5;
    break;
  }
}

void Server::sendAnswer(SOCKET socket, ServerAnswers answer)
{
  send(socket, reinterpret_cast<const char*>(&answer), sizeof(answer), NULL);
}

ClientAnswers Server::getClientAnswer(SOCKET socket)
{
  ClientAnswers clientAnswer;
  recv(socket, reinterpret_cast<char*>(&clientAnswer), sizeof(clientAnswer), NULL);
  return clientAnswer;
}

PacketType Server::getPacketType(SOCKET socket)
{
  PacketType packetType;
  recv(socket, reinterpret_cast<char*>(&packetType), sizeof(packetType), NULL);
  return packetType;
}

void Server::sendPacketType(SOCKET socket, PacketType packetType)
{
  send(socket, reinterpret_cast<const char*>(&packetType), sizeof(packetType), NULL);
}

std::string Server::getStringFromClient(SOCKET socket)
{
  constexpr int bufferSize = 1024;
  char buffer[bufferSize];
  int strSize = 0;
  recv(socket, reinterpret_cast<char*>(&strSize), sizeof(strSize), NULL);
  if (strSize <= 0)
    throw runtime_error("Size of string <= 0.");

  unique_ptr<char> dynamic_buffer;
  char* result_buffer;
  if (strSize >= bufferSize)
  {
    dynamic_buffer = unique_ptr<char>(new char[strSize + 1]);
    result_buffer = dynamic_buffer.get();
  }
  else result_buffer = buffer;

  result_buffer[strSize] = '\0';

  recv(socket, result_buffer, strSize, NULL);
  
  return result_buffer;
}
