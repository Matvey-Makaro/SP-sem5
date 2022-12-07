#include <WinSock2.h>
#include <string>

enum PacketType
{
  PT_ClientName,
  PT_ReceiverName,
  PT_ChatMessage,
  PT_MessageWithReceiverName,
  PT_ClientGetNewMessages,
  PT_MessageWithSenderName,
  PT_StartSendClientMessages,
  PT_FinishSendClientMessages,
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

class Client
{
public:
  void start();

private:
  void init();
  void fillAddr();
  void makeConnection();
  void mainLoop();
  void sendPacketType(PacketType packetType);
  PacketType getPacketType();
  void sendAnswer(ClientAnswers answer);
  ServerAnswers getServerAnswer();
  void sendString(const std::string& str);
  std::string getString();

private:
  SOCKADDR_IN addr;
  SOCKET connection;
};