#pragma once

#include <QDialog>
#include <QEvent>
#include <QTimer>
#include "ui_GWPttMeeting.h"
#include "GWPttManager.h"

class GWPttMeeting : public QDialog, public GWPttClientCallback
{
	Q_OBJECT

public:
	GWPttMeeting(const QString &name, uint32_t id, bool autoAccept, QWidget *parent = nullptr);
	~GWPttMeeting();

public:
	void init();

signals:
	void userListDataReady(const QList<Member> &list);
	void meetingEvent(const GWPttMeetingEventData &event);

protected:
	virtual void onPttClientEvent(int event, void *data);

protected:
	void closeEvent(QCloseEvent *ev) override;

private:
	void initView();
	void initEvent();
	void updateEnterMeetingView();

private:
	bool isUserSelected(uint32_t uid);

private:
	void createMeeting();
	void quitMeeting();
	void joinMeeting();
	void queryMeeting();
	void acceptMeeting();
	void rejectMeeting();
	void queryUser();
	void onMuteClick();
	void onUserItemClicked(QListWidgetItem *item);
	void onUserListDataReady(const QList<Member> &list);
	void onMeetingEvent(const GWPttMeetingEventData &event);

private:
	Ui::GWPttMeetingClass ui;
	QTimer *uiTimer_;

private:
	QString meetingName_;
	QString meetingPass_;
	uint32_t meetingId_;
	bool enterMeeting_;
	bool meetingCreater_;
	bool autoAccept_;
	uint32_t meetingTime_;
	std::vector<uint32_t> selectMemberUids;
	bool mute;

private:
	int totalUserSize_;
	int pageSize_ = PTT_QUERY_PAGE_SIZE;
	QList<Member> currentPageUserlist_;
};
