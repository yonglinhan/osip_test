#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <map>
#include <algorithm>
#include <eXosip2/eXosip.h>
#include <winsock.h>

//这是一个随机数
#define NONCE "9bd055"
//UAS默认加密算法
#define ALGORITHTHM "MD5"

struct DeviceInfo
{
	std::string m_Ip; //设备的ip
	std::string m_Port; //设备的端口号
	int m_iPort;
	std::string m_ID;	//设备的ID
	std::string m_domain; //设备的域名
	bool isAuthNull;
	//authorization信息
	std::string m_authUserName;
	std::string m_authAlgorithm;
	std::string m_authrealm;
	std::string m_authnonce;
	std::string m_authResponse;
	std::string m_authUri;

};

class Device //存储在服务器中的设备
{
public:
	Device();
	DeviceInfo *GetDeviceInfo();//获取设备的信息
	int SetDeviceBaseInfo(osip_message_t *msg);//设置设备的信息
	int SetDeviceAuthInfo(osip_authorization_t *auth);
	int SetDeviceInfo(osip_message_t *msg);
	bool isAuthNull();

	~Device();
private:
	DeviceInfo *m_Info = NULL;//设备的信息
	
};

class ExosipServer
{
public:
	enum RETURNVALUE
	{
		INIT_SUCCESS = 101, //初始化sip服务器成功
		INIT_FAIL,			//初始化sip服务器失败
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
	std::map<std::string, Device*> DeviceMap;//存储设备的map，key值为设备的编号，value为Device
	Device *m_tmpDevice = NULL;//请求注册的设备暂时性的存储在这里

	DeviceInfo *m_serverInfo = NULL; //存储服务器的信息
	eXosip_t *m_server = NULL;
	eXosip_event_t *m_event = NULL;//监听到的事件

	void PrintMsg(osip_message_t *msg);
	int SendRegisteAnswer();
	std::string GetFormatHeader();
	std::string GetFromHeader(std::string id = "defualt");
	std::string GetToHeader(std::string id = "defualt");
	std::string GetRouteHeade(std::string id = "defualt");

};

