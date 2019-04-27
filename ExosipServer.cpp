#include "ExosipServer.h"
#include <string>
#include <iostream>
using namespace std;

ExosipServer::ExosipServer(std::string ip, std::string port, int iport, std::string domain, string id)
{
	if (m_serverInfo == NULL)
	{
		m_serverInfo = new DeviceInfo;//初始化服务器信息
		m_serverInfo->m_Ip = ip;
		m_serverInfo->m_domain = domain;
		m_serverInfo->m_Port = port;
		m_serverInfo->m_iPort = iport;
		m_serverInfo->m_ID = id;
	}
}

int ExosipServer::ServerInit()
{
	int ret = INIT_FAIL;

	m_server = eXosip_malloc();
	if (eXosip_init(m_server) == OSIP_SUCCESS && m_serverInfo != NULL)
	{
		eXosip_set_option(m_server, EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, m_serverInfo->m_Ip.c_str());
		if (eXosip_listen_addr(m_server, IPPROTO_UDP, NULL, m_serverInfo->m_iPort, AF_INET, 0) == OSIP_SUCCESS)
			ret = INIT_SUCCESS;
		else
		{
			eXosip_quit(m_server);
			//osip_free(m_server);
			m_server = NULL;
		}
	}
	else
	{
		eXosip_quit(m_server);
		//osip_free(m_server);
		m_server = NULL;
	}

	return ret;
}

int ExosipServer::ServerListen(int m, int s)
{
	int ret = -1;
	bool flag = true;
	if (m_server == NULL)
		goto RESULT;


	while (flag)
	{
		m_event = eXosip_event_wait(m_server, m, s);
		if (m_event == NULL)
		{
			cout << "time out..." << endl;
			continue;
		}
		eXosip_lock(m_server);
		switch (m_event->type)
		{
		case EXOSIP_MESSAGE_NEW://收到一条新的message，判断其是否是registe请求
			PrintMsg(m_event->request);
			if (!strcmp(m_event->request->cseq->method, "REGISTER"))//这是一条注册请求
			{
				//将请求注册的设备信息，存储
				if (m_tmpDevice != NULL)
				{
					delete m_tmpDevice;
					m_tmpDevice = NULL;
				}
				m_tmpDevice = new Device();
				m_tmpDevice->SetDeviceInfo(m_event->request);
				ret = SendRegisteAnswer();
				if (ret == REGISTE_SUCCESS)
					flag = false;
			}
			break;
		case EXOSIP_MESSAGE_ANSWERED:
			cout << "Get Message Answered" << endl;
			if(m_event->response->status_code == 200)
				flag = false;
			break;
		default:
			cout << "Event type: " << m_event->type << endl;
			flag = false;
			break;
		}
		eXosip_unlock(m_server);
	}

RESULT:
	return ret;
}

int ExosipServer::ServerTurnControl(std::string id, int controlComm)
{
	int ret = -1;
	char command[64] = "";
	char xml_body[1024] = "";
	if (DeviceMap.find(id) == DeviceMap.end())
	{
		cout << "Device Not exist: " << id << endl;
	}

	osip_message_t *msg = NULL;
	eXosip_lock(m_server);
	ret = eXosip_message_build_request(m_server, &msg, "MESSAGE", GetToHeader(id).c_str(), GetFromHeader().c_str(), GetRouteHeade(id).c_str());
	if (ret != OSIP_SUCCESS)
		goto RESULT;
	osip_message_set_content_type(msg, "Application/MANSCDP+xml");
	
	switch (controlComm) {
	case LEFT:
		sprintf(command, "A50F01021F0000D6");
		break;
	case RIGHT:
		sprintf(command, "A50F01011F0000D5");
		break;
	case UP:
		sprintf(command, "A50F0108001F00DC");
		break;
	case DOWN:
		sprintf(command, "A50F0104001F00D8");
		break;
	case ZOOMLARGE:
		sprintf(command, "A50F0110000010D5");
		break;
	case ZOOMLITTLE:
		sprintf(command, "A50F0120000010E5");
		break;
	case STOP_CONTROL:
		sprintf(command, "A50F0100000000B5");
		break;
	default:
		sprintf(command, "A50F01021F0000D6");
	}
	
	sprintf(xml_body, "<?xml version=\"1.0\"?>\r\n"
		"<Control>\r\n"
		"<CmdType>DeviceControl</CmdType>\r\n"
		"<SN>11</SN>\r\n"
		"<DeviceID>34020000001320000001</DeviceID>\r\n"
		"<PTZCmd>%s</PTZCmd>"
		"</Control>\r\n", command);
	osip_message_set_body(msg, xml_body, strlen(xml_body));

	ret = eXosip_message_send_request(m_server, msg);
	if (ret == OSIP_SUCCESS)
		ret = CONTROL_SUCCESS;
	else
		cout << "send control fail" << endl;

RESULT:
	//osip_message_free(msg);
	eXosip_unlock(m_server);
	return ret;
}

ExosipServer::~ExosipServer()
{

	if (m_serverInfo != NULL)
	{
		delete m_serverInfo;
		m_serverInfo = NULL;
	}
	if (m_server != NULL)
	{
		eXosip_quit(m_server);
		//osip_free(m_server);
		m_server = NULL;
	}
}

void ExosipServer::PrintMsg(osip_message_t  *message)
{
	osip_contact_t *contact = NULL;
	osip_message_t *msg = NULL;
	osip_message_clone(message, &msg);
	osip_message_get_contact(msg, 0, &contact);

	cout << "From username: " << msg->from->url->username << endl;
	cout << "From host: " << msg->from->url->host << endl;
	cout << "port: " << contact->url->port << endl;
	cout << "TO username: " << msg->to->url->username << endl;
	cout << "TO host: " << msg->to->url->host << endl;
	cout << "Contact IP host: " << contact->url->host << endl;
	//cout << "From Passwd: " << msg->from->url->password << endl;
	cout << "Method: " << msg->cseq->method << endl;
	cout << "Method num: " << msg->cseq->number << endl;

	osip_contact_free(contact);
	//osip_message_free(msg);
}

int ExosipServer::SendRegisteAnswer()
{

	int ret = -1;
	int status = -1;
	osip_message_t *answer = NULL;
	DeviceInfo *info = NULL;
	if (m_tmpDevice->isAuthNull())
		status = 401;
	else
		status = 200;

	eXosip_lock(m_server);

	if (eXosip_message_build_answer(m_server, m_event->tid, status, &answer) == OSIP_SUCCESS)
	{
		info = m_tmpDevice->GetDeviceInfo();
		if (status == 401)
		{
			string authHead = "";
			authHead += "Digest realm=\"";
			authHead += info->m_ID;
			authHead += "\",nonce=\"";
			authHead += NONCE;
			authHead += "\",algorithm=\"";
			authHead += ALGORITHTHM;
			authHead += "\"";
			osip_message_set_header(answer, "WWW-Authenticate", authHead.c_str());
			//osip_message_set_www_authenticate(answer, authHead.c_str());
			cout << "send 401 unathorization" << endl;
		}
		else if (status == 200)
		{
			osip_message_set_contact(answer, GetFormatHeader().c_str());//<sip:id@ip:port>;expires=
			osip_message_set_date(answer, "2019-04-27");
			osip_message_set_expires(answer, "3600");
		}
		else
		{
		}

		ret = eXosip_message_send_answer(m_server, m_event->tid, status, answer);
	}
	else
	{
		ret = eXosip_message_send_answer(m_server, m_event->tid, 400, NULL);
	}

	eXosip_unlock(m_server);

	if (ret == OSIP_SUCCESS && status == 200)
	{
		ret = REGISTE_SUCCESS;
		osip_header_t *dst = NULL;
		osip_message_get_expires(m_event->request, 0, &dst);
		if (!strcmp(dst->hvalue, "0"))
		{
			//删除设备
			if (DeviceMap.find(m_tmpDevice->GetDeviceInfo()->m_ID) != DeviceMap.end())
			{
				DeviceMap.erase(m_tmpDevice->GetDeviceInfo()->m_ID);
				cout << "Delete Device: " << m_tmpDevice->GetDeviceInfo()->m_ID << endl;
			}
			else
				cout << "No Device: " << m_tmpDevice->GetDeviceInfo()->m_ID << endl;
			delete m_tmpDevice;
			m_tmpDevice = NULL;
		}
		else
		{
			//设备插入map中
			DeviceMap.insert(pair<string, Device*>(m_tmpDevice->GetDeviceInfo()->m_ID, m_tmpDevice));
			cout << "Add Device: " << m_tmpDevice->GetDeviceInfo()->m_ID << endl;
			m_tmpDevice = NULL;
		}

	}
	else if (ret == OSIP_SUCCESS)
		ret = SEND_ANSWER_SUCCESS;

	return ret;
}

std::string ExosipServer::GetFormatHeader()
{
	string contact;
	contact += "<sip:";
	contact += m_serverInfo->m_ID;
	contact += "@";
	contact += m_serverInfo->m_Ip;
	contact += ":";
	contact += m_serverInfo->m_Port;
	contact += ">";
	return contact;
}

std::string ExosipServer::GetFromHeader(std::string id)
{
	string contact;
	if (id == "defualt")
	{
		contact += "sip:";
		contact += m_serverInfo->m_ID;
		contact += "@";
		contact += m_serverInfo->m_domain;
		contact += ":";
		contact += m_serverInfo->m_Port;

	}
	else
	{
		contact += "sip:";
		contact += id;
		contact += "@";
		contact += m_serverInfo->m_domain;
		contact += ":";
		contact += m_serverInfo->m_Port;
	}
	cout << "From:" << contact << endl;
	return contact;
}

std::string ExosipServer::GetToHeader(std::string id)//用来寻找设备信息的
{
	string contact;
	//根据id寻找设备，就先这么将就着吧
	contact += "sip:";
	contact += id;
	contact += "@";
	contact += m_serverInfo->m_domain;
	contact += ":";
	contact += "5060";
	cout << "To:" << contact << endl;
	return contact;
}

std::string ExosipServer::GetRouteHeade(std::string id)
{
	string contact;
	//根据id寻找设备，就先这么将就着吧
	contact += "sip:";
	contact += id;
	contact += "@";
	contact += m_serverInfo->m_Ip;
	contact += ":";
	contact += "5060";
	cout << "Route:" << contact << endl;
	return contact;

}




Device::Device()
{
	if (m_Info == NULL)
		m_Info = new DeviceInfo();
}

DeviceInfo * Device::GetDeviceInfo()
{
	// TODO: 在此处插入 return 语句
	return m_Info;
}

int Device::SetDeviceBaseInfo(osip_message_t *msg)
{
	int ret = -1;
	osip_contact_t *contact = NULL;
	if (msg == NULL || m_Info == NULL)
		goto RESULT;

	osip_message_get_contact(msg, 0, &contact);
	m_Info->m_domain = string(msg->from->url->host);
	m_Info->m_ID = string(msg->from->url->username);
	m_Info->m_Port = string(contact->url->port);
	m_Info->m_iPort = -1;
	m_Info->m_Ip = string(contact->url->host);
	m_Info->isAuthNull = true;
	osip_contact_free(contact);
RESULT:

	return ret;
}

int Device::SetDeviceAuthInfo(osip_authorization_t *auth)
{
	//设置authorization信息
	if (auth == NULL || m_Info == NULL)
		goto RESULT;

	m_Info->m_authAlgorithm = auth->algorithm;
	m_Info->m_authnonce = auth->nonce;
	m_Info->m_authrealm = auth->realm;
	m_Info->m_authResponse = auth->response;
	m_Info->m_authUri = auth->uri;
	m_Info->m_authUserName = auth->username;
	m_Info->isAuthNull = false;

RESULT:
	return 0;
}

int Device::SetDeviceInfo(osip_message_t * msg)
{
	int ret = -1;
	osip_authorization_t *auth = NULL;
	if (msg == NULL)
		goto RESULT;

	ret = SetDeviceBaseInfo(msg);
	osip_message_get_authorization(msg, 0, &auth);
	if (auth != NULL)
	{
		SetDeviceAuthInfo(auth);
	}

RESULT:
	return ret;
}

bool Device::isAuthNull()
{
	return m_Info->isAuthNull;
}

Device::~Device()
{

}
