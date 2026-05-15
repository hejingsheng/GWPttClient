#pragma once

#include <QMainWindow>
#include <QTimer>
#include "ui_GWPttAppMain.h"
#include "GWPttManager.h"

class GWPttAppMain : public QMainWindow, public GWPttClientCallback
{
	Q_OBJECT

public:
	GWPttAppMain(QWidget *parent = nullptr);
	~GWPttAppMain();

public:
	void init();

signals:
	void groupListDataReady(const QList<Group> &list);
	void userListDataReady(const QList<Member> &list);
	void currentGroupChange(const QString &groupname);
	void generalGroupToken(const GroupOperate &token);
	void speakerInfo(const Speaker &speaker);
	void deleteOrExitGroup(const GroupOperate &token);
	void operateFail(const int code);
	void offline();
	void tmpCallInfo(const int data);
	void nameChangeOrRecvInfo(int type, const QString &data);

protected:
	virtual void onPttClientEvent(int event, void *data);

private:
	void initView();
	void initEvent();

private:
	void queryGroup();
	void onGroupListDataReady(const QList<Group> &list);
	void createGroup();
	void shareGroup();
	void onGroupToken(const GroupOperate &token);
	void showEnterGroupDialog();
	void deleteGroup();
	void onDeletOrExit(const GroupOperate &data);
	void switchGroup();
	void onCurrentGrpChange(const QString &name);
	void onSpeakerInfo(const Speaker &speaker);
	void onOperateFail(const int code);
	void queryUser();
	void onUserListDataReady(const QList<Member> &list);
	void logout();
	void tempCall();
	void onTempCall(const int data);

private:
	void onPttPressed();
	void onPttReleased();

public:
	bool isSos;

private:
	Ui::GWPttAppMainClass ui;
	QTimer *uiTimer_;

private:
	int totalGroupSize_;
	int totalUserSize_;
	int pageSize_ = PTT_QUERY_PAGE_SIZE;
	QList<Group> currentPageGrouplist_;
	QList<Member> currentPageUserlist_;
	bool isTempCall;
};
