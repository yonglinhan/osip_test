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
	i = eXosip_listen_addr(IPPROTO_UDP, NULL, 8889, AF_INET, 0);

	if (i != 0)
	{
		eXosip_quit();
		fprintf(stderr, "could not initialize transport layer \n");
		return -1;
	}

	osip_message_t *invite = NULL;

	i = eXosip_call_build_initial_invite(&invite, "sip:127.0.0.1:8888", "sip:127.0.0.1:8889", NULL, "this is a call for a conversation");

	if (i != 0)
	{
		return -1;
	}

	osip_message_set_supported(invite, "100rel");
	{
		char tmp[4096] = "";
		char localip[128] = "";
		eXosip_guess_localip(AF_INET, localip, 128);
		snprintf(tmp, 4096,
			"v=0\r\n"
			"o=josua 0 0 IN IP4 %s\r\n"
			"s=conversation\r\n"
			"c=IN IP4 %s\r\n"
			"t=0 0\r\n"
			"m=audio %s RTP/ACP0 8 101\r\n"
			"a=rtpmap:0 PCMU/8000\r\n"
			"a=rtpmap:8 PCMA/8000\r\n"
			"a=fmtp:101 0-11\r\n", localip, localip, "8888"
		);

		osip_message_set_body(invite, tmp, strlen(tmp));
		osip_message_set_content_type(invite, "application/sdp");
	}
	eXosip_lock();
	i = eXosip_call_send_initial_invite(invite);
	if (i > 0)
	{
		cout << "send successs" << endl;
		//eXosip_call_set_reference(i, NULL);
	}

	eXosip_event *je;
	while (1)
	{
		je = eXosip_event_wait(0, 500);
		eXosip_lock();
		eXosip_automatic_action();
		eXosip_unlock();

		if (je == NULL)
		{
			cout << "no connect..." << endl;
			continue;
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
		else if (je->type == EXOSIP_CALL_PROCEEDING)
		{
			cout << "EXOSIP_CALL_PROCEEDING " << endl;
		}
		else
		{
			cout << "other situation" << je->type << endl;
		}

		eXosip_event_free(je);
	}

	//cout << "test" << endl;
	system("pause");
	return 0;
}