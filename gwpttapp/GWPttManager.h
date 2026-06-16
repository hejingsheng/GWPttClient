#pragma once

#include "GWPttAudioDevice.h"
#include <QObject>
#include <QThread>
#include <QMetaType>
#include "pt.h"

#define TIME_1_MICROSECOND  (1)
#define TIME_1_MILLSECOND   (1000 * TIME_1_MICROSECOND)
#define TIME_1_SECOND       (1000 * TIME_1_MILLSECOND)

#define PTT_CLIENT_EVENT_LOGIN       0
#define PTT_CLIENT_EVENT_QUERYGROUP  1
#define PTT_CLIENT_EVENT_SWITCHGROUP 2
#define PTT_CLIENT_EVENT_CREATEGROUP 3
#define PTT_CLIENT_EVENT_GROUPTOKEN  4
#define PTT_CLIENT_EVENT_GROUPDELETE 5
#define PTT_CLIENT_EVENT_SPEAK       6
#define PTT_CLIENT_EVENT_QUERYUSER   7
#define PTT_CLIENT_EVENT_OFFLINE     8
#define PTT_CLIENT_EVENT_ENTTMPCALL  9
#define PTT_CLIENT_EVENT_EXTTMPCALL  10   
#define PTT_CLIENT_EVENT_ERRTMPCALL  11
#define PTT_CLIENT_EVENT_RECVTXT     12
#define PTT_CLIENT_EVENT_NAMECHANGE  13
#define PTT_CLIENT_EVENT_RECVCMD     14
#define PTT_CLIENT_EVENT_REVCLOC     15
#define PTT_CLIENT_EVENT_RECVSOS     16
#define PTT_CLIENT_EVENT_LISTEN_GRP  17

#define PTT_QUERY_PAGE_SIZE   5

struct Speaker
{
	QString name;
	int uid;
	int status;

	Speaker() {
		name = "";
		uid = 0;
		status = 1;
	}

	Speaker(const Speaker &spk) {
		this->name = spk.name;
		this->uid = spk.uid;
		this->status = spk.status;
	}
};
Q_DECLARE_METATYPE(Speaker)

struct Group
{
	QString name;
	uint32_t gid;
	int type;
	uint32_t author;
	bool monitor;
};
Q_DECLARE_METATYPE(Group)

struct GroupOperate
{
	int result;
	QString name;
	int token;
	int ttl;
	uint32_t gid;

	GroupOperate() {
		result = 0;
		name = "";
		token = 0;
		ttl = 0;
		gid = 0;
	}

	GroupOperate(GroupOperate *data) {
		result = data->result;
		name = data->name;
		token = data->token;
		ttl = data->ttl;
		gid = data->gid;
	}

	static std::shared_ptr<GroupOperate> copy(const GroupOperate &grpToken)
	{
		std::shared_ptr<GroupOperate> ptr = std::make_shared<GroupOperate>();
		ptr->result = grpToken.result;
		ptr->name = grpToken.name;
		ptr->token = grpToken.token;
		ptr->ttl = grpToken.ttl;
		ptr->gid = grpToken.gid;
		return ptr;
	}

	static std::shared_ptr<GroupOperate> buildNewGroupToken(const Group &grp, int token, int ttl)
	{
		std::shared_ptr<GroupOperate> ptr = std::make_shared<GroupOperate>();
		ptr->result = 0;
		ptr->name = grp.name;
		ptr->token = token;
		ptr->ttl = ttl;
		ptr->gid = 0;
		return ptr;
	}

	static std::shared_ptr<GroupOperate> buildDeleteExit(int result, int gid = 0)
	{
		std::shared_ptr<GroupOperate> ptr = std::make_shared<GroupOperate>();
		ptr->result = result;
		ptr->gid = gid;
		return ptr;
	}
};
Q_DECLARE_METATYPE(GroupOperate)

struct GWPttQueryGroupEventData
{
	int totalpage;
	QList<Group> grouplist;
};

struct Member
{
	QString name;
	uint32_t uid;
	bool online;
};
Q_DECLARE_METATYPE(Member)

struct GWPttQueryUserEventData
{
	int totalpage;
	QList<Member> userlist;
};

class GWPttClientCallback
{
public:
	virtual void onPttClientEvent(int event, void *data) = 0;
	virtual ~GWPttClientCallback() = default;
};

class GWPttClient : public QObject
{
	Q_OBJECT

	enum PTT_CLIENT_STATUS_ENUM {
		PTT_CLIENT_STATUS_NONE = -1,
		PTT_CLIENT_STATUS_NETSCAN = 0,
		PTT_CLIENT_STATUS_NETCONN = 1,
		PTT_CLIENT_STATUS_LOGIN = 2,
		PTT_CLIENT_STATUS_REGMSG = 3,
		PTT_CLIENT_STATUS_JOINGRP = 4,
		PTT_CLIENT_STATUS_IDLE = 5,
		PTT_CLIENT_STATUS_OFFLINE = 6,
	};

public:
	static GWPttClient *getPtt();

public:
	GWPttClient();
	~GWPttClient();

public:
	void registerObserver(GWPttClientCallback *cb);
	void initPttMain(QString &account, QString &password, QString &address, int port);
	void pttStart();
	void queryGroup(int pageNum, int pageSize = PTT_QUERY_PAGE_SIZE);
	void createGroup();
	void enterGroup(uint32_t code, int type);
	void shareGroup(uint32_t gid);
	void deleteGroup(uint32_t gid);
	void exitGroup(uint32_t gid);
	void switchGroup(uint32_t gid);
	void pttSpeak(int flag);
	void queryUser(uint32_t gid, int pageNum, int pageSize = PTT_QUERY_PAGE_SIZE);
	void tempCall(uint32_t uid);
	void reportLocation(double lat, double lon, int type = 0);
	void sendSos(double lat, double lon, bool start);
	void listenGroup(uint32_t gid, int action);
	void logout();
	void pttEventReport(int event, const char *data, int len);
	void msgEventReport(int event, const char *msg, int len);

public:
	const QString& getSdkVersion() const {
		return sdkVersion;
	}
	const QString& getUsername() const {
		return name;
	}
	const QString& getCurrentGroup() const {
		return  currentGrpName;
	}
	const uint32_t getUid() const {
		return uid;
	}
	const uint32_t getTime() const;

signals:
	void sendSignalToPttMainThread(int param);

public slots:
	void pttMainThread(int param);

private:
	static void onGWPttEvent(int event, char *data, int data1);
	static void onGWMsgEvent(int status, char *msg, int length);

private:
	char pttMainLoop(int ms);

private:
	QThread *pttMainThread_;
	QTimer *pttCoreTimer_;
	QString sdkVersion;
	struct pt ptt_pt;
	GWPttClient::PTT_CLIENT_STATUS_ENUM status; //-1 none 0 start net scan 1 start net connect 2 start login 3 start join group  4 enter idle 5 offline
	int net_search_timeout;
	int net_ready_timeout;
	int login_timeout;
	int regmsg_timeout;
	int joingrp_timeout;
	int heart_timeout;
	int heart_period;
	QString account;
	QString password;
	QString name;
	uint32_t uid;
	int defaultGrpId;
	int currentGrpId;
	int currentGrpPri;
	int currentGrpType;
	int gps;
	int gps_period;
	int message;
	int call;
	bool msgStart;
	QString currentGrpName;
	QString serverIp;
	QString platformDns;
	int platformPort;
	int mode;
	bool exitai;
	QString locinfo;
	int reportLoc;
	QString weatherinfo;
	signed char local_zone;
	char local_zone_dec;
	char battery;
	QString net;

private:
	GWPttAudioModule *pttAudioDevice;

private:
	GWPttClientCallback *callback;
};
