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

void Client::init()
{
  WSAData wsaData;
  WORD DLLVersion = MAKEWORD(2, 1);
  if (WSAStartup(DLLVersion, &wsaData) != 0)
    throw runtime_error("WSAStartup failed!");
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
    sendString(name);

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

  while (true)
  {
    cout << "1) Send message.\n";
    cout << "2) Get messages.\n";
    int answer = 0;
    cin >> answer;
    if (answer == 1)
    {
      while (true)
      {
        cout << "Enter receiver: ";
        string receiver;
        cin.get();
        getline(cin, receiver);

        sendPacketType(PT_ReceiverName);
        sendString(receiver);

        auto serverAnswer = getServerAnswer();
        if (serverAnswer == SA_OK)
          break;
        else if (serverAnswer != SA_NO_SUCH_NAME)
          throw runtime_error("Unknown sever answer");

        cout << "No such user. Try again.\n";
      }
      

      cout << "Enter message:\n";
      string message;
      getline(cin, message);
      sendPacketType(PT_ChatMessage);
      sendString(message);

    }
    else if (answer == 2)
    {
      sendPacketType(PT_ClientGetNewMessages);
      auto packet_type = getPacketType();
      if (packet_type != PT_StartSendClientMessages)
        throw std::runtime_error("Unexpected packet type from server.");

      packet_type = getPacketType();
      cout << "New messages:" << endl;
      while (packet_type == PT_MessageWithSenderName)
      {
        string sender = getString();
        string messageBody = getString();

        cout << "Sender: " << sender << '\n';
        cout << "Body: " << messageBody << "\n\n";


        packet_type = getPacketType();
      }

      if (packet_type != PT_FinishSendClientMessages)
        throw std::runtime_error("Unexpected packet type from server.");

    }
    else cout << "Wrong input. Try again." << endl;
  }
}

void Client::sendPacketType(PacketType packetType)
{
  send_s(connection, reinterpret_cast<const char*>(&packetType), sizeof(packetType));
}

PacketType Client::getPacketType()
{
  PacketType packetType;
  recv_s(connection, reinterpret_cast<char*>(&packetType), sizeof(packetType));
  return packetType;
}

void Client::sendAnswer(ClientAnswers answer)
{
  send_s(connection, reinterpret_cast<const char*>(&answer), sizeof(answer));
}

ServerAnswers Client::getServerAnswer()
{
  ServerAnswers answer;
  recv_s(connection, reinterpret_cast<char*>(&answer), sizeof(answer));
  return answer;
}

void Client::sendString(const string& str)
{
  int size = str.size();
  send_s(connection, reinterpret_cast<const char*>(&size), sizeof(size));
  send_s(connection, str.c_str(), size);
}

string Client::getString()
{
  constexpr int bufferSize = 1024;
  char buffer[bufferSize];
  int strSize = 0;
  recv_s(connection, reinterpret_cast<char*>(&strSize), sizeof(strSize));
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

  recv_s(connection, result_buffer, strSize);

  return result_buffer;
}

void Client::recv_s(SOCKET s, char* buf, int len, int flags)
{
  auto recvResult = recv(s, buf, len, flags);
  if (recvResult == SOCKET_ERROR)
    throw runtime_error("Recv error: " + to_string(WSAGetLastError()));
}

void Client::send_s(SOCKET s, const char* buf, int len, int flags)
{
  auto sendResult = send(s, buf, len, flags);
  if (sendResult == SOCKET_ERROR)
    throw runtime_error("Send error: " + to_string(WSAGetLastError()));
}
