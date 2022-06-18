#include "Turboc.h"
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
	case SC_LOGIN_OK:
	{
		SC_LOGIN_OK_PACKET* login_packet = reinterpret_cast<SC_LOGIN_OK_PACKET*>(packet);

		if (login_packet->level < 0)
		{
			Game::printDebug("FAILED", "LOGIN ");
			Game::DBConnected = false;
		}
		else
		{
			Game::myPlayerID = login_packet->id;

			Game::playerIDMapper[login_packet->id] = Game::nowPlayerNums;

			Player* tmpPlayer = new Player();
			tmpPlayer->p();
			tmpPlayer->x = login_packet->x;
			tmpPlayer->y = login_packet->y;
			tmpPlayer->race = login_packet->race;
			tmpPlayer->level = login_packet->level;
			tmpPlayer->HP = login_packet->HP;
			tmpPlayer->MaxHP = login_packet->MaxHP;
			tmpPlayer->MP = login_packet->MP;
			tmpPlayer->MaxMP = login_packet->MaxMP;
			tmpPlayer->scID = login_packet->id;
			tmpPlayer->EXP = login_packet->EXP;
			tmpPlayer->MaxEXP = 100 + (login_packet->level * 100);
			strcpy(tmpPlayer->name, login_packet->name);

			Game::players[Game::playerIDMapper[login_packet->id]] = tmpPlayer;

			Game::ingame = true;

			char num[10];
			itoa(Game::myPlayerID, num, 10);
			Game::printDebug(num, "LOGIN SUCCESS");

			Game::nowPlayerNums++;
		}
	}
	break;
	case SC_LOGIN_FAIL:
	{
		SC_LOGIN_FAIL_PACKET* login_packet = reinterpret_cast<SC_LOGIN_FAIL_PACKET*>(packet);
		Game::printDebug("FAILED", "LOGIN ");
		Game::DBConnected = false;
		switch (login_packet->reason)
		{
		case 0:
			Game::print("                           ", SCREEN_WIDTH / 2 - 11, 26);
			Game::print("ID / Password Invaild... ", SCREEN_WIDTH / 2 - 12, 26);
			break;
		case 1:
			Game::print("                           ", SCREEN_WIDTH / 2 - 11, 26);
			Game::print("Already Playing ID. ", SCREEN_WIDTH / 2 - 10, 26);
			break;
		case 2:
			Game::print("                           ", SCREEN_WIDTH / 2 - 11, 26);
			Game::print("Server Full. Please Wait. ", SCREEN_WIDTH / 2 - 13, 26);
			break;
		}
	}
	break;
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* add_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(packet);
		Game::playerIDMapper[add_packet->id] = Game::nowPlayerNums;

		Player* tmpPlayer = nullptr;
		if (Game::players[Game::playerIDMapper[add_packet->id]] == nullptr)
		{
			tmpPlayer = new Player();
		}
		else
		{
			tmpPlayer = Game::players[Game::playerIDMapper[add_packet->id]];
		}
		tmpPlayer->x = add_packet->x;
		tmpPlayer->y = add_packet->y;
		tmpPlayer->scID = add_packet->id;
		tmpPlayer->setPlayerActive(true);
		strcpy(tmpPlayer->name, add_packet->name);
		tmpPlayer->HP = add_packet->hp;
		tmpPlayer->MaxHP = add_packet->hpmax;
		tmpPlayer->MP = add_packet->mp;
		tmpPlayer->MaxMP = add_packet->mpmax;
		tmpPlayer->race = add_packet->race;
		tmpPlayer->m();

		Game::players[Game::playerIDMapper[add_packet->id]] = tmpPlayer;

		Game::nowPlayerNums++;
		char tmp[10];
		sprintf(tmp, "%d", add_packet->id);
		Game::printDebug("ADD", tmp);
	}
	break;
	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* remove_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(packet);
		if (Game::playerIDMapper.contains(remove_packet->id))
		{
			Game::players[Game::playerIDMapper[remove_packet->id]]->setPlayerActive(false);
			char tmp[10];
			sprintf(tmp, "%d", remove_packet->id);
			Game::printDebug("REMOVE", tmp);
		}
	}
	break;
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* move_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(packet);

		if (Game::playerIDMapper.contains(move_packet->id))
		{
			Game::players[Game::playerIDMapper[move_packet->id]]->x = move_packet->x;
			Game::players[Game::playerIDMapper[move_packet->id]]->y = move_packet->y;
			//Game::printDebug("MOVE");
		}
	}
	break;
	case SC_CHAT:
	{
		SC_CHAT_PACKET* chat_packet = reinterpret_cast<SC_CHAT_PACKET*>(packet);

		std::string chat{};

		if (chat_packet->type == 3)
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
	case SC_STAT_CHANGE:
	{
		SC_STAT_CHANGE_PACKET* stat_change_packet = reinterpret_cast<SC_STAT_CHANGE_PACKET*>(packet);

		if (Game::playerIDMapper.contains(stat_change_packet->id))
		{
			Game::players[Game::playerIDMapper[stat_change_packet->id]]->level = stat_change_packet->level;
			Game::players[Game::playerIDMapper[stat_change_packet->id]]->EXP = stat_change_packet->exp;
			Game::players[Game::playerIDMapper[stat_change_packet->id]]->HP = stat_change_packet->hp;
			Game::players[Game::playerIDMapper[stat_change_packet->id]]->MaxHP = stat_change_packet->hpmax;
			Game::players[Game::playerIDMapper[stat_change_packet->id]]->MP = stat_change_packet->mp;
			Game::players[Game::playerIDMapper[stat_change_packet->id]]->MaxMP = stat_change_packet->mpmax;
		}
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

	if (mess[0] == '1')
	{
		cs_chat_packet.chat_type = 1;
	}
	else if (mess[0] == '2')
	{
		cs_chat_packet.chat_type = 2;
	}
	else
	{
		cs_chat_packet.chat_type = 0;
	}

	cs_chat_packet.size = sizeof(cs_chat_packet) - sizeof(cs_chat_packet.mess) + strlen(mess) +1;
	cs_chat_packet.type = CS_CHAT;
	strcpy(cs_chat_packet.mess, mess);
	SendPacket(&cs_chat_packet);
}

void Network::SendAttack()
{
	CS_ATTACK_PACKET cs_attack_packet;

	cs_attack_packet.size = sizeof(cs_attack_packet);
	cs_attack_packet.type = CS_ATTACK;
	cs_attack_packet.skilltype = 0;
	SendPacket(&cs_attack_packet);
}