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
  void login();
  void sendMessage();
  void getMessages();
  void mainLoop();
  void sendPacketType(PacketType packetType);
  PacketType getPacketType();
  void sendAnswer(ClientAnswers answer);
  ServerAnswers getServerAnswer();
  void sendString(const std::string& str);
  std::string getString();
  void recv_s(SOCKET s, char* buf, int len, int flags = NULL);
  void send_s(SOCKET s, const char* buf, int len, int flags = NULL);

private:
  SOCKADDR_IN addr;
  SOCKET connection;
};