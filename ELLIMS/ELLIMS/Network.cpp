#include "Network.h"
#include "Game.h"

HANDLE Network::g_h_iocp;
SOCKET Network::g_c_socket;
OverlappedExtra Network::recv_over;

void Network::error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	printf("%S", lpMsgBuf);

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
		error_display("CONNETION ERROR", err_no);
	}

	recv_over._comp_type = OP_RECV;
	recv_over._wsabuf.buf = reinterpret_cast<CHAR*>(recv_over._send_buf);
	recv_over._wsabuf.len = sizeof(recv_over._send_buf);

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
			error_display("RECV ERROR", err_no);
		}
	}
}

void Network::SendPacket(void* packet)
{
	int psize = reinterpret_cast<unsigned char*>(packet)[0];
	int ptype = reinterpret_cast<unsigned char*>(packet)[1];

	OverlappedExtra* over = new OverlappedExtra;
	over->_comp_type = OP_SEND;
	memcpy(over->_send_buf, packet, psize);
	ZeroMemory(&over->_over, sizeof(over->_over));
	over->_wsabuf.buf = reinterpret_cast<CHAR*>(over->_send_buf);
	over->_wsabuf.len = psize;

	int ret = WSASend(g_c_socket, &over->_wsabuf, 1, NULL, 0, &over->_over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			error_display("SEND ERROR", err_no);
	}
	// std::cout << "Send Packet [" << ptype << "] To Client : " << cl << std::endl;
}