#include "DataBaseManager.h"
#include "SESSION.h"
#include "DBEvent.h"
#include "LoginData.h"
#include "AMBIT.h"
#include <Windows.h>
#include <stdio.h>  
#include <iostream>
#include <locale>
#include <sqlext.h>  

using namespace std;
concurrent_queue<DB_EVENT> DataBaseManager::dbWorkQueue;
static SQLHENV henv;
static SQLHDBC hdbc;
static SQLHSTMT hstmt;

void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

DataBaseManager::DataBaseManager()
{

}

#define STRESSTEST
void DataBaseManager::DBThread()
{
	SQLRETURN retcode = 0;

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
		}
	}

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	while (true)
	{
		DB_EVENT ev;

		while (!dbWorkQueue.try_pop(ev))
		{
			//SleepEx(50, TRUE);
		}
		switch (ev.EVENT)
		{
		case DB_EVENT_TYPE::DB_EV_LOGIN:
		{
			LoginData d;
#ifdef STRESSTEST
			//			strcpy(d.name, "_DUMMY");
			//			d.x = rand() % W_WIDTH;
			//			d.y = rand() % W_HEIGHT;
			//			d.isValidLogin = true;
			//			d.isPlayer = true;
			//			sprintf_s(d.id, "%S", "DUMMY");
			//			sprintf_s(d.name, "%S", "DUMMY");
			//			d.level = 1;
			//			d.HP = 100;
			//			d.MaxHP = 100;
			//			d.MP = 2;
			//			d.MaxMP = 2;
			//			d.EXP = 1;
			//			d.race = 0;
			d = getLoginData(const_cast<char*>("test"), const_cast <char*>("test"));
			d.x = rand() % W_WIDTH;
			d.y = rand() % W_HEIGHT;
#else
			d = getLoginData(ev.session.name, ev.session.pass);
#endif
			d.sc_id = ev.id;

			OVER_DB over;
			over._db_type = DB_EV_LOGIN;
			over._comp_type = OP_DB;
			over.datas = d;
			PostQueuedCompletionStatus(g_h_iocp, 0, d.sc_id, reinterpret_cast<LPOVERLAPPED>(&over));
		}
		break;
		case DB_EVENT_TYPE::DB_EV_NEWUSER:
		{
			LoginData d;
			d = newLoginData(ev.session.name, ev.session.pass);
			d.sc_id = ev.id;

			OVER_DB over;
			over._db_type = DB_EV_LOGIN;
			over._comp_type = OP_DB;
			over.datas = d;
			PostQueuedCompletionStatus(g_h_iocp, 0, d.sc_id, reinterpret_cast<LPOVERLAPPED>(&over));
		}
		break;
		case DB_EVENT_TYPE::DB_EV_LOGOUT:
			setLoginData(ev.session);
			break;
		}
	}

	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);

}

void DataBaseManager::addDBEvent(int id, DB_EVENT_TYPE type, LoginData& sess)
{
	DB_EVENT ev;
	ev.id = id;
	ev.EVENT = type;
	ev.session = sess;
	dbWorkQueue.push(ev);
}

LoginData DataBaseManager::getLoginData(char* name, char* password)
{
	LoginData result{};
	result.level = -1;

	std::setlocale(LC_ALL, "korean");
	//std::wcout.imbue(std::locale("korean"));

	SQLRETURN retcode =0;

	constexpr int length = 10;
	SQLWCHAR id[100], Nname[100];
	SQLINTEGER level, x, y, HP, MaxHP, MP, MaxMP, EXP;
	SQLLEN cid, cNName, clevel, cx, cy, cHP, cMaxHP, cMP, cMaxMP, cEXP;




	// Set login timeout to 5 seconds  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{

		// Connect to data source  
		retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2017180021_gameserver", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

		// Allocate statement handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

			SQLWCHAR proc[100];
			wsprintf(proc, L"EXEC user_data %S, %S", name, password);
			//printf("%S\n", proc);

			retcode = SQLExecDirect(hstmt, proc, SQL_NTS);

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				{
					// Bind columns 1, 2, 3
					//retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &id, 10, &cid);
					retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &Nname, 20, &cNName);
					retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &level, 10, &clevel);
					retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &x, 10, &cx);
					retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &y, 10, &cy);
					retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &HP, 10, &cHP);
					retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &MaxHP, 10, &cMaxHP);
					retcode = SQLBindCol(hstmt, 7, SQL_C_LONG, &MP, 10, &cMP);
					retcode = SQLBindCol(hstmt, 8, SQL_C_LONG, &MaxMP, 10, &cMaxMP);
					retcode = SQLBindCol(hstmt, 9, SQL_C_LONG, &EXP, 10, &cEXP);

					// Fetch and print each row of data. On an error, display a message and exit.  
					for (int i = 0; ; i++) {
						retcode = SQLFetch(hstmt);
						if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
						{
							ShowError(hstmt, SQL_HANDLE_STMT, retcode);
						}
						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
						{
							sprintf_s(result.id, "%S", Nname);
							sprintf_s(result.name, "%S", Nname);
							result.level = level;
							result.isPlayer = true;
							result.x = x;
							result.y = y;
							result.HP = HP;
							result.MaxHP = MaxHP;
							result.MP = MP;
							result.MaxMP = MaxMP;
							result.EXP = EXP;
							result.isValidLogin = true;
							result.race = 0;
#ifdef STRESSTEST
							static int liter = 1;
							sprintf_s(result.id, "%s%d", "tester", liter);
							sprintf_s(result.name, "%s%d", "tester", liter);
							liter++;
#endif
							//cout << "DB Login Success : " << result.name << endl;
						}
						else
						{
							ShowError(hstmt, SQL_HANDLE_STMT, retcode);
							break;
						}
					}
				}
			}
			else {
				ShowError(hstmt, SQL_HANDLE_STMT, retcode);
			}
			// Process data  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLCancel(hstmt);
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			}
		}
	}
	return result;

}

LoginData DataBaseManager::newLoginData(char* name, char* password)
{
	LoginData result{};
	result.level = -1;

	std::setlocale(LC_ALL, "korean");
	//std::wcout.imbue(std::locale("korean"));

	SQLRETURN retcode;

	constexpr int length = 10;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2017180021_gameserver", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					SQLWCHAR proc[100];
					wsprintf(proc, L"EXEC new_user_data %S, %S", name, password);
					//printf("%S\n", proc);

					retcode = SQLExecDirect(hstmt, proc, SQL_NTS);

					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{

						// Fetch and print each row of data. On an error, display a message and exit.  

						if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
						{
							ShowError(hstmt, SQL_HANDLE_STMT, retcode);
						}
						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
						{
							sprintf_s(result.id, "%s", name);
							sprintf_s(result.name, "%s", name);
							result.level = 1;
							result.isPlayer = true;
							result.x = 200;
							result.y = 200;
							result.HP = 50;
							result.MaxHP = 50;
							result.MP = 10;
							result.MaxMP = 10;
							result.EXP = 0;
							result.isValidLogin = true;
							result.race = 0;
							//cout << "DB New User Success : " << result.name << endl;
						}
						else
						{
							ShowError(hstmt, SQL_HANDLE_STMT, retcode);
						}

					}
					else {
						ShowError(hstmt, SQL_HANDLE_STMT, retcode);
					}
					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
	return result;
}

bool DataBaseManager::setLoginData(LoginData& data)
{
	bool ret = true;
#ifndef STRESSTEST
	ret = false;

	setlocale(LC_ALL, "korean");
	//std::wcout.imbue(std::locale("korean"));

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	constexpr int length = 10;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2017180021_gameserver", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				// Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					SQLWCHAR proc[300];
					wsprintf(proc, L"EXEC set_user_data %S, %S, %d, %d, %d, %d, %d, %d, %d, %d", data.id, data.name, data.level, data.x, data.y, data.HP, data.MaxHP, data.MP, data.MaxMP, data.EXP);
					//printf("%S\n", proc);

					retcode = SQLExecDirect(hstmt, proc, SQL_NTS);

					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						{
							if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
								ShowError(hstmt, SQL_HANDLE_STMT, retcode);
							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
							{
								ret = true;
								//printf("%s : Save Success\n", data.id);
							}
						}
					}
					else {
						ShowError(hstmt, SQL_HANDLE_STMT, retcode);
					}
					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
#endif
	return ret;
}

void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	SQLWCHAR wszMessage[1000];
	SQLWCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}