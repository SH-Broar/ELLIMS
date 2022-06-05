#include "Network.h"
#include "Game.h"
#include "Player.h"

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

void Network::NetworkCodex()
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

	DWORD recv_flag = 0;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), g_h_iocp, 1, 0);

	CS_LOGIN_PACKET l_packet;


	//SERVER IMMIDIATE LOGIN
	int temp = 1;
	sprintf_s(l_packet.name, "Player%d", temp);
	l_packet.size = sizeof(l_packet);
	l_packet.type = CS_LOGIN;
	SendPacket(&l_packet);

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
				Game::printDebug("FAILED!", "LOGIN");
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
		Game::myPlayerID = 0;
		Game::playerIDMapper[login_packet->id] = Game::myPlayerID;
		Game::players.emplace_back(new Player());
		Game::players[Game::playerIDMapper[login_packet->id]]->p();
		Game::players[Game::playerIDMapper[login_packet->id]]->x = login_packet->x;
		Game::players[Game::playerIDMapper[login_packet->id]]->y = login_packet->y;
		Game::ingame = true;
		Game::printDebug("SUCCESS", "LOGIN");

	}
	break;
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* add_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(packet);
	}
	break;
	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* remove_packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(packet);
	}
	break;
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* move_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(packet);
	}
	break;
	case SC_CHAT:
	{
		SC_CHAT_PACKET* move_packet = reinterpret_cast<SC_CHAT_PACKET*>(packet);
	}
	break;
	default: Game::printDebug("Unknown Packet", "RECV");

	}
}