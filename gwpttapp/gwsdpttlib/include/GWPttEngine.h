/*
 * GWEngine.h
 *
 *  Created on: 2024年5月24日
 *      Author: Administrator
 */

#ifndef GW_GWPTTENGINE_H
#define GW_GWPTTENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GWPttAudioDevice.h"
#include "gw_ptt.h"

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

typedef void (*print_log_cb)(char *log);

DLL_EXPORT int pttSetCustomMalloc(memory_allocate alloc);

DLL_EXPORT int pttInit(GWPttEvent cb, GWMsgEvent cb1, GWPttAudioModule *audioModule, char externalCodec, int encodeLevel, int framesize);

DLL_EXPORT int pttInitWithoutAi(GWPttEvent cb, GWMsgEvent cb1, GWPttAudioModule *audioModule, char externalCodec, int encodeLevel, int framesize);

DLL_EXPORT int pttSetLogCallback(int level, print_log_cb cb);

DLL_EXPORT int pttControlLog(int level, char logToFile);

DLL_EXPORT int pttSetQueuePath(const char *path);

DLL_EXPORT int pttEnableMsg(int enable);

DLL_EXPORT char *pttGetVersion(void);

DLL_EXPORT unsigned int pttGetTime(void);

DLL_EXPORT int pttConfigServer(int type, char *host, int port);

DLL_EXPORT int pttSaveVoice(int open, char *path);

DLL_EXPORT int pttNetCheck(int type, char *host, int port);

DLL_EXPORT int pttLogin(const char *account, const char *pass, const char *imei, const char *iccid);

DLL_EXPORT int pttQueryGroup(void);

DLL_EXPORT int pttQueryGroupByPage(int pageSize, int pageNum);

DLL_EXPORT int pttJoinGroup(int gid, int type);

DLL_EXPORT int pttQueryMember(int gid, int type);

DLL_EXPORT int pttQueryMemberByPage(int gid, int type, int online, int pageSize, int pageNum);

DLL_EXPORT int pttQueryLastMsg(int gid);

DLL_EXPORT int pttQueryDispatcher(int gid, int type);

DLL_EXPORT int pttTempGroup(int *uids, int num);

DLL_EXPORT int pttDuplexCall(int uid, int action);

DLL_EXPORT int pttGroupCreate(const char *gname);

DLL_EXPORT int pttGroupCreateWithParam(const char *prefix,const char *gname,char inviteMg);

DLL_EXPORT int pttGroupAddUser(int gid, int num, int *uids);

DLL_EXPORT int pttGroupDelUser(int gid, int num, int *uids);

DLL_EXPORT int pttGroupDelete(int gid);

DLL_EXPORT int pttGroupRename(int gid, const char *gname);

DLL_EXPORT int pttGroupExit(int gid);

DLL_EXPORT int pttGroupEnter(int gid);

DLL_EXPORT int pttGroupGeneralToken(int *gids, int num);

DLL_EXPORT int pttGroupEnterByToken(int token, int type);

DLL_EXPORT int pttGetManagerInfo(void);

DLL_EXPORT int pttUnbind(void);

DLL_EXPORT int pttSpeakStartDmr(char type, int id, long long ms);

DLL_EXPORT int pttSpeak(int action, long long ms);

DLL_EXPORT int pttRegOfflineMsg(int groups[], int types[], int num, char security);

DLL_EXPORT int pttRegOnlineMsg(int groups[], int types[], int num, char security);

DLL_EXPORT int pttSendMsg(int sid, const char *snm, int type, int id, const char *rnm, int msgType, const char *content, const char *thumburl, int time);

DLL_EXPORT int pttSendTextMsg(int rid,int type,const char *content);

DLL_EXPORT int pttReportLocationGps(double lat, double lon,int type,int uid);

DLL_EXPORT int pttReportLocationCell(int cellid, int lac, int mode, char *mcc, char *mnc,int type,int uid);

DLL_EXPORT int pttReportLocationWifi(char *bssid, int signal, char *ssid);

DLL_EXPORT int pttGetWeather(int cellid, int lac, int mode, char *mcc, char *mnc);

DLL_EXPORT int pttGetLocation(int cellid, int lac, int mode, char *mcc, char *mnc);

DLL_EXPORT int pttPatrol(int id, char *tag, int result, char *desc, int taskId, char *taskNm, char *attach, int uid);

DLL_EXPORT int pttFileUpload(int filetype, char *name, int uid, char *url, char *thumburl, char *desc);

DLL_EXPORT int pttQueryTmpGrp(int type);

DLL_EXPORT int pttHeart(int battery, const char *net);

DLL_EXPORT int pttSendSos(int sid, const char *snm, int id, long long ms, double lat, double lon, char hasLoc);

DLL_EXPORT int pttSendSelfMsg(int id, int type, char *selfmsg, char offline);

DLL_EXPORT int pttVoiceToggle(int dir, int open);

DLL_EXPORT int pttLogout(void);

DLL_EXPORT int pttOnPcmData(char *pcm, int len);

DLL_EXPORT int pttOnEncodeData(char *enc, int len);

DLL_EXPORT int pttUpdateLeftVoicePacket(int num);

DLL_EXPORT int pttAddFriend(int uid);

DLL_EXPORT int pttQueryFriends(int type);

DLL_EXPORT int pttDelFriend(int uid);

DLL_EXPORT int pttAcceptAddFriend(int uid);

DLL_EXPORT int pttRejectAddFriend(int uid);

DLL_EXPORT int pttQueryChatGroups(int uid);

DLL_EXPORT int pttQueryGroupDetail(int gid, int type);

DLL_EXPORT int pttActiveSession(int sid, int type);

DLL_EXPORT int pttDeactiveSession(int sid, int type);

DLL_EXPORT int pttDeleteSession(int sid, int type);

DLL_EXPORT int pttQueryMsgDetail(int gid, int type, char *msgid);

DLL_EXPORT int pttEnterAI(int role, char hasloc, double lat, double lon);

DLL_EXPORT int pttConfigAI(int role, char hasloc, double lat, double lon);

DLL_EXPORT int pttAIPriority(int pri);

DLL_EXPORT int pttAISpeak(int action);

DLL_EXPORT int pttAICancel(void);

DLL_EXPORT int pttExitAI(void);

DLL_EXPORT int pttConfigAgeGain(int db);

DLL_EXPORT int pttConfigAgeBalance(int balance);

#ifdef __cplusplus
}
#endif

#endif /* GW_GWPTTENGINE_H */
