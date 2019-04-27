#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "ExosipServer.h"

using namespace std;

int main()
{
	ExosipServer *server = new ExosipServer();

	if (server->ServerInit() == server->INIT_SUCCESS)
	{
		/*cout << "Server init success" << endl;
		server->ServerListen();
		server->ServerListen();*/

		//¿ØÖÆ
		while (1)
		{
			cout << "ÊäÈë¿ØÖÆu,d,l,r,z,x" << endl;
			char comm;
			cin >> comm;
			switch (comm)
			{
			case 'u':
				server->ServerTurnControl("34020000001320000001", server->UP);
				server->ServerListen();
				break;
			case 'd':
				server->ServerTurnControl("34020000001320000001", server->DOWN);
				server->ServerListen();
				break;
			case 'l':
				server->ServerTurnControl("34020000001320000001", server->LEFT);
				server->ServerListen();
				break;
			case 'r':
				server->ServerTurnControl("34020000001320000001", server->RIGHT);
				server->ServerListen();
				break;
			case 'z':
				server->ServerTurnControl("34020000001320000001", server->ZOOMLARGE);
				server->ServerListen();
				break;
			case 'x':
				server->ServerTurnControl("34020000001320000001", server->ZOOMLITTLE);
				server->ServerListen();
				break;

			default:
				server->ServerTurnControl("34020000001320000001", server->STOP_CONTROL);
				server->ServerListen();
				break;
			}
		}
		
	}
		
	delete server;

	system("pause");
	return 0;
}