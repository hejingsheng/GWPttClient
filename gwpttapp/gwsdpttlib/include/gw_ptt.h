//
// Created by Administrator on 2024/5/8.
//

#ifndef GW_GW_PTT_H
#define GW_GW_PTT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_LENGTH (64)

#define GW_PTT_AUDIO_SAMPLERATE            (16000)
#define GW_PTT_AUDIO_BITS                  (16)
#define GW_PTT_AUDIO_CHANNELS              (1)
#define GW_PTT_AUDIO_DURATION_MS_PER_FRAME (20)
#define GW_PTT_AUDIO_BYTES_PER_FRAME       ((GW_PTT_AUDIO_SAMPLERATE/(1000/GW_PTT_AUDIO_DURATION_MS_PER_FRAME)*GW_PTT_AUDIO_BITS/8)*GW_PTT_AUDIO_CHANNELS)
#define GW_PTT_ENCODE_LEVEL_LOW            0
#define GW_PTT_ENCODE_LEVEL_MID            1
#define GW_PTT_ENCODE_LEVEL_HIGH           2

#define GW_PTT_LOG_LEVEL_DEBUG             1
#define GW_PTT_LOG_LEVEL_INFO              2
#define GW_PTT_LOG_LEVEL_WARN              3
#define GW_PTT_LOG_LEVEL_ERROR             4
#define GW_PTT_LOG_LEVEL_NEVER             5

enum {
    GW_PTT_SPEAK_START = 0,
    GW_PTT_SPEAK_END = 1,
};

enum {
    GW_PTT_DUPLEX_ACTION_CREATE = 20,  // start call
    GW_PTT_DUPLEX_ACTION_ACCEPT = 21,  // accept call
    GW_PTT_DUPLEX_ACTION_HANGUP = 25,  // hangup call
};

enum {
    GW_PTT_EVENT_ERROR = -1,
    GW_PTT_EVENT_LOGIN = 0,
    GW_PTT_EVENT_QUERY_GROUP = 1,
    GW_PTT_EVENT_JOIN_GROUP = 2,
    GW_PTT_EVENT_QUERY_MEMBER = 3,
    GW_PTT_EVENT_REQUEST_MIC = 4,
    GW_PTT_EVENT_TMP_GROUP_ACTIVE = 5,
    GW_PTT_EVENT_TMP_GROUP_PASSIVE = 6,
    GW_PTT_EVENT_DUPLEX = 7,
    GW_PTT_EVENT_LOGOUT = 8,
    GW_PTT_EVENT_KICKOUT = 9,
    GW_PTT_EVENT_CURRENT_GROUP = 10,
    GW_PTT_EVENT_SPEAK = 11,
    GW_PTT_EVENT_PLAY_DATA = 12,
    GW_PTT_EVENT_LOSTMIC = 13,
    GW_PTT_EVENT_TEXT_RECV = 14,
    GW_PTT_EVENT_GROUP_OPERATE = 15,
    GW_PTT_EVENT_QUERY_DISPATCH = 16,
    GW_PTT_EVENT_FRIEND_OPERATE = 17,
    GW_PTT_EVENT_QUERY_CHAT_GRP = 18,
    GW_PTT_EVENT_CHAT_GRP_DETAIL = 19,
    GW_PTT_EVENT_CHAT_SESS_ACTION = 20,
    GW_PTT_EVENT_CHAT_MSG_DETAIL = 21,
    GW_PTT_EVENT_CHAT_SEND_MSG = 22,
    GW_PTT_EVENT_AI_STATUS = 23,
    GW_PTT_EVENT_UNBIND = 24,
    GW_PTT_EVENT_QUERY_TMPGRP = 25,
    GW_PTT_EVENT_RECV_TEXT = 26,
    GW_PTT_EVENT_NAME_CHANGE = 27,
};

enum {
    GW_PTT_DUPLEX_STATUS_START = 10,
    GW_PTT_DUPLEX_STATUS_INVITE = GW_PTT_DUPLEX_STATUS_START,
    GW_PTT_DUPLEX_STATUS_ACCEPTED = 11,
    GW_PTT_DUPLEX_STATUS_END = 15,
    GW_PTT_DUPLEX_STATUS_BUSY = 16,
    GW_PTT_DUPLEX_STATUS_OTHER_INVITE = 17,
};

enum {
    GW_PTT_MSG_RECV_TYPE_USER = 0,
    GW_PTT_MSG_RECV_TYPE_GROUP = 16,
};

enum {
	GW_PTT_INVITE_TO_GROUP = 0,
	GW_PTT_KICK_FROM_GROUP = 1,
	GW_PTT_RENAME_GRPUP = 2,
	GW_PTT_USER_LEAVE_GROUP = 3,
    GW_PTT_USER_ENTER_GROUP = 4,
    GW_PTT_GROUP_ADMIN_CHANGE = 5,
	GW_PTT_ADD_FRIEND_REQ = 6,
	GW_PTT_DEL_FRIEND_REP = 7,
	GW_PTT_ACCEPT_ADD_FRIEND = 8,
	GW_PTT_REJECT_ADD_FRIEND = 9,
    GW_PTT_DEVICE_CREATE_GROUP = 10,
    GW_PTT_DEVICE_DELETE_GROUP = 11,
};

enum {
	GW_PTT_ADD_FRIEND = 0,
	GW_PTT_QUERY_FRIEND = 1,
	GW_PTT_DEL_FRIEND = 2,
	GW_PTT_ACCEPT_ADDFRIEND = 3,
	GW_PTT_REJECT_ADDFRIEND = 4,
    GW_PTT_QUERY_FRIEND_RECORD_STATUS = 5,
};

enum {
    GW_PTT_FRIEND_RECORD_STATUS_WAIT_REMOTE_OPERATE = 1,
    GW_PTT_FRIEND_RECORD_STATUS_HAVE_ADD = 2,
    GW_PTT_FRIEND_RECORD_STATUS_WAIT_LOCAL_OPERATE = 3,
    GW_PTT_FRIEND_RECORD_STATUS_HAVE_REJECT = 4,
};

enum {
	GW_PTT_CREATE_GROUP = 0,
	GW_PTT_ADD_USER_TO_GROUP= 1,
	GW_PTT_DEL_USER_FROM_GROUP = 2,
	GW_PTT_DEL_GROUP = 3,
	GW_PTT_RENAME_GROUP = 4,
	GW_PTT_EXIT_GROUP = 5,
	GW_PTT_ENTER_GROUP = 6,
};

enum {
    GW_PTT_MSG_TYPE_TEXT = 0,
    GW_PTT_MSG_TYPE_PHOTO = 1,
    GW_PTT_MSG_TYPE_VOICE = 2,
    GW_PTT_MSG_TYPE_VIDEO = 3,
    GW_PTT_MSG_TYPE_PTT_VOICE = 4,

    GW_PTT_MSG_TYPE_USER_SIGNAL = 100,
};

enum {
    GW_PTT_PATROL_RESULT_PUNCH = 0,
    GW_PTT_PATROL_RESULT_NORMAL = 1,
    GW_PTT_PATROL_RESULT_EXCE = 2,
};

enum {
    GW_PTT_FILE_TYPE_IMAGE = 0,
    GW_PTT_FILE_TYPE_VOICE = 1,
    GW_PTT_FILE_TYPE_VIDEO = 2,
};

enum {
    GW_MSG_STATUS_ERROR = -1,
    GW_MSG_STATUS_SUCC = 0,
    GW_MSG_STATUS_DATA = 1,
	GW_MSG_STATUS_WEATHER = 2,
	GW_MSG_STATUS_ADDRESS = 3,
    GW_MSG_STATUS_PATROL = 4,
    GW_MSG_STATUS_SELF_DATA = 5,
    GW_MSG_STATUS_FILE_UPLOAD = 6,
    GW_MSG_STATUS_GROUP_OPERATE_CREATE = 7,
    GW_MSG_STATUS_GROUP_OPERATE_ADDUSER = 8,
    GW_MSG_STATUS_GROUP_OPERATE_DELUSER = 9,
    GW_MSG_STATUS_GROUP_OPERATE_ENTER = 10,
    GW_MSG_STATUS_GROUP_OPERATE_EXIT = 11,
    GW_MSG_STATUS_GROUP_OPERATE_RENAME = 12,
    GW_MSG_STATUS_GROUP_OPERATE_DELETE = 13,
    GW_MSG_STATUS_QUERY_MEMBER = 14,
    GW_MSG_STATUS_QUERY_GROUP = 15,
    GW_MSG_STATUS_QUERY_LASTMSG = 16,
    GW_MSG_STATUS_GENERAL_GROUP_TOKEN = 17,
    GW_MSG_STATUS_GROUP_TOKEN_ENTER = 18,
    GW_MSG_STATUS_UNBIND = 19,
    GW_MSG_STATUS_MANAGER_INFO = 20,
};

enum {
	GW_PTT_LOC_TYPE_GPS = 0,
	GW_PTT_LOC_TYPE_CELL = 1,
	GW_PTT_LOC_TYPE_WIFI = 2,
};

enum {
	GW_PTT_NETMODE_2G = 2,
	GW_PTT_NETMODE_3G = 3,
	GW_PTT_NETMODE_4G = 4,
};

typedef struct
{
    int uid;
    int priority;
    int change;
    char name[MAX_NAME_LENGTH];
}GWPttSpeaker;

typedef struct
{
	double lat;
	double lon;
}GWLocGps;

typedef struct
{
	unsigned int cellid;
	unsigned int lac_or_tac;
	int mode;
	char mcc[4];
	char mnc[4];
}GWLocCell;

typedef struct
{
	char *bssid;
	int signal;
	char *ssid;
}GWLocWifi;

typedef struct
{
	union {
		GWLocGps gps;
		GWLocCell cell;
		GWLocWifi wifi;
	}location;
	int type;
}GWPttLocation;

typedef void (*GWPttEvent)(int event, char *data, int data1);
typedef void (*GWMsgEvent)(int status, char *msg, int length);
typedef void (*GWVideoEvent)(char *data, int length);
typedef void* (*memory_allocate)(unsigned int size);


int gwPttSetCustomMalloc(memory_allocate alloc);

int gwPttInit(GWPttEvent event, GWMsgEvent event1, GWVideoEvent event2, char externalCodec, int encodeLevel, int frameSize, char lowMemory);

char *gwPttGetVersion(void);

int gwPttConfigServer(int type, char *host, int port);

int gwPttSaveVoice(int open, char *path);

int gwPttControlLog(int level, const char *logfile);

int gwPttNetCheck(int type, char *host, int port);

int gwPttEnableMsg(int enable);

int gwPttLogin(const char *account, const char *pass, const char *imei, const char *iccid);

int gwPttQueryGroup(void);

int gwPttQueryGroup1(char **response);

int gwPttQueryGroupByPage(int pageSize, int pageNum, char **response);

int gwPttJoinGroup(int gid, int type);

int gwPttGetLastGid(void);

int gwPttQueryMember(int gid, int type);

int gwPttQueryMember1(int gid, char **response);

int gwPttQueryMemberByPage(int gid, int type, int pageSize, int pageNum, char **response);

int gwPttQueryLastMsg(int gid, char **response);

int gwPttQueryDispatcher(int gid, int type);

int gwPttSpeakStartToDmr(char dmrtype, int dmrid, long long ms);

int gwPttSpeak(int action, long long ms);

int gwPttHeart(int battery, const char *net);

int gwPttQueryTmpGrp(int type);

int gwPttLogout(void);

int gwPttDuplexCall(int uid, int action);

int gwPttGroupOperate(char *data, int type, char **response);

int gwPttGeneralToken(int *gids, int num, char **response);

int gwPttEnterGroupByToken(int token, int type, char **response);

int gwPttGetManagerInfo(char **response);

int gwPttUnbind(char **response);

int gwPttCreateGroup(const char *gname);

int gwPttGroupAddUser(int gid, int num, int *uids);

int gwPttGroupDelUser(int gid, int num, int *uids);

int gwPttGroupDelete(int gid);

int gwPttGroupRename(int gid, const char *gname);

int gwPttGroupExit(int gid);

int gwPttEnterGroup(int gid);

int gwPttOperateFriend(int operate, int uid);

int gwPttQueryChatGroups(int uid);

int gwPttQueryGroupDetail(int gid, int type);

int gwPttActiveSession(int sid, int type);

int gwPttDeactiveSession(int sid, int type);

int gwPttDeleteSession(int sid, int type);

int gwPttQueryMsgDetail(int gid, int type, char *msgid);

int gwPttTempGroup(int *uids, int num);

int gwPttRegMsg(int groups[], int type[], int num, char tls, char offline);

int gwPttSendVideoMsg(char *sid, const char *data);

int gwPttSendMsg(int sid, const char *snm, int type, int id, const char *rnm, int msgType, const char *content, const char *thumburl, int time);

int gwPttSosMsg(int sid, const char *snm, int id, long long ms, double lat, double lon, char hasLoc);

int gwPttSendSelf(int rid, int type, char *data, int len, char offline);

int gwPttSendText(int rid, int type, char *data, int len);

int gwPttAddDelGroupOfflineMsg(int group, int type, int act);

int gwPttExitMsg(void);

int gwPttReportLocation(GWPttLocation *location, char **out, long long time);

int gwPttReportCurrentLocation(GWPttLocation *loc,char **out,long long time);

int gwPttUploadLastestLocation(char *data, char **response);

int gwPttUploadLocation(char *data, char **response);

int gwPttGetWeather(char *param, char **response);

int gwPttUploadPatrol(char *data, char **response);

int gwPttUploadFile(char *data, int type, char **response);

int gwPttGetLocationByBS(char *param, char **response);

unsigned int gwPttGetTime(void);

int gwPttRecordDataReady(int len, long long ms);

void gwPttUpdateLeftVoicePacket(int num);

int gwPttVoiceAttachBuffer(int type, char *pcm, int size);

int gwPttEnterAI(char *role, char hasloc, double lat, double lon);

int gwPttAIPriority(int pri);

int gwPttAIHeart(void);

int gwPttAISpeakStart(void);

int gwPttAISpeakEnd(void);

int gwPttAICancel(void);

int gwPttExitAI(void);

int gwPttConfigAgeGain(int db);

int gwPttConfigAgeBalance(int balanceLevel);

#ifdef __cplusplus
}
#endif

#endif //GW_GW_PTT_H
