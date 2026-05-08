#include "GWPttManager.h"
#include "GWPttEngine.h"
#include "GWLog.h"
#include <QTimer>
#include <QHostInfo>
#include "cJSON.h"
#include "PcAudioDevice.h"

#pragma comment(lib, "gwptt.lib")

#define PTT_CLIENT_DEFAULT_HEART_PERIOD   10

#define PTT_THREAD_TIMER_EVENT_TYPE         1
#define PTT_THREAD_NET_READY_EVENT_TYPE     2
#define PTT_THREAD_LOGIN_EVENT_TYPE         3
#define PTT_THREAD_REG_MSG_EVENT_TYPE       4
#define PTT_THREAD_IDLE_EVENT_TYPE          5
#define PTT_THREAD_OFFLINE_EVENT_TYPE       6

static int configAddress(QString host, int port)
{
	QString domain = host;
	QString ipaddr;
	QHostInfo info = QHostInfo::fromName(domain);
	if (info.error() == QHostInfo::NoError) 
	{
		foreach(const QHostAddress &address, info.addresses()) 
		{
			ipaddr = address.toString();
			break;
		}
	}
	else 
	{
		return -1;
	}
	pttConfigServer(0, (char*)ipaddr.toStdString().c_str(), port);
	return 0;
}

static GWPttClient pttMain;
GWPttClient *GWPttClient::getPtt()
{
	return &pttMain;
}

void GWPttClient::onGWPttEvent(int event, char *data, int data1)
{
	GWPttClient::getPtt()->pttEventReport(event, data, data1);
}

void GWPttClient::onGWMsgEvent(int status, char *msg, int length)
{
	GWPttClient::getPtt()->msgEventReport(status, msg, length);
}

GWPttClient::GWPttClient()
{
	char *version = nullptr;

	version = pttGetVersion();
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "ptt version %s", version);
	pttControlLog(1, 1);
	sdkVersion = version;
}

GWPttClient::~GWPttClient()
{
	if (pttCoreTimer_ != nullptr)
	{
		pttCoreTimer_->stop();
		delete pttCoreTimer_;
	}
	pttCoreTimer_ = nullptr;
	if (pttMainThread_ != nullptr)
	{
		pttMainThread_->quit();
		pttMainThread_->wait();
		delete pttMainThread_;
	}
	pttMainThread_ = nullptr;
	callback = nullptr;
	pttAudioDevice = nullptr;
}

void GWPttClient::pttStart()
{
	emit sendSignalToPttMainThread(PTT_THREAD_NET_READY_EVENT_TYPE);
}

void GWPttClient::queryGroup(int pageNum, int pageSize)
{
	pttQueryGroupByPage(pageSize, pageNum);
}

void GWPttClient::createGroup()
{
	pttGroupCreate("");
}

void GWPttClient::enterGroup(uint32_t code, int type)
{
	pttGroupEnterByToken(code, type);
}

void GWPttClient::shareGroup(uint32_t gid)
{
	pttGroupGeneralToken((int*)&gid, 1);
}

void GWPttClient::deleteGroup(uint32_t gid)
{
	pttGroupDelete(gid);
}

void GWPttClient::exitGroup(uint32_t gid)
{
	pttGroupExit(gid);
}

void GWPttClient::switchGroup(uint32_t gid)
{
	pttJoinGroup(gid, 16);
}

void GWPttClient::pttSpeak(int flag)
{
	if (flag == 0)
	{
		::pttSpeak(GW_PTT_SPEAK_START, 0);
	}
	else
	{
		::pttSpeak(GW_PTT_SPEAK_END, 0);
	}
}

void GWPttClient::queryUser(uint32_t gid, int pageNum, int pageSize)
{
	pttQueryMemberByPage(gid, 16, 0, pageSize, pageNum);
}

void GWPttClient::tempCall(uint32_t uid)
{
	pttTempGroup((int*)&uid, 1);
}

void GWPttClient::reportLocation(double lat, double lon, int type)
{
	pttReportLocationGps(lat, lon, type, uid);
}

void GWPttClient::logout()
{
	pttLogout();
}

void GWPttClient::pttEventReport(int event, const char *data, int len)
{
	cJSON *root = NULL;
	int ret;

	if (event == GW_PTT_EVENT_LOGIN)
	{
		cJSON *item;
		root = cJSON_Parse(data);
		ret = cJSON_GetObjectItem(root, "result")->valueint;
		if (ret == 0)
		{
			uid = cJSON_GetObjectItem(root, "uid")->valueint;
			defaultGrpId = cJSON_GetObjectItem(root, "defaultGid")->valueint;
			item = cJSON_GetObjectItem(root, "gps");
			if (item != NULL)
			{
				gps = item->valueint;
				if (gps)
				{
					item = cJSON_GetObjectItem(root, "gps_period");
					gps_period = item->valueint;
				}
			}
			else
			{
				gps = 0;
			}
			item = cJSON_GetObjectItem(root, "message");
			if (item != NULL)
			{
				message = item->valueint;
			}
			else
			{
				message = 0;
			}
			item = cJSON_GetObjectItem(root, "call");
			if (item != NULL)
			{
				call = item->valueint;
			}
			else
			{
				call = 0;
			}
			item = cJSON_GetObjectItem(root, "video");
			name = cJSON_GetObjectItem(root, "name")->valuestring;
			emit sendSignalToPttMainThread(PTT_THREAD_LOGIN_EVENT_TYPE);
		}
		else if (ret == 1)
		{
		}
		else
		{
			uid = 0;
			name = "";
			emit sendSignalToPttMainThread(PTT_THREAD_LOGIN_EVENT_TYPE);
		}
	}
	else if (event == GW_PTT_EVENT_JOIN_GROUP)
	{
		uint32_t creater;
		root = cJSON_Parse(data);
		ret = cJSON_GetObjectItem(root, "result")->valueint;
		if (ret == 0)
		{
			currentGrpPri = cJSON_GetObjectItem(root, "priority")->valueint;
			currentGrpId = cJSON_GetObjectItem(root, "gid")->valueint;
			creater = cJSON_GetObjectItem(root, "creater")->valueint;
			if (cJSON_GetObjectItem(root, "gname") != NULL)
			{
				currentGrpName = cJSON_GetObjectItem(root, "gname")->valuestring;
			}
			currentGrpType = 16;
			emit sendSignalToPttMainThread(PTT_THREAD_IDLE_EVENT_TYPE);
			if (callback != nullptr)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_SWITCHGROUP, (void*)currentGrpName.toStdString().c_str());
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "join group fail");
			emit sendSignalToPttMainThread(PTT_THREAD_IDLE_EVENT_TYPE);
			if (callback != nullptr)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_SWITCHGROUP, nullptr);
			}
		}
	}
	else if (event == GW_PTT_EVENT_SPEAK)
	{
		int speakerid = 0;
		int speakerpri = 0;
		char *speakernm = NULL;
		int status = 0;
		cJSON *item = NULL;

		root = cJSON_Parse(data);
		speakerid = cJSON_GetObjectItem(root, "uid")->valueint;
		if (speakerid > 0)
		{
			speakernm = cJSON_GetObjectItem(root, "name")->valuestring;
			item = cJSON_GetObjectItem(root, "priority");
			if (item != NULL && item->type == cJSON_Number)
			{
				speakerpri = item->valueint;
			}
			if (speakerpri >= currentGrpPri)
			{
				status = 0;
			}
			else
			{
				status = 1;
			}
			if (callback != nullptr)
			{
				std::shared_ptr<Speaker> speaker = std::make_shared<Speaker>();
				speaker->name = speakernm;
				speaker->uid = speakerid;
				speaker->status = status;
				callback->onPttClientEvent(PTT_CLIENT_EVENT_SPEAK, (void*)speaker.get());
			}
		}
		else
		{
			status = 1;
			if (callback != nullptr)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_SPEAK, nullptr);
			}
		}

	}
	else if (event == GW_PTT_EVENT_CURRENT_GROUP)
	{
		cJSON *item;
		uint32_t creater;
		root = cJSON_Parse(data);
		creater = cJSON_GetObjectItem(root, "creater")->valueint;
		currentGrpId = cJSON_GetObjectItem(root, "gid")->valueint;
		item = cJSON_GetObjectItem(root, "name");
		if (item != NULL)
		{
			currentGrpName = item->valuestring;
		}
		currentGrpType = 16;
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "join group %d, %s(%d)", currentGrpId, currentGrpName.toStdString().c_str(), creater);
		if (callback != nullptr)
		{
			callback->onPttClientEvent(PTT_CLIENT_EVENT_SWITCHGROUP, (void*)currentGrpName.toStdString().c_str());
		}
	}
	else if (event == GW_PTT_EVENT_QUERY_TMPGRP)
	{

	}
#if 0
	else if (event == GW_PTT_EVENT_AI_STATUS)
	{
		int status;
		root = cJSON_Parse(data);
		ret = cJSON_GetObjectItem(root, "result")->valueint;
		if (ret == 0)
		{
			status = cJSON_GetObjectItem(root, "status")->valueint;
			if (status == 0)
			{
				//updateUI(EXIT_AI, NULL);
				exitai = 0;
			}
			else
			{
				//updateUI(ENTER_AI, NULL);
			}
		}
		else
		{
			//updateUI(EXIT_AI, NULL);
		}
	}
#endif
	else if (event == GW_PTT_EVENT_TMP_GROUP_ACTIVE)
	{
		root = cJSON_Parse(data);
		ret = cJSON_GetObjectItem(root, "result")->valueint;
		if (callback != nullptr)
		{
			if (ret == 0)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_ENTTMPCALL, nullptr);
			}
			else if (ret == 1)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_EXTTMPCALL, nullptr);
			}
			else
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_ERRTMPCALL, nullptr);
			}
		}
	}
	else if (event == GW_PTT_EVENT_TMP_GROUP_PASSIVE)
	{
		cJSON *tmp;
		root = cJSON_Parse(data);
		tmp = cJSON_GetObjectItem(root, "status");
		if (tmp != NULL)
		{
			//exit temp call
			if (callback != nullptr)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_EXTTMPCALL, nullptr);
			}
		}
		else
		{
			char *name;
			tmp = cJSON_GetObjectItem(root, "name");
			if (tmp != NULL)
			{
				name = tmp->valuestring;
			}
			else
			{
				//name = "";
			}
			if (callback != nullptr)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_ENTTMPCALL, nullptr);
			}
		}
	}
	else if (event == GW_PTT_EVENT_RECV_TEXT)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "recv data is %s", data);
		if (callback != nullptr)
		{
			callback->onPttClientEvent(PTT_CLIENT_EVENT_RECVTXT, (void*)data);
		}
	}
	else if (event == GW_PTT_EVENT_NAME_CHANGE)
	{
		cJSON *tmp;
		char *name;
		root = cJSON_Parse(data);
		tmp = cJSON_GetObjectItem(root, "data");
		if (tmp != NULL)
		{
			name = tmp->valuestring;
			if (callback != nullptr)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_NAMECHANGE, (void*)name);
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "name change error");
		}
	}
	else if (event == GW_PTT_EVENT_LOGOUT || event == GW_PTT_EVENT_KICKOUT || event == GW_PTT_EVENT_ERROR || event == GW_PTT_EVENT_UNBIND)
	{
		uid = 0;
		defaultGrpId = 0;
		currentGrpId = 0;
		msgStart = 0;
		emit sendSignalToPttMainThread(PTT_THREAD_OFFLINE_EVENT_TYPE);
		if (callback != nullptr)
		{
			callback->onPttClientEvent(PTT_CLIENT_EVENT_OFFLINE, nullptr);
		}
	}
	if (root)
	{
		cJSON_Delete(root);
	}
}

void GWPttClient::msgEventReport(int status, const char *msg, int len)
{
	cJSON *root = NULL;
	cJSON *data = NULL;
	cJSON *item = NULL;
	int sta;
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "msg status %d", status);
	if (status == GW_MSG_STATUS_ERROR)
	{
		msgStart = 0;
		emit sendSignalToPttMainThread(PTT_THREAD_REG_MSG_EVENT_TYPE);
	}
	if (status == GW_MSG_STATUS_SUCC)
	{
		msgStart = 1;
		emit sendSignalToPttMainThread(PTT_THREAD_REG_MSG_EVENT_TYPE);
	}
	if (status == GW_MSG_STATUS_DATA)
	{
		int msgtype;
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		msgtype = cJSON_GetObjectItem(root, "msgtype")->valueint;
		if (msgtype == GW_PTT_MSG_TYPE_TEXT
			|| msgtype == GW_PTT_MSG_TYPE_VOICE
			|| msgtype == GW_PTT_MSG_TYPE_PHOTO
			|| msgtype == GW_PTT_MSG_TYPE_VIDEO
			|| msgtype == GW_PTT_MSG_TYPE_PTT_VOICE)
		{
			
		}
		else if (msgtype == GW_PTT_MSG_TYPE_USER_SIGNAL)
		{
			
		}
		else
		{
			if (callback != nullptr)
			{
				if (msgtype == 101)
				{
					// {"sendId":10006,"sendNm":"18038065657","recvtype":0,"msgtype":101,"signal":{"action":14,"users":[]},"timestamp":1778150469511}
					item = cJSON_GetObjectItem(root, "signal");
					int action = cJSON_GetObjectItem(item, "action")->valueint;
					callback->onPttClientEvent(PTT_CLIENT_EVENT_RECVCMD, (void*)action);
				}
				else
				{
					callback->onPttClientEvent(PTT_CLIENT_EVENT_RECVCMD, (void*)msgtype);
				}
			}
		}
	}
	else if (status == GW_MSG_STATUS_ADDRESS)
	{
		root = cJSON_Parse(msg);
		if (root != NULL)
		{
			item = cJSON_GetObjectItem(root, "status");
			sta = item->valueint;
			if (sta == 200)
			{
				data = cJSON_GetObjectItem(root, "data");
				item = cJSON_GetObjectItem(data, "ads");
				if (item->valuestring != NULL)
				{
					char *address = item->valuestring;
					if (callback != nullptr)
					{
						callback->onPttClientEvent(PTT_CLIENT_EVENT_REVCLOC, (void*)address);
					}
				}
			}
			else
			{
				// error
			}
		}
	}
	else if (status == GW_MSG_STATUS_WEATHER)
	{

	}
	else if (status == GW_MSG_STATUS_QUERY_GROUP)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			int size;
			int index;
			int pages;
			int totals;
			cJSON *groups;
			cJSON *tmp;
			data = cJSON_GetObjectItem(root, "data");
			if (data == NULL)
			{
				goto error;
			}
			tmp = cJSON_GetObjectItem(root, "pages");
			if (tmp == NULL || tmp->valueint == 0)
			{
				// normal query
				pages = 0;
			}
			else
			{
				pages = tmp->valueint;
				totals = cJSON_GetObjectItem(root, "total")->valueint;
			}
			groups = cJSON_GetObjectItem(data, "groups");
			if (groups != NULL)
			{
				size = cJSON_GetArraySize(groups);
				if (size > 0)
				{
					std::shared_ptr<GWPttQueryGroupEventData> queryGroupData = std::make_shared<GWPttQueryGroupEventData>();
					for (index = 0; index < size; index++)
					{
						struct Group group;
						cJSON *item = cJSON_GetArrayItem(groups, index);
						group.name = cJSON_GetObjectItem(item, "name")->valuestring;
						group.gid = cJSON_GetObjectItem(item, "gid")->valueint;
						group.author = cJSON_GetObjectItem(item, "authorUid")->valueint;
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "group id %d, name %s, creater %d", group.gid, group.name, group.author);
						queryGroupData->grouplist.append(group);
					}
					if (pages == 0)
					{
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "group count %d", size);
						queryGroupData->totalpage = -1;
					}
					else
					{
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "page group count %d,%d", totals, size);
						queryGroupData->totalpage = totals;
					}
					if (callback != nullptr)
					{
						callback->onPttClientEvent(PTT_CLIENT_EVENT_QUERYGROUP, (void*)(queryGroupData.get()));
					}
				}
				else
				{
					GWLOG_PRINT(GW_LOG_LEVEL_INFO, "no group");
					if (callback != nullptr)
					{
						callback->onPttClientEvent(PTT_CLIENT_EVENT_QUERYGROUP, nullptr);
					}
				}
			}
			else
			{
				GWLOG_PRINT(GW_LOG_LEVEL_INFO, "no group");
				if (callback != nullptr)
				{
					callback->onPttClientEvent(PTT_CLIENT_EVENT_QUERYGROUP, nullptr);
				}
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "query group fail %d", sta);
			if (callback != nullptr)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_QUERYGROUP, nullptr);
			}
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_CREATE)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			struct Group group;
			int token;
			int ttl;
			data = cJSON_GetObjectItem(root, "data");
			group.name = cJSON_GetObjectItem(data, "name")->valuestring;
			group.gid = cJSON_GetObjectItem(data, "gid")->valueint;
			token = cJSON_GetObjectItem(data, "code")->valueint;
			ttl = cJSON_GetObjectItem(data, "ttl")->valueint;
			group.author = uid;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "create group %s(%d) success", group.name, group.gid);
			if (callback != nullptr)
			{
				std::shared_ptr<GroupOperate> ptr = GroupOperate::buildNewGroupToken(group, token, ttl);
				callback->onPttClientEvent(PTT_CLIENT_EVENT_CREATEGROUP, (void*)(ptr.get()));
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "create group fail");
			if (callback != nullptr)
			{
				std::shared_ptr<GroupOperate> ptr = std::make_shared<GroupOperate>();
				ptr->result = sta;
				callback->onPttClientEvent(PTT_CLIENT_EVENT_CREATEGROUP, (void*)(ptr.get()));
			}
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_ADDUSER)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "add user success");
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "add user fail");
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_DELUSER)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "delete user success");
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "delete user fail");
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_ENTER)
	{

	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_EXIT)
	{
		cJSON *data;
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			int gid;
			data = cJSON_GetObjectItem(root, "data");
			gid = cJSON_GetObjectItem(data, "gid")->valueint;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "exit group %d success", gid);
			if (callback != nullptr)
			{
				std::shared_ptr<GroupOperate> ptr = GroupOperate::buildDeleteExit(0, gid);
				callback->onPttClientEvent(PTT_CLIENT_EVENT_GROUPDELETE, (void*)(ptr.get()));
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "exit group fail");
			if (callback != nullptr)
			{
				std::shared_ptr<GroupOperate> ptr = GroupOperate::buildDeleteExit(sta);
				callback->onPttClientEvent(PTT_CLIENT_EVENT_GROUPDELETE, (void*)(ptr.get()));
			}
		}
		
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_RENAME)
	{
		// not process
	}
	else if (status == GW_MSG_STATUS_GROUP_OPERATE_DELETE)
	{
		cJSON *data;
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			int gid;
			data = cJSON_GetObjectItem(root, "data");
			gid = cJSON_GetObjectItem(data, "gid")->valueint;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "delete group %d success", gid);
			if (callback != nullptr)
			{
				std::shared_ptr<GroupOperate> ptr = GroupOperate::buildDeleteExit(0, gid);
				callback->onPttClientEvent(PTT_CLIENT_EVENT_GROUPDELETE, (void*)(ptr.get()));
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "delete group fail");
			if (callback != nullptr)
			{
				std::shared_ptr<GroupOperate> ptr = GroupOperate::buildDeleteExit(sta);
				callback->onPttClientEvent(PTT_CLIENT_EVENT_GROUPDELETE, (void*)(ptr.get()));
			}
		}

	}
	else if (status == GW_MSG_STATUS_GENERAL_GROUP_TOKEN)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			cJSON *code;
			GroupOperate token;
			data = cJSON_GetObjectItem(root, "data");
			code = cJSON_GetObjectItem(data, "grpCode");
			token.name = "";
			token.token = cJSON_GetObjectItem(code, "code")->valueint;
			token.ttl = cJSON_GetObjectItem(code, "ttl")->valueint;
			if (callback != nullptr)
			{
				std::shared_ptr<GroupOperate> ptr = GroupOperate::copy(token);
				callback->onPttClientEvent(PTT_CLIENT_EVENT_GROUPTOKEN, (void*)(ptr.get()));
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "general token fail")
		}
	}
	else if (status == GW_MSG_STATUS_GROUP_TOKEN_ENTER)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "enter group success");
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "enter group fail");
		}
	}
	else if (status == GW_MSG_STATUS_UNBIND)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "unbind success");
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "unbind fail");
		}
	}
	else if (status == GW_MSG_STATUS_QUERY_MEMBER)
	{
		root = cJSON_Parse(msg);
		if (root == NULL)
		{
			goto error;
		}
		sta = cJSON_GetObjectItem(root, "status")->valueint;
		if (sta == 200)
		{
			cJSON *members;
			cJSON *tmp;
			int size;
			int idx;
			int pages;
			int totals;
			GWLOG_PRINT(GW_LOG_LEVEL_INFO, "query member success");
			data = cJSON_GetObjectItem(root, "data");
			if (data == NULL)
			{
				goto error;
			}
			tmp = cJSON_GetObjectItem(root, "pages");
			if (tmp == NULL || tmp->valueint == 0)
			{
				// normal query
				pages = 0;
			}
			else
			{
				pages = tmp->valueint;
				totals = cJSON_GetObjectItem(root, "total")->valueint;
			}
			members = data;
			if (members != NULL)
			{
				size = cJSON_GetArraySize(members);
				if (size > 0)
				{
					std::shared_ptr<GWPttQueryUserEventData> queryMemberData = std::make_shared<GWPttQueryUserEventData>();
					for (idx = 0; idx < size; idx++)
					{
						struct Member member;
						cJSON *item = cJSON_GetArrayItem(members, idx);
						int uid = cJSON_GetObjectItem(item, "uid")->valueint;
						char *uname = cJSON_GetObjectItem(item, "name")->valuestring;
						int online = cJSON_GetObjectItem(item, "online")->valueint;
						member.name = uname;
						member.uid = uid;
						member.online = online;
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "user id %d, name %s, online %d", uid, uname, online);
						queryMemberData->userlist.append(member);
					}
					if (pages == 0)
					{
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "user count %d", size);
						queryMemberData->totalpage = -1;
					}
					else
					{
						GWLOG_PRINT(GW_LOG_LEVEL_INFO, "page user count %d,%d", totals, size);
						queryMemberData->totalpage = totals;
					}
					if (callback != nullptr)
					{
						callback->onPttClientEvent(PTT_CLIENT_EVENT_QUERYUSER, (void*)(queryMemberData.get()));
					}
				}
				else
				{
					GWLOG_PRINT(GW_LOG_LEVEL_INFO, "no user");
					if (callback != nullptr)
					{
						callback->onPttClientEvent(PTT_CLIENT_EVENT_QUERYUSER, nullptr);
					}
				}
			}
			else
			{
				GWLOG_PRINT(GW_LOG_LEVEL_INFO, "no user");
				if (callback != nullptr)
				{
					callback->onPttClientEvent(PTT_CLIENT_EVENT_QUERYUSER, nullptr);
				}
			}
		}
		else
		{
			GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "query member fail %d", sta);
			if (callback != nullptr)
			{
				callback->onPttClientEvent(PTT_CLIENT_EVENT_QUERYUSER, nullptr);
			}
		}
	}
error:
	if (root != NULL)
	{
		cJSON_Delete(root);
	}
}

const uint32_t GWPttClient::getTime() const
{
	return pttGetTime();
}

char GWPttClient::pttMainLoop(int ms)
{
	int ret;
	struct pt *pt = &ptt_pt;

	if (ms != 0)
	{
		if (net_search_timeout > ms)
		{
			net_search_timeout -= ms;
		}
		else
		{
			net_search_timeout = 0;
		}
		if (net_ready_timeout > ms)
		{
			net_ready_timeout -= ms;
		}
		else
		{
			net_ready_timeout = 0;
		}
		if (login_timeout > ms)
		{
			login_timeout -= ms;
		}
		else
		{
			login_timeout = 0;
		}
		if (regmsg_timeout > ms)
		{
			regmsg_timeout -= ms;
		}
		else
		{
			regmsg_timeout = 0;
		}
		if (joingrp_timeout > ms)
		{
			joingrp_timeout -= ms;
		}
		else
		{
			joingrp_timeout = 0;
		}
		if (heart_timeout > ms)
		{
			heart_timeout -= ms;
		}
		else
		{
			heart_timeout = 0;
		}
	}

	if (status == PTT_CLIENT_STATUS_OFFLINE)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "offline...");
		PT_INIT(pt);
	}

	PT_BEGIN(pt);

	PT_WAIT_UNTIL(pt, (status == PTT_CLIENT_STATUS_LOGIN));

LOGIN:
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "net connect success start logining...");
	ret = configAddress(platformDns, platformPort);
	if (ret < 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "domain or ip is error");
		PT_EXIT(pt);
	}
	pttLogin(account.toStdString().c_str(), password.toStdString().c_str(), "12345", "54321");

	login_timeout = 30 * TIME_1_SECOND;
	PT_WAIT_UNTIL(pt, (status == PTT_CLIENT_STATUS_REGMSG) || (login_timeout == 0));
	if (uid == 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "login fail...");
		status = PTT_CLIENT_STATUS_LOGIN;
		if (callback != nullptr)
		{
			callback->onPttClientEvent(PTT_CLIENT_EVENT_LOGIN, (void*)1);
		}
		goto LOGIN;
	}
	if (login_timeout == 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "login timeout...");
		goto LOGIN;
	}

	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "login success start register msg...");
REGMSG:
	//pttQueryGroup();
	if (message == 1)
	{
		pttRegOfflineMsg(NULL, NULL, 0, 0);
	}
	else
	{
		status = PTT_CLIENT_STATUS_JOINGRP;
	}
	regmsg_timeout = 10 * TIME_1_SECOND;
	PT_WAIT_UNTIL(pt, (status == PTT_CLIENT_STATUS_JOINGRP) || (regmsg_timeout == 0));
	if ((msgStart == 0 && message == 1) || (regmsg_timeout == 0))
	{
		GWLOG_PRINT(GW_LOG_LEVEL_WARN, "register msg error");
		status = PTT_CLIENT_STATUS_REGMSG;
		goto REGMSG;
	}
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "register msg success start join group...");
JOINGRP:
	if (defaultGrpId != 0)
	{
		pttJoinGroup(defaultGrpId, 16);
	}
	else
	{
		GWLOG_PRINT(GW_LOG_LEVEL_WARN, "not default group enter idle");
		status = PTT_CLIENT_STATUS_IDLE;
	}
	joingrp_timeout = 30 * TIME_1_SECOND;
	PT_WAIT_UNTIL(pt, (status == PTT_CLIENT_STATUS_IDLE) || (joingrp_timeout == 0));
	if (joingrp_timeout == 0)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_ERROR, "join group fail...");
		goto JOINGRP;
	}
	GWLOG_PRINT(GW_LOG_LEVEL_INFO, "join group success...");
	if (callback != nullptr)
	{
		callback->onPttClientEvent(PTT_CLIENT_EVENT_LOGIN, (void*)0);
	}
	while (1)
	{
		heart_timeout = heart_period * TIME_1_SECOND;
		PT_WAIT_UNTIL(pt, (heart_timeout == 0));
		pttHeart(battery, net.toStdString().c_str());
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "send heart...");
	}
	PT_END(pt);
}

void GWPttClient::pttMainThread(int param)
{
	switch (param)
	{
	case PTT_THREAD_TIMER_EVENT_TYPE:
		pttMainLoop(TIME_1_SECOND);
		break;
	case PTT_THREAD_NET_READY_EVENT_TYPE:
		status = PTT_CLIENT_STATUS_LOGIN;
		pttMainLoop(0);
		break;
	case PTT_THREAD_LOGIN_EVENT_TYPE:
		status = PTT_CLIENT_STATUS_REGMSG;
		pttMainLoop(0);
		break;
	case PTT_THREAD_REG_MSG_EVENT_TYPE:
		status = PTT_CLIENT_STATUS_JOINGRP;
		pttMainLoop(0);
		break;
	case PTT_THREAD_IDLE_EVENT_TYPE:
		status = PTT_CLIENT_STATUS_IDLE;
		pttMainLoop(0);
		break;
	case PTT_THREAD_OFFLINE_EVENT_TYPE:
		status = PTT_CLIENT_STATUS_OFFLINE;
		pttMainLoop(0);
		break;
	default:
		break;
	}
}

void GWPttClient::registerObserver(GWPttClientCallback *cb)
{
	callback = cb;
}

void GWPttClient::initPttMain(QString &account, QString &password, QString &address, int port)
{
	pttAudioDevice = pcInitAudioDevice(GW_PTT_AUDIO_SAMPLERATE, GW_PTT_AUDIO_BITS, GW_PTT_AUDIO_CHANNELS);
	pttInit(onGWPttEvent, onGWMsgEvent, pttAudioDevice, 0, GW_PTT_ENCODE_LEVEL_HIGH, 0);

	pttCoreTimer_ = new QTimer(this);
	pttCoreTimer_->setInterval(TIME_1_SECOND / 1000);        // 1秒
	pttCoreTimer_->setSingleShot(false);  // 默认就是 false，可省略
	connect(pttCoreTimer_, &QTimer::timeout, this, [this]() {
		emit sendSignalToPttMainThread(PTT_THREAD_TIMER_EVENT_TYPE);
	});
	pttCoreTimer_->start();

	this->account = account;
	this->password = password;
	this->platformDns = address;
	this->platformPort = port;

	this->status = PTT_CLIENT_STATUS_NONE;
	this->heart_period = PTT_CLIENT_DEFAULT_HEART_PERIOD;
	this->local_zone = 8; // beijing time
	this->local_zone_dec = 0;
	this->battery = 100;
	this->net = "net";

	pttMainThread_ = new QThread;
	this->moveToThread(pttMainThread_);

	pttMainThread_->start();
	QObject::connect(this, &GWPttClient::sendSignalToPttMainThread, this, &GWPttClient::pttMainThread, Qt::QueuedConnection);

}
