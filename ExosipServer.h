#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <map>
#include <algorithm>
#include <eXosip2/eXosip.h>
#include <winsock.h>

//����һ�������
#define NONCE "9bd055"
//UASĬ�ϼ����㷨
#define ALGORITHTHM "MD5"

struct DeviceInfo
{
	std::string m_Ip; //�豸��ip
	std::string m_Port; //�豸�Ķ˿ں�
	int m_iPort;
	std::string m_ID;	//�豸��ID
	std::string m_domain; //�豸������
	bool isAuthNull;
	//authorization��Ϣ
	std::string m_authUserName;
	std::string m_authAlgorithm;
	std::string m_authrealm;
	std::string m_authnonce;
	std::string m_authResponse;
	std::string m_authUri;

};

class Device //�洢�ڷ������е��豸
{
public:
	Device();
	DeviceInfo *GetDeviceInfo();//��ȡ�豸����Ϣ
	int SetDeviceBaseInfo(osip_message_t *msg);//�����豸����Ϣ
	int SetDeviceAuthInfo(osip_authorization_t *auth);
	int SetDeviceInfo(osip_message_t *msg);
	bool isAuthNull();

	~Device();
private:
	DeviceInfo *m_Info = NULL;//�豸����Ϣ
	
};

class ExosipServer
{
public:
	enum RETURNVALUE
	{
		INIT_SUCCESS = 101, //��ʼ��sip�������ɹ�
		INIT_FAIL,			//��ʼ��sip������ʧ��
		SEND_ANSWER_SUCCESS,
		REGISTE_SUCCESS,
		CONTROL_SUCCESS
	};
	enum CONTROLCOMM {
		LEFT,
		RIGHT,
		UP,
		DOWN,
		ZOOMLARGE,
		ZOOMLITTLE,
		STOP_CONTROL
	};

	ExosipServer(std::string ip = "127.0.0.1", std::string port = "8880", int iport = 8880, std::string domain = "3402000000", std::string id = "34020000002000000001");
	int ServerInit();
	int ServerListen(int m = 0, int s = 1000);
	int ServerTurnControl(std::string id, int controlComm);

	~ExosipServer();
private:
	std::map<std::string, Device*> DeviceMap;//�洢�豸��map��keyֵΪ�豸�ı�ţ�valueΪDevice
	Device *m_tmpDevice = NULL;//����ע����豸��ʱ�ԵĴ洢������

	DeviceInfo *m_serverInfo = NULL; //�洢����������Ϣ
	eXosip_t *m_server = NULL;
	eXosip_event_t *m_event = NULL;//���������¼�

	void PrintMsg(osip_message_t *msg);
	int SendRegisteAnswer();
	std::string GetFormatHeader();
	std::string GetFromHeader(std::string id = "defualt");
	std::string GetToHeader(std::string id = "defualt");
	std::string GetRouteHeade(std::string id = "defualt");

};

