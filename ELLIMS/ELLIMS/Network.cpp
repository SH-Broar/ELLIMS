#include "Network.h"
#include "Game.h"
#include "Player.h"
#include "ChatDialogue.h"

HANDLE Network::g_h_iocp;
SOCKET Network::g_c_socket;
OverlappedExtra Network::recv_over;

unsigned char Network::packet_buf[BUF_SIZE] = {};
int Network::prev_packet_data = 0;
int Network::curr_packet_size = 0;

void Network::error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	Game::printDebug(msg);
	//printf("%S", lpMsgBuf);

	//MessageBox(hWnd, lpMsgBuf, L"ERROR", 0);
	LocalFree(lpMsgBuf);
	// while (true);
}

Network::Network()
{
	prev_packet_data = 0;
	curr_packet_size = 0;
}

Network::~Network()
{
	closesocket(g_c_socket);
	WSACleanup();
}

void Network::NetworkCodex(char* id, char* pass)
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	//
	g_c_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	//server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);

	if (WSAConnect(g_c_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr), NULL, NULL, NULL, NULL) != 0)
	{
		int err_no = WSAGetLastError();
		Game::printDebug("CONNETION ERROR", "IOCP");
		//error_display("CONNETION ERROR", err_no);
	}
	else
	{
		Game::networkConnected = true;
	}
	recv_over._comp_type = OP_RECV;
	recv_over._wsabuf.buf = reinterpret_cast<CHAR*>(recv_over._overlapped_buf);
	recv_over._wsabuf.len = sizeof(recv_over._overlapped_buf);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), g_h_iocp, 1, 0);

	TryLogin(id,  pass);
}

void Network::TryLogin(char* id, char* pass)
{
	recv_over._comp_type = OP_RECV;
	recv_over._wsabuf.buf = reinterpret_cast<CHAR*>(recv_over._overlapped_buf);
	recv_over._wsabuf.len = sizeof(recv_over._overlapped_buf);

	//SERVER IMMIDIATE LOGIN
	CS_LOGIN_PACKET l_packet;

	int temp = 1;

	//아이디
	strcpy_s(l_packet.name, id);
	strcpy_s(l_packet.pass, pass);

	l_packet.size = sizeof(l_packet) - sizeof(l_packet.pass) + strlen(pass) + 1;
	l_packet.type = CS_LOGIN;
	SendPacket(&l_packet);

	DWORD recv_flag = 0;
	//RECV LOGIN DATA
	int ret = WSARecv(g_c_socket, &recv_over._wsabuf, 1, NULL, &recv_flag, &recv_over._over, NULL);
	if (SOCKET_ERROR == ret) {
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
		{
			Game::printDebug("RECV ERROR", "IOCP");
		}
	}
}

void Network::SendPacket(void* packet)
{
	int psize = reinterpret_cast<unsigned char*>(packet)[0];
	int ptype = reinterpret_cast<unsigned char*>(packet)[1];

	OverlappedExtra* over = new OverlappedExtra;
	over->_comp_type = OP_SEND;
	memcpy(over->_overlapped_buf, packet, psize);
	ZeroMemory(&over->_over, sizeof(over->_over));
	over->_wsabuf.buf = reinterpret_cast<CHAR*>(over->_overlapped_buf);
	over->_wsabuf.len = psize;

	int ret = WSASend(g_c_socket, &over->_wsabuf, 1, NULL, 0, &over->_over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			Game::printDebug("SEND ERROR", "IOCP");
	}
	// std::cout << "Send Packet [" << ptype << "] To Client : " << cl << std::endl;
}

void Network::PacketProcess()
{
	DWORD io_size;
	ULONG_PTR ci;
	WSAOVERLAPPED* over;
	BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &io_size, &ci, reinterpret_cast<LPWSAOVERLAPPED*>(&over), INFINITE);
	OverlappedExtra* cross_over = reinterpret_cast<OverlappedExtra*>(over);
	// std::cout << "GQCS :";
	int client_id = static_cast<int>(ci);

	if (FALSE == ret) {

		Game::printDebug("FAILED", "GQCS");
		return;
		//Game::gameEnded();
	//int err_no = WSAGetLastError();

	}
	if (0 == io_size) {
		Game::printDebug("CONNECTION LOST", "GQCS");
		Game::gameEnded();
		return;
	}

	switch (cross_over->_comp_type)
	{
	case OP_SEND:
		delete over;
		break;
	case OP_RECV:
	{
		//패킷 재조립 (수정 필요)
		char* buf = recv_over._overlapped_buf;
		unsigned psize = curr_packet_size;
		unsigned pr_size = prev_packet_data;
		while (io_size > 0) {
			if (0 == psize) psize = buf[0];
			if (io_size + pr_size >= psize) {
				// 패킷 완성 가능
				unsigned char packet[BUF_SIZE];
				memcpy(packet, packet_buf, pr_size);
				memcpy(packet + pr_size, buf, psize - pr_size);

				//패킷 처리
				RecvPacketProcess(packet);

				io_size -= psize - pr_size;
				buf += psize - pr_size;
				psize = 0; pr_size = 0;
			}
			else {
				memcpy(packet_buf + pr_size, buf, io_size);
				pr_size += io_size;
				io_size = 0;
			}
		}
		curr_packet_size = psize;
		prev_packet_data = pr_size;
		DWORD recv_flag = 0;
		int ret = WSARecv(g_c_socket, &recv_over._wsabuf, 1, NULL, &recv_flag, &recv_over._over, NULL);
		if (SOCKET_ERROR == ret) {
			int err_no = WSAGetLastError();
			if (err_no != WSA_IO_PENDING)
			{
				error_display("RECV ERROR", err_no);
				Game::gameEnded();
			}
		}
	}
	break;
	case OP_PLAYER_MOVE:
		delete over;
		break;
	}

}

void Network::RecvPacketProcess(unsigned char packet[])
{
	switch (packet[1]) {
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* login_packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);

		if (login_packet->level < 0)
		{
			Game::printDebug("FAILED", "LOGIN ");
			Game::DBConnected = false;
		}
		else
		{
			Game::myPlayerID = login_packet->id;

			Game::playerIDMapper[login_packet->id] = Game::nowPlayerNums;

			Game::players[Game::playerIDMapper[login_packet->id]] = new Player();
			Game::players[Game::playerIDMapper[login_packet->id]]->p();
			Game::players[Game::playerIDMapper[login_packet->id]]->x = login_packet->x;
			Game::players[Game::playerIDMapper[login_packet->id]]->y = login_packet->y;
			Game::players[Game::playerIDMapper[login_packet->id]]->level = login_packet->level;
			Game::players[Game::playerIDMapper[login_packet->id]]->HP = login_packet->HP;
			Game::players[Game::playerIDMapper[login_packet->id]]->MaxHP = login_packet->MaxHP;
			Game::players[Game::playerIDMapper[login_packet->id]]->MP = login_packet->MP;
			Game::players[Game::playerIDMapper[login_packet->id]]->MaxMP = login_packet->MaxMP;
			Game::players[Game::playerIDMapper[login_packet->id]]->scID = login_packet->id;
			Game::players[Game::playerIDMapper[login_packet->id]]->EXP = login_packet->EXP;
			Game::players[Game::playerIDMapper[login_packet->id]]->MaxEXP = 100 + (login_packet->level * 100);
			strcpy(Game::players[Game::playerIDMapper[login_packet->id]]->name, login_packet->name);

			Game::ingame = true;

			char num[10];
			itoa(Game::myPlayerID, num, 10);
			Game::printDebug(num, "LOGIN SUCCESS");

			Game::nowPlayerNums++;
		}
	}
	break;
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* add_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(packet);
		Game::playerIDMapper[add_packet->id] = Game::nowPlayerNums;

		Game::players[Game::playerIDMapper[add_packet->id]] = new Player();
		Game::players[Game::playerIDMapper[add_packet->id]]->x = add_packet->x;
		Game::players[Game::playerIDMapper[add_packet->id]]->y = add_packet->y;
		strcpy(Game::players[Game::playerIDMapper[add_packet->id]]->name, add_packet->name);
		Game::players[Game::playerIDMapper[add_packet->id]]->scID = add_packet->id;
		Game::players[Game::playerIDMapper[add_packet->id]]->setPlayerActive(true);

		Game::nowPlayerNums++;
		char tmp[10];
		sprintf(tmp, "%d", add_packet->id);
		Game::printDebug("ADD", tmp);
	}
	break;
	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* remove_packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(packet);
		if (Game::playerIDMapper.contains(remove_packet->id))
		{
			Game::players[Game::playerIDMapper[remove_packet->id]]->setPlayerActive(false);
			Game::printDebug("REMOVE", "SERVER");
		}
	}
	break;
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* move_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(packet);

		if (Game::playerIDMapper.contains(move_packet->id))
		{
			Game::players[Game::playerIDMapper[move_packet->id]]->x = move_packet->x;
			Game::players[Game::playerIDMapper[move_packet->id]]->y = move_packet->y;
			//char tmp[10];
			//sprintf(tmp, "%d", move_packet->id);
			//Game::printDebug("MOVE", tmp);
		}

	}
	break;
	case SC_CHAT:
	{
		SC_CHAT_PACKET* chat_packet = reinterpret_cast<SC_CHAT_PACKET*>(packet);

		std::string chat{};

		if (strcmp(Game::players[Game::playerIDMapper[chat_packet->id]]->name, "_SYSTEM") == 0)
		{
			chat = chat_packet->mess;
		}
		else
		{
			chat = Game::players[Game::playerIDMapper[chat_packet->id]]->name;
			chat += " : ";
			chat += chat_packet->mess;
		}

		Game::printDebug(chat.c_str(), "CHAT");
		Game::chat.dialoguePrint(chat);
	}
	break;
	default:
		char tmp[10];
		sprintf(tmp, "%d", packet[1]);
		Game::printDebug(tmp, "Unknown Packet");
	}
}

void Network::SendMove(int dir)
{
	CS_MOVE_PACKET cs_move_packet;

	cs_move_packet.size = sizeof(cs_move_packet);
	cs_move_packet.type = CS_MOVE;
	cs_move_packet.direction = dir;
	cs_move_packet.client_time = 0;
	SendPacket(&cs_move_packet);
}

void Network::SendChat(char* mess)
{
	CS_CHAT_PACKET cs_chat_packet;

	cs_chat_packet.size = sizeof(cs_chat_packet) - sizeof(cs_chat_packet.mess) + strlen(mess) +1;
	cs_chat_packet.type = CS_CHAT;
	strcpy(cs_chat_packet.mess, mess);
	SendPacket(&cs_chat_packet);
}
