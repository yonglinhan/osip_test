#include <iostream>
#include <eXosip2/eXosip.h>
#include <stdio.h>
#include <stdlib.h>
//#include <winsock2.h>
#include <winsock.h>
using namespace std;

int main()
{
	int i = -1;

	TRACE_INITIALIZE(6, stdout);
	i = eXosip_init();

	if (i != 0)
		return -1;
	i = eXosip_listen_addr(IPPROTO_UDP,NULL, 8888, AF_INET, 0);

	if (i != 0)
	{
		eXosip_quit();
		fprintf(stderr, "could not initialize transport layer \n");
		return -1;
	}

	eXosip_event *je;
	while (1)
	{
		je = eXosip_event_wait(0, 500);
		eXosip_lock();
		eXosip_default_action(je);
		eXosip_automatic_action();
		eXosip_unlock();

		if (je == NULL)
		{
			cout << "no connect..." << endl;
			continue;
		}
		else
		{
			cout << "je id: " << je->cid << "je did: " << je->did << endl;
		}
		if (je->type == EXOSIP_CALL_INVITE)
		{
			cout << "EXOSIP_CALL_INVITE " << endl;
		}
		else if (je->type == EXOSIP_CALL_ACK)
		{
			cout << "EXOSIP_CALL_ACK " << endl;
		}
		else if (je->type == EXOSIP_CALL_ANSWERED)
		{
			cout << "EXOSIP_CALL_ANSWERED " << endl;
		}
		else
		{
			cout << "other situation" << endl;
		}

		eXosip_event_free(je);
	}

	//cout << "test" << endl;
	system("pause");
	return 0;
}