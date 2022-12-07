#include <unordered_map>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <WinSock2.h>

struct Message
{
  std::string sender;
  std::string body;
};

using NameToSocket = std::unordered_map<std::string, SOCKET>;
using Messages = std::vector<Message>;
using NameToMessages = std::unordered_map<std::string, Messages>;

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

class Server
{
public:
  void start();

  ~Server();

private:
  void init();
  void fillAddr();
  void createListener();
  std::string clientNameHandler(SOCKET currConnection);
  std::string receiverNameHandler(SOCKET currConnection);
  void chatMessageHandler(SOCKET currConnection, std::string sender, const std::string& receiver);
  void clientGetNewMessagesHandler(SOCKET currConnection, const std::string& currClientName);
  void clientHandler(SOCKET currConnection);
  void sendAnswer(SOCKET socket, ServerAnswers answer);
  ClientAnswers getClientAnswer(SOCKET socket);
  PacketType getPacketType(SOCKET socket);
  void sendPacketType(SOCKET socket, PacketType packetType);
  std::string getStringFromClient(SOCKET socket);
  void sendStringToClient(SOCKET socket, const std::string str);
  void recv_s(SOCKET s, char* buf, int len);

private:
  NameToSocket nameToSocket;
  std::mutex nameToSocketMutex;
  std::vector<std::thread> threads;
  SOCKET sListener;
  SOCKADDR_IN addr;
  int sizeofaddr;

  NameToMessages receiverToMessages;
};
