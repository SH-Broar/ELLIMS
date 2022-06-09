#include "DataBaseManager.h"
#include "SESSION.h"
#include "DBEvent.h"
#include "LoginData.h"
#include <Windows.h>
#include <stdio.h>  
#include <iostream>
#include <locale>
#include <sqlext.h>  

using namespace std;
concurrent_queue<DB_EVENT> DataBaseManager::dbWorkQueue;

void ShowError(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

DataBaseManager::DataBaseManager()
{

}

void DataBaseManager::DBThread()
{
	while (true)
	{
		DB_EVENT ev;
		
		while (!dbWorkQueue.try_pop(ev))
		{
			SleepEx(1, TRUE);
		}
		switch (ev.EVENT)
		{
		case DB_EVENT_TYPE::DB_EV_LOGIN:
		{
			LoginData d = getLoginData(ev.session.name, ev.session.pass);
			d.sc_id = ev.id;
			OVER_DB over;
			over._db_type = DB_EV_LOGIN;
			over._comp_type = OP_DB;
			over.datas = d;
			PostQueuedCompletionStatus(g_h_iocp, 0, d.sc_id, reinterpret_cast<LPOVERLAPPED>(&over));
		}
			break;
		case DB_EVENT_TYPE::DB_EV_LOGOUT:
			break;
		case DB_EVENT_TYPE::DB_EV_DUMMY:
			break;

		}
	}
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
	//string tmp{ name };
	//string tmp2{ password };

	////SQL Injection ¹æÁö

	//if (tmp.find('-') == string::npos || tmp.find('\'') == string::npos || tmp.find('/') == string::npos || tmp.find('\\') == string::npos ||
	//	tmp2.find('-') == string::npos || tmp2.find('\'') == string::npos || tmp2.find('/') == string::npos || tmp2.find('\\') == string::npos)
	//{
	//	result.level = -1;
	//	std::cout << "Don't SQL INJECTION!" << std::endl;
	//	return result;
	//}

	LoginData result{};

	setlocale(LC_ALL, "korean");
	//std::wcout.imbue(std::locale("korean"));

	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	constexpr int length = 10;
	SQLWCHAR id[100], Nname[100];
	SQLINTEGER level, x, y, HP, MaxHP, MP, MaxMP, EXP;
	SQLLEN cid, cNName, clevel, cx, cy, cHP, cMaxHP, cMP, cMaxMP, cEXP;

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
					wsprintf(proc, L"EXEC user_data %S, %S", name, password);
					printf("%S\n", proc);

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
									ShowError(hstmt, SQL_HANDLE_STMT, retcode);
								if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
								{
									//sprintf_s(result.id, "%S", id);
									sprintf_s(result.name, "%S", Nname);
									result.level = level;
									result.x = x;
									result.y = y;
									result.HP = HP;
									result.MaxHP = MaxHP;
									result.MP = MP;
									result.MaxMP = MaxMP;
									result.EXP = EXP;
									result.isValidLogin = true;
									cout << "DB Login Success : " << result.name << endl;
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

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
	return result;
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