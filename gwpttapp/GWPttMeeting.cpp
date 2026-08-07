#include "GWPttMeeting.h"
#include <QMessageBox>
#include "GWLog.h"

GWPttMeeting::GWPttMeeting(const QString &name, uint32_t id, bool autoAccept, QWidget *parent)
	: QDialog(parent), 
	uiTimer_(nullptr), 
	meetingTime_(0), 
	meetingCreater_(false),
	meetingName_(name), 
	meetingId_(id),
	autoAccept_(autoAccept), 
	enterMeeting_(false),
	totalUserSize_(0),
	meetingPass_(""),
	mute(false)
{
	//setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
	ui.setupUi(this);
	if (meetingId_ != -1)
	{
		GWLOG_PRINT(GW_LOG_LEVEL_INFO, "create meeting(%s) dialog %d %d", meetingName_.toStdString().c_str(), meetingId_, autoAccept);
	}
	selectMemberUids.clear();
}

GWPttMeeting::~GWPttMeeting()
{
	if (uiTimer_ != nullptr)
	{
		uiTimer_->stop();
		delete uiTimer_;
	}
	uiTimer_ = nullptr;
	selectMemberUids.clear();
	GWPttClient::getPtt()->unregisterObserver(this);
}

void GWPttMeeting::init()
{
	uiTimer_ = new QTimer(this);
	uiTimer_->setInterval(1 * TIME_1_SECOND / 1000);        // 1秒
	uiTimer_->setSingleShot(false);  // 默认就是 false，可省略
	connect(uiTimer_, &QTimer::timeout, this, [this]() {
		meetingTime_++;
		int minutes = meetingTime_ / 60;
		int seconds = meetingTime_ % 60;
		QString timestr = "";
		if (minutes < 10) {
			timestr = "0" + QString::number(minutes);
		}
		else {
			timestr = QString::number(minutes);
		}
		timestr += ":";
		if (seconds < 10) {
			timestr += "0" + QString::number(seconds);
		}
		else {
			timestr += QString::number(seconds);
		}
		ui.label->setText(timestr);
	});
	//uiTimer_->start();
	GWPttClient::getPtt()->registerObserver(this);
	initView();
	initEvent();
}

void GWPttMeeting::initView()
{
	ui.label->setText("00:00");
	if (meetingId_ != -1)
	{
		// accept or reject meeting
		ui.labelMeetingName->setText(meetingName_ + "("+QString::number(meetingId_)+")");
		ui.btnCreateMeeting->setEnabled(false);
		ui.btnQuitMeeting->setEnabled(false);
		ui.btnJoinMeeting->setEnabled(false);
		ui.btnQueryMeeting->setEnabled(false);
		if (autoAccept_)
		{
			ui.btnAcceptMeeting->setEnabled(false);
			ui.btnRejectMeeting->setEnabled(false);
		}
		ui.editMeetingId->setReadOnly(true);
		ui.editPassword->setReadOnly(true);
	}
	else
	{
		// create meeting or join meeting
		ui.btnQuitMeeting->setEnabled(false);
		ui.btnQueryMeeting->setEnabled(false);
		ui.btnAcceptMeeting->setEnabled(false);
		ui.btnRejectMeeting->setEnabled(false);
	}
	ui.listUser->setSelectionMode(QAbstractItemView::MultiSelection);
	ui.pageNumUsr->setValidator(new QIntValidator(1, 99, this));
	ui.editMeetingId->setValidator(new QIntValidator(1, INT_MAX, this));
	ui.speakerList->setReadOnly(true);
	//QString speakerList = "";
	//for (int i = 0; i < 4; i++) {
	//	speakerList += "User"+QString::number(i) + "\n";
	//}
	//ui.speakerList->setText(speakerList);
}

void GWPttMeeting::initEvent()
{
	connect(ui.btnCreateMeeting, &QPushButton::clicked, this, &GWPttMeeting::createMeeting);
	connect(ui.btnQueryMeeting, &QPushButton::clicked, this, &GWPttMeeting::queryMeeting);
	connect(ui.btnJoinMeeting, &QPushButton::clicked, this, &GWPttMeeting::joinMeeting);
	connect(ui.btnQuitMeeting, &QPushButton::clicked, this, &GWPttMeeting::quitMeeting);
	connect(ui.btnAcceptMeeting, &QPushButton::clicked, this, &GWPttMeeting::acceptMeeting);
	connect(ui.btnRejectMeeting, &QPushButton::clicked, this, &GWPttMeeting::rejectMeeting);
	connect(ui.btnQueryUser, &QPushButton::pressed, this, &GWPttMeeting::queryUser);
	connect(ui.listUser, &QListWidget::itemClicked, this, &GWPttMeeting::onUserItemClicked);
	connect(ui.btnMuteMic, &QPushButton::clicked, this, &GWPttMeeting::onMuteClick);

	qRegisterMetaType<Member>();
	qRegisterMetaType<QList<Member>>();
	connect(this, &GWPttMeeting::userListDataReady, this, &GWPttMeeting::onUserListDataReady);

	qRegisterMetaType<GWPttMeetingEventData>();
	connect(this, &GWPttMeeting::meetingEvent, this, &GWPttMeeting::onMeetingEvent);

	if (autoAccept_)
	{
		GWPttClient::getPtt()->acceptMeeting(meetingId_);
	}
}

void GWPttMeeting::updateEnterMeetingView()
{
	if (!meetingName_.isEmpty())
	{
		ui.labelMeetingName->setText(meetingName_ + "(" + QString::number(meetingId_) + ")");
	}
	else
	{
		QString name = QCoreApplication::translate("GWPttAppMainClass", "Meeting", nullptr);
		ui.labelMeetingName->setText(name + "(" + QString::number(meetingId_) + ")");
	}
	ui.btnQueryUser->setEnabled(true);
	ui.btnCreateMeeting->setEnabled(false);
	ui.btnQuitMeeting->setEnabled(true);
	ui.btnJoinMeeting->setEnabled(false);
	ui.btnQueryMeeting->setEnabled(true);
	ui.btnAcceptMeeting->setEnabled(false);
	ui.btnRejectMeeting->setEnabled(false);
	ui.editMeetingId->setReadOnly(true);
	ui.editPassword->setReadOnly(true);
}

bool GWPttMeeting::isUserSelected(uint32_t uid)
{
	for (uint32_t id : selectMemberUids)
	{
		if (id == uid)
		{
			return true;
		}
	}
	return false;
}

void GWPttMeeting::createMeeting()
{
	if (selectMemberUids.empty())
	{
		QString title = QCoreApplication::translate("GWPttNoticeInfo", "Error");
		QString msg = QCoreApplication::translate("GWPttNoticeInfo", "selcetuser");
		QMessageBox::information(this, title, msg);
	}
	else
	{
		GWPttClient::getPtt()->createMeeting(selectMemberUids.data(), selectMemberUids.size());
	}
}

void GWPttMeeting::quitMeeting()
{
	if (meetingCreater_)
	{
		// destroy meeting
		GWPttClient::getPtt()->destroyMeeting(meetingId_);
	}
	else
	{
		// quit meeting
		GWPttClient::getPtt()->leaveMeeting(meetingId_);
	}
}

void GWPttMeeting::joinMeeting()
{
	// show an input dialog
	QString meetingIDStr = ui.editMeetingId->text();
	QString meetingPass = ui.editPassword->text();
	if (meetingIDStr.isEmpty())
	{
		//QString title = QCoreApplication::translate("GWPttNoticeInfo", "Error");
		//QString msg = QCoreApplication::translate("GWPttNoticeInfo", "inputport");
		//QMessageBox::information(this, title, msg);
		return;
	}
	bool ok;
	int mid = meetingIDStr.toInt(&ok);
	GWPttClient::getPtt()->joinMeeting(mid, meetingPass);
}

void GWPttMeeting::queryMeeting()
{
	GWPttClient::getPtt()->queryMeeting(meetingId_);
}

void GWPttMeeting::acceptMeeting()
{
	GWPttClient::getPtt()->acceptMeeting(meetingId_);
}

void GWPttMeeting::rejectMeeting()
{
	GWPttClient::getPtt()->rejectMeeting(meetingId_);
}

void GWPttMeeting::queryUser()
{
	QString num = ui.pageNumUsr->text();
	if (num != "")
	{
		int currentPageNum_ = num.toInt();
		int gid = GWPttClient::getPtt()->getCurrentGroupId();
		GWPttClient::getPtt()->queryUser(gid, currentPageNum_);
	}
	else
	{
		QString title = QCoreApplication::translate("GWPttNoticeInfo", "Error");
		QString msg = QCoreApplication::translate("GWPttNoticeInfo", "Input page");
		QMessageBox::information(this, title, msg);
	}
}

void GWPttMeeting::onMuteClick()
{
	mute = !mute;
	if (mute)
	{
		ui.btnMuteMic->setText(QCoreApplication::translate("GWPttMeetingClass", "Mute", nullptr) + "*");
	}
	else
	{
		ui.btnMuteMic->setText(QCoreApplication::translate("GWPttMeetingClass", "Mute", nullptr));
	}
	GWPttClient::getPtt()->muteMic(mute);
}

void GWPttMeeting::onUserItemClicked(QListWidgetItem *item)
{
	if (!item)
		return;

	int row = ui.listUser->row(item);
	struct Member selectMem = currentPageUserlist_.at(row);
	bool isSelected = item->isSelected();
	if (isSelected)
	{
		QString txt = item->text();
		txt += " *";
		item->setText(txt);
		selectMemberUids.push_back(selectMem.uid);
		
	}
	else
	{
		for (auto iter = selectMemberUids.begin(); iter != selectMemberUids.end(); iter++)
		{
			if (*iter == selectMem.uid)
			{
				item->setText(selectMem.name);
				selectMemberUids.erase(iter);
				break;
			}
		}
	}
}

void GWPttMeeting::onUserListDataReady(const QList<Member>& list)
{
	ui.listUser->clear();
	for (struct Member member : list) {
		bool select = isUserSelected(member.uid);
		if (select) {
			ui.listUser->addItem(member.name+" *");
		}
		else {
			ui.listUser->addItem(member.name);
		}
	}
	currentPageUserlist_.append(list);
	int pageNum = (totalUserSize_ % PTT_QUERY_PAGE_SIZE == 0) ? (totalUserSize_ / PTT_QUERY_PAGE_SIZE) : (totalUserSize_ / PTT_QUERY_PAGE_SIZE + 1);
	ui.labelUsrTotalPage->setText("/" + QString::number(pageNum));
}

void GWPttMeeting::onMeetingEvent(const GWPttMeetingEventData &event)
{
	// 0 create 1 destroy 2 quit  3 join 4 accept 5 reject 6 query  10 invite 11 speaker change
	if (event.action == 0)
	{
		meetingCreater_ = true;
		enterMeeting_ = true;
		meetingId_ = event.meetingId;
		meetingName_ = event.meetingName;
		meetingPass_ = event.meetingPass;
		uiTimer_->start();
		updateEnterMeetingView();
	}
	else if (event.action == 1)
	{
		enterMeeting_ = false;
		meetingId_ = -1;
		accept();
	}
	else if (event.action == 2)
	{
		enterMeeting_ = false;
		meetingId_ = -1;
		accept();
	}
	else if (event.action == 3)
	{
		meetingCreater_ = false;
		enterMeeting_ = true;
		meetingId_ = event.meetingId;
		meetingName_ = event.meetingName;
		meetingPass_ = event.meetingPass;
		uiTimer_->start();
		updateEnterMeetingView();
	}
	else if (event.action == 4)
	{
		meetingCreater_ = false;
		enterMeeting_ = true;
		//meetingId_ = event.meetingId;
		meetingName_ = event.meetingName;
		uiTimer_->start();
		updateEnterMeetingView();
	}
	else if (event.action == 5)
	{
		enterMeeting_ = false;
		meetingId_ = -1;
		accept();
	}
	else if (event.action == 6)
	{
		QString title = QCoreApplication::translate("GWPttNoticeInfo", "Info");
		QString msg = event.meetingName + "\n";
		if (!event.meetingPass.isEmpty())
		{
			msg += QCoreApplication::translate("GWPttMeetingClass", "Password", nullptr) + ":" + event.meetingPass;
		}
		QMessageBox::information(this, title, msg);
	}
	else if (event.action == 10)
	{
		// not process
	}
	else if (event.action == 11)
	{
		QString speakerList = "";
		for (struct Member member : event.meetingUserList) {	
			speakerList += member.name + "\n";
		}
		ui.speakerList->setText(speakerList);
	}
	else if (event.action == 12)
	{
		enterMeeting_ = false;
		meetingId_ = -1;
		accept();
	}
	else
	{
		// not process
	}
}

void GWPttMeeting::onPttClientEvent(int event, void *data)
{
	if (event == PTT_CLIENT_EVENT_QUERYUSER)
	{
		if (data != nullptr)
		{
			GWPttQueryUserEventData *queryData = (GWPttQueryUserEventData*)data;
			if (queryData->totalpage != -1)
			{
				totalUserSize_ = queryData->totalpage;
				QList<Member> userlist;
				userlist.append(queryData->userlist);
				emit userListDataReady(userlist);
			}
		}
		else
		{
			//emit operateFail(0);
		}
	}
	else if (event == PTT_CLIENT_EVENT_MEETING)
	{
		if (data != nullptr)
		{
			GWPttMeetingEventData *meeting = (GWPttMeetingEventData*)data;
			if (meeting->action != 10)
			{
				GWPttMeetingEventData event = *meeting;
				emit meetingEvent(event);
			}
			else
			{
				// not process
			}
		}
		else
		{
			//emit operateFail(0);
		}
	}
}

void GWPttMeeting::closeEvent(QCloseEvent *ev)
{
	if (enterMeeting_)
	{
		if (meetingCreater_)
		{
			// destroy meeting
			GWPttClient::getPtt()->destroyMeeting(meetingId_);
		}
		else
		{
			// quit meeting
			GWPttClient::getPtt()->leaveMeeting(meetingId_);
		}
	}
	else
	{
		if (meetingId_ == -1)
		{
			// do nothing
		}
		else
		{
			// reject meeting
			GWPttClient::getPtt()->rejectMeeting(meetingId_);
		}
	}
}
