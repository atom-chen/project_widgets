// remote.cpp : 定义控制台应用程序的入口点。
//
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <WinSock2.h>
#include "Common.h"
#include "ByteArray.h"
#include "SocketTCP.h"
#include "SocketTCPHandler.h"
//--------------------------------------------------------------------------
struct net_info {
	std::string ip;
	unsigned int port;
	std::string group;
	std::string flag;
	std::string content;

	net_info(void) {}

	net_info(const std::string& ip, unsigned int port, const std::string& group, const std::string& flag, const std::string& content)
		: ip(ip), port(port), group(group), flag(flag), content(content) {}

	bool encode(ByteArray& byteArray) {
		if (byteArray.write_string(ip) &&
			byteArray.write_uint(port) &&
			byteArray.write_string(group) &&
			byteArray.write_string(flag) &&
			byteArray.write_string(content)) {
			return true;
		}
		printf("byte array is overflow max size (%d)\n", byteArray.max_size());
		return false;
	}

	void decode(ByteArray& byteArray) {
		ip = byteArray.read_string();
		port = byteArray.read_uint();
		group = byteArray.read_string();
		flag = byteArray.read_string();
		content = byteArray.read_string();
	}
};
//--------------------------------------------------------------------------
std::string dateString(void) {
	struct tm date = Common::getDate();
	return Common::formatString("%04d-%02d-%02d %02d:%02d:%02d", date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec);
}
//--------------------------------------------------------------------------
void handleClient(std::string ip, unsigned int port, unsigned int tag, std::string group, std::string flag, std::string content) {
	if ("" == ip || 0 == port || "" == group || "" == flag) {
		printf("client configuration fail, please check (ip, port, group, flag) [%s]\n", dateString().c_str());
		return;
	}
	int socketfd = SocketTCP::create();
	if (socketfd < 0) {
		printf("client initialize fail (%s %d, %s %s) [%s]\n", ip.c_str(), port, group.c_str(), flag.c_str(), dateString().c_str());
		return;
	}
	printf("client initialize success (%s %d, %s %s) [%s]\n", ip.c_str(), port, group.c_str(), flag.c_str(), dateString().c_str());
	if (!SocketTCP::startClient(socketfd, ip.c_str(), port)) {
		printf("client start fail (%s %d, %s %s) [%s]\n", ip.c_str(), port, group.c_str(), flag.c_str(), dateString().c_str());
		SocketTCP::close(socketfd);
		return;
	}
	printf("client start success (%s %d, %s %s) [%s]\n", ip.c_str(), port, group.c_str(), flag.c_str(), dateString().c_str());
	ByteArray bytes;
	SocketTCPHandler stcpHandler;
	while (true) {
		bytes.reuse();
		int length = stcpHandler.recv(socketfd, bytes);
		if (length < 0) {
			break;
		} else if (0 == length || 0 == bytes.getCurLength()) {
			continue;
		}
		net_info recvInfo;
		recvInfo.decode(bytes);
		if ("SERVER_STATUS_BUSY" == recvInfo.content) {
			printf("[%s] %s\n", dateString().c_str(), recvInfo.content.c_str());
			break;
		} else if ("SERVER_STATUS_FREE" == recvInfo.content) {
			printf("[%s] %s\n", dateString().c_str(), recvInfo.content.c_str());
			long msgLength = 0;
			char* fileData = (char*)Common::getFileData(content, &msgLength);
			net_info sendInfo(recvInfo.ip, recvInfo.port, group, flag, "");
			if (NULL == fileData) {
				sendInfo.content = content;
			} else {
				sendInfo.content = fileData;
				delete fileData;
				fileData = NULL;
			}
			bytes.reuse();
			std::string tempContent = 0 == tag ? "CONTENT_HIDE" : (std::string(content.empty() ? "" : "\n") + content);
			if (sendInfo.encode(bytes)) {
				if (stcpHandler.send(socketfd, bytes) > 0) {
					printf("send success: [%s] %s\n", dateString().c_str(), tempContent.c_str());
				} else {
					printf("send fail: [%s] %s\n", dateString().c_str(), tempContent.c_str());
				}
			} else if (msgLength > 0) {
				sendInfo.content = content;
				bytes.reuse();
				if (sendInfo.encode(bytes)) {
					if (stcpHandler.send(socketfd, bytes) > 0) {
						printf("send success: [%s] %s\n", dateString().c_str(), tempContent.c_str());
					} else {
						printf("send fail: [%s] %s\n", dateString().c_str(), tempContent.c_str());
					}
				} else {
					printf("send fail (overflow): [%s] %s\n", dateString().c_str(), tempContent.c_str());
				}
			}
			break;
		}
	}
	SocketTCP::close(socketfd);
}
//--------------------------------------------------------------------------
static std::map<int, net_info> sNetInfoMap;
static std::mutex sNetInfoMutex;
static unsigned int sTag = 0;
static unsigned int sHeartbeat = 0;

void serverPeerProcessFunc(int peerfd) {
	ByteArray bytes;
	SocketTCPHandler stcpHandler;
	std::map<int, net_info>::iterator iter = sNetInfoMap.find(peerfd);
	net_info heartbeatInfo;
	if (sNetInfoMap.end() != iter) {
		net_info sendInfo = iter->second;
		sendInfo.content = "SERVER_STATUS_FREE";
		if (sendInfo.encode(bytes)) {
			stcpHandler.send(peerfd, bytes);
		}
		heartbeatInfo = iter->second;
		heartbeatInfo.content = "SERVER_HEARTBEAT";
	}
	double heartbeatTime = Common::getTime();
	while (true) {
		double st = Common::getTime();
		if (sHeartbeat > 0 && st - heartbeatTime >= sHeartbeat) {
			heartbeatTime = st;
			bytes.reuse();
			if (heartbeatInfo.encode(bytes)) {
				stcpHandler.send(peerfd, bytes);
			}
		}
		bytes.reuse();
		int length = stcpHandler.recv(peerfd, bytes);
		if (length < 0) {
			int err = WSAGetLastError();
			if (WSAEWOULDBLOCK == err) {
				continue;
			}
			break;
		} else if (0 == length || 0 == bytes.getCurLength()) {
			continue;
		}
		net_info recvInfo;
		recvInfo.decode(bytes);
		for (iter = sNetInfoMap.begin(); sNetInfoMap.end() != iter; ++iter) {
			if (peerfd == iter->first) {
				std::string tempContent = 0 == sTag ? "CONTENT_HIDE" : (std::string(recvInfo.content.empty() ? "" : "\n") + recvInfo.content);
				printf("(%s %d, %s %s): [%s] %s\n", recvInfo.ip.c_str(), recvInfo.port, recvInfo.group.c_str(), recvInfo.flag.c_str(), dateString().c_str(), tempContent.c_str());
			} else if ("0" == recvInfo.group || "1" == recvInfo.group || "0" == iter->second.group || "2" == iter->second.group || recvInfo.group == iter->second.group) {
				bytes.reuse();
				if (recvInfo.encode(bytes)) {
					stcpHandler.send(iter->first, bytes);
				}
			}
		}
	}
	sNetInfoMutex.lock();
	iter = sNetInfoMap.find(peerfd);
	if (sNetInfoMap.end() != iter) {
		printf("client (%s %d) [%s] offline\n", iter->second.ip.c_str(), iter->second.port, dateString().c_str());
		sNetInfoMap.erase(iter);
	}
	SocketTCP::close(peerfd, false);
	sNetInfoMutex.unlock();
}

void handleServer(std::string ip, unsigned int port, unsigned int tag, unsigned int count, unsigned int heartbeat) {
	if (ip.empty() || 0 == port) {
		printf("server configuration fail, please check (ip port) [%s]\n", dateString().c_str());
		return;
	}
	int socketfd = SocketTCP::create();
	if (socketfd < 0) {
		printf("server initialize fail (%s %d) [%s]\n", ip.c_str(), port, dateString().c_str());
		return;
	}
	printf("server initialize success (%s %d) [%s]\n", ip.c_str(), port, dateString().c_str());
	if (!SocketTCP::startServer(socketfd, ip.c_str(), port)) {
		printf("server start fail (%s %d) [%s]\n", ip.c_str(), port, dateString().c_str());
		SocketTCP::close(socketfd);
		return;
	}
	printf("server start success (%s %d) [%s]\n", ip.c_str(), port, dateString().c_str());
	sTag = tag;
	sHeartbeat = heartbeat;
	sockaddr_in peeraddr;
	ByteArray bytes;
	SocketTCPHandler stcpHandler;
	while (true) {
		memset(&peeraddr, 0, sizeof(peeraddr));
		int peerfd = SocketTCP::accept(socketfd, &peeraddr);
		if (peerfd < 0 || 0 != SocketTCP::setBlock(peerfd, false)) {
			SocketTCP::close(peerfd, false);
			continue;
		}
		net_info info(inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), "", "", "");
		if (sNetInfoMap.size() >= (count = count < 2 ? 2 : count)) {
			printf("server accept upper limit, only support %d client [%s]\n", count, dateString().c_str());
			bytes.reuse();
			info.content = "SERVER_STATUS_BUSY";
			if (info.encode(bytes)) {
				stcpHandler.send(peerfd, bytes);
			}
			SocketTCP::close(peerfd, false);
			continue;
		}
		printf("client (%s %d) [%s] online\n", info.ip.c_str(), info.port, dateString().c_str());
		sNetInfoMap.insert(std::make_pair(peerfd, info));
		std::thread peerThread(serverPeerProcessFunc, peerfd);
		peerThread.detach();
	}
	SocketTCP::close(socketfd);
}
//--------------------------------------------------------------------------
int main(int argc, char* argv[]) {
	std::string type = "server";	// 类型:client,server
	std::string ip = "127.0.0.1";	// 地址,如:127.0.0.1
	unsigned int port = 4096;		// 端口,如:4096
	unsigned int tag = 1;			// 标签:0.隐藏内容,1.显示内容
	std::string group = "0";		// 客户端组别,当type为client时有效,如:0(可发送/接收所有组别),1(可发送所有组别),2(可接收所有组别),...(其他,只能和相同的组别通信)
	std::string flag = "code";		// 客户端内容标志,当type为client时有效,表示content的自定义标志,如:code,tplt,...
	std::string content = "";		// 客户端发送内容,当type为client时有效,文件路径(则发送文件内容,若文件内容过大,则发送文件路径)或字符串内容(直接发送),如:F:\temp.txt,hello
	unsigned int count = 2;			// 服务端连接数,当type为server时有效,指定可连接服务端的客户端数量,>=2
	unsigned int heartbeat = 0;		// 服务端心跳间隔,当type为server时有效(默认没有心跳)
	for (int i=1; i<argc; ++i) {
		std::string cmd = argv[i];
		if ("-type" == cmd && i + 1 < argc) {
			type = argv[i + 1];
			continue;
		}
		if ("-ip" == cmd && i + 1 < argc) {
			ip = argv[i + 1];
			continue;
		}
		if ("-port" == cmd && i + 1 < argc) {
			port = atoi(argv[i + 1]);
			continue;
		}
		if ("-tag" == cmd && i + 1 < argc) {
			tag = atoi(argv[i + 1]);
			continue;
		}
		if ("-group" == cmd && i + 1 < argc) {
			group = argv[i + 1];
			continue;
		}
		if ("-flag" == cmd && i + 1 < argc) {
			flag = argv[i + 1];
			continue;
		}
		if ("-content" == cmd && i + 1 < argc) {
			content = argv[i + 1];
			continue;
		}
		if ("-count" == cmd && i + 1 < argc) {
			count = atoi(argv[i + 1]);
			continue;
		}
		if ("-heartbeat" == cmd && i + 1 < argc) {
			heartbeat = atoi(argv[i + 1]);
			continue;
		}
	}
	if ("client" != type && "server" != type) {
		printf("type '%s' is wrong, please set 'client' or 'server' !!!\n", type.c_str());
		return 0;
	}
	if (ip.empty() || port > 65535) {
		printf("ip '%s' or port '%d' is wrong, e.g. -ip 127.0.0.1 -port [0, 65535] !!!\n", ip.c_str(), port);
		return 0;
	}
	if ("client" == type) {
		handleClient(ip, port, tag, group, flag, content);
	} else if ("server" == type) {
		handleServer(ip, port, tag, count, heartbeat);
	}
	return 0;
}
//--------------------------------------------------------------------------