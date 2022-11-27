#include <unordered_map>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <WinSock2.h>

using NameToSocket = std::unordered_map<std::string, SOCKET>;

enum PacketType
{
  PT_ClientName,
  PT_ReceiverName,
  PT_ChatMessage,
  PT_MessageWithReveiverName,
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

private:
  void init();
  void fillAddr();
  void createListener();
  void clientNameHandler(SOCKET currConnection);
  std::string receiverNameHandler(SOCKET currConnection);
  void chatMessageHandler(SOCKET currConnection, const std::string& sender);
  void clientHandler(SOCKET currConnection);
  void sendAnswer(SOCKET socket, ServerAnswers answer);
  ClientAnswers getClientAnswer(SOCKET socket);
  PacketType getPacketType(SOCKET socket);
  void sendPacketType(SOCKET socket, PacketType packetType);
  std::string getStringFromClient(SOCKET socket);

private:
  NameToSocket nameToSocket;
  std::mutex nameToSocketMutex;
  std::vector<std::thread> threads;
  SOCKET sListener;
  SOCKADDR_IN addr;
  int sizeofaddr;
};
