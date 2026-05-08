#include "GWPttAppMain.h"
#include <QDateTime>
#include <QMessageBox>
#include "GWPttEnterGrpDialog.h"
#include "GWPttConfig.h"

GWPttAppMain::GWPttAppMain(QWidget *parent)
	: QMainWindow(parent)
{
	isTempCall = false;
	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
	ui.setupUi(this);
}

GWPttAppMain::~GWPttAppMain()
{
	if (uiTimer_ != nullptr)
	{
		uiTimer_->stop();
		delete uiTimer_;
	}
	uiTimer_ = nullptr;
	currentPageGrouplist_.clear();
	currentPageUserlist_.clear();
}

void GWPttAppMain::init()
{
	uiTimer_ = new QTimer(this);
	uiTimer_->setInterval(60 * TIME_1_SECOND / 1000);        // 1秒
	uiTimer_->setSingleShot(false);  // 默认就是 false，可省略
	connect(uiTimer_, &QTimer::timeout, this, [this]() {
		uint32_t secs = GWPttClient::getPtt()->getTime();
		QDateTime datetime = QDateTime::fromTime_t(secs);
		QString timestr = datetime.toString("yyyy-MM-dd HH:mm");
		ui.labelTime->setText(timestr);
	});
	uiTimer_->start();
	GWPttClient::getPtt()->registerObserver(this);
	initView();
	initEvent();
}

void GWPttAppMain::onPttClientEvent(int event, void *data)
{
	if (event == PTT_CLIENT_EVENT_QUERYGROUP)
	{
		if (data != nullptr)
		{
			GWPttQueryGroupEventData *queryData = (GWPttQueryGroupEventData*)data;
			if (queryData->totalpage != -1)
			{
				totalGroupSize_ = queryData->totalpage;
				QList<Group> grouplist;
				grouplist.append(queryData->grouplist);
				emit groupListDataReady(grouplist);
			}
		}
		else
		{
			emit operateFail(0);
		}
	}
	else if (event == PTT_CLIENT_EVENT_SWITCHGROUP)
	{
		if (data != nullptr)
		{
			char *name = (char*)data;
			QString groupname = name;
			emit currentGroupChange(groupname);
		}
		else
		{
			emit operateFail(0);
		}
	}
	else if (event == PTT_CLIENT_EVENT_CREATEGROUP)
	{
		if (data != nullptr)
		{
			GroupOperate newGrp(static_cast<GroupOperate*>(data));
			if (newGrp.result == 0)
			{
				emit generalGroupToken(newGrp);
			}
			else
			{
				emit operateFail(newGrp.result);
			}
		}
	}
	else if (event == PTT_CLIENT_EVENT_GROUPTOKEN)
	{
		if (data != nullptr)
		{
			GroupOperate newGrp(static_cast<GroupOperate*>(data));
			if (newGrp.result == 0)
			{
				emit generalGroupToken(newGrp);
			}
			else
			{
				emit operateFail(newGrp.result);
			}
		}
	}
	else if (event == PTT_CLIENT_EVENT_GROUPDELETE)
	{
		if (data != nullptr)
		{
			GroupOperate newGrp(static_cast<GroupOperate*>(data));
			if (newGrp.result == 0)
			{
				emit deleteOrExitGroup(newGrp);
			}
			else
			{
				emit operateFail(newGrp.result);
			}
		}
	}
	else if (event == PTT_CLIENT_EVENT_SPEAK)
	{
		Speaker speaker;
		if (data != nullptr)
		{
			speaker = *((Speaker*)data);
		}
		emit speakerInfo(speaker);
	}
	else if (event == PTT_CLIENT_EVENT_QUERYUSER)
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
			emit operateFail(0);
		}
	}
	else if (event == PTT_CLIENT_EVENT_ENTTMPCALL)
	{
		emit tmpCallInfo(0);
	}
	else if (event == PTT_CLIENT_EVENT_EXTTMPCALL)
	{
		emit tmpCallInfo(1);
	}
	else if (event == PTT_CLIENT_EVENT_ERRTMPCALL)
	{
		emit operateFail(0);
	}
	else if (event == PTT_CLIENT_EVENT_RECVTXT)
	{
		QString txt = (char*)data;
		emit nameChangeOrRecvInfo(0, txt);
	}
	else if (event == PTT_CLIENT_EVENT_NAMECHANGE)
	{
		QString name = (char*)data;
		emit nameChangeOrRecvInfo(1, name);
	}
	else if (event == PTT_CLIENT_EVENT_RECVCMD)
	{
		int cmd = (int)data;
		if (cmd == 14)
		{
			// start alarm
			emit nameChangeOrRecvInfo(2, "");
		}
		else if (cmd == 15)
		{
			// stop alarm
			emit nameChangeOrRecvInfo(4, "");
		}
		else if (cmd == 102)
		{
			// loaction
			ConfigReader config;
			double lat = config.readValue<double>("GPS", "Latitude", 0.0);
			double lon = config.readValue<double>("GPS", "Longitude", 0.0);
			GWPttClient::getPtt()->reportLocation(lat, lon, 1);
		}
		else
		{

		}
	}
	else if (event == PTT_CLIENT_EVENT_REVCLOC)
	{
		QString locationinfo = (char*)data;
		emit nameChangeOrRecvInfo(3, locationinfo);
	}
	else if (event == PTT_CLIENT_EVENT_OFFLINE)
	{
		emit offline();
	}
}

void GWPttAppMain::initView()
{
	const QString username = GWPttClient::getPtt()->getUsername();
	ui.labelUser->setText(username);
	const QString groupname = GWPttClient::getPtt()->getCurrentGroup();
	ui.labelGroup->setText(groupname);
	uint32_t secs = GWPttClient::getPtt()->getTime();
	QDateTime datetime = QDateTime::fromTime_t(secs);
	QString timestr = datetime.toString("yyyy-MM-dd HH:mm");
	ui.labelTime->setText(timestr);

	ui.pageNumGrp->setValidator(new QIntValidator(1, 99, this));
	ui.pageNumUsr->setValidator(new QIntValidator(1, 99, this));

	ui.labelVersion->setText(GWPttClient::getPtt()->getSdkVersion());
}

void GWPttAppMain::initEvent()
{
	connect(ui.btnQueryGrp, &QPushButton::clicked, this, &GWPttAppMain::queryGroup);
	connect(ui.btnCreateGrp, &QPushButton::clicked, this, &GWPttAppMain::createGroup);
	connect(ui.btnEnterGrp, &QPushButton::clicked, this, &GWPttAppMain::showEnterGroupDialog);
	connect(ui.btnShareGrp, &QPushButton::clicked, this, &GWPttAppMain::shareGroup);
	connect(ui.btnDeleteGrp, &QPushButton::clicked, this, &GWPttAppMain::deleteGroup);
	connect(ui.btnSwitchGrp, &QPushButton::clicked, this, &GWPttAppMain::switchGroup);
	connect(ui.btnPTT, &QPushButton::pressed, this, &GWPttAppMain::onPttPressed);
	connect(ui.btnPTT, &QPushButton::released, this, &GWPttAppMain::onPttReleased);
	connect(ui.btnQueryUser, &QPushButton::clicked, this, &GWPttAppMain::queryUser);
	connect(ui.btnTempCall, &QPushButton::clicked, this, &GWPttAppMain::tempCall);
	connect(ui.btnLogout, &QPushButton::clicked, this, &GWPttAppMain::logout);
	qRegisterMetaType<Group>();
	qRegisterMetaType<QList<Group>>();
	connect(this, &GWPttAppMain::groupListDataReady, this, &GWPttAppMain::onGroupListDataReady);
	qRegisterMetaType<Member>();
	qRegisterMetaType<QList<Member>>();
	connect(this, &GWPttAppMain::userListDataReady, this, &GWPttAppMain::onUserListDataReady);
	connect(this, &GWPttAppMain::currentGroupChange, this, &GWPttAppMain::onCurrentGrpChange);
	qRegisterMetaType<GroupOperate>();
	connect(this, &GWPttAppMain::generalGroupToken, this, &GWPttAppMain::onGroupToken);
	connect(this, &GWPttAppMain::deleteOrExitGroup, this, &GWPttAppMain::onDeletOrExit);
	qRegisterMetaType<Speaker>();
	connect(this, &GWPttAppMain::speakerInfo, this, &GWPttAppMain::onSpeakerInfo);
	connect(this, &GWPttAppMain::operateFail, this, &GWPttAppMain::onOperateFail);
	connect(this, &GWPttAppMain::offline, this, [this]() {
		close();
	});
	connect(this, &GWPttAppMain::tmpCallInfo, this, &GWPttAppMain::onTempCall);
	connect(this, &GWPttAppMain::nameChangeOrRecvInfo, this, [this](int type, QString data) {
		if (type == 0)
		{
			QString title = "Info";
			QString msg = "Recv txt:"+data;
			QMessageBox::information(this, title, msg);
		}
		else if (type == 1)
		{
			ui.labelUser->setText(data);
		}
		else if (type == 2)
		{
			QString title = "Warning";
			QString msg = "Open Alarm!!!";
			QMessageBox::information(this, title, msg);
		}
		else if (type == 4)
		{
			QString title = "Warning";
			QString msg = "Close Alarm!!!";
			QMessageBox::information(this, title, msg);
		}
		else if (type == 3)
		{
			if (data.length() > 20)
			{
				data = data.mid(0, 20);
			}
			ui.labelPosition->setText(data);
		}
		else
		{

		}
	});
}

void GWPttAppMain::queryGroup()
{
	QString num = ui.pageNumGrp->text();
	if (num != "")
	{
		int currentPageNum_ = num.toInt();
		GWPttClient::getPtt()->queryGroup(currentPageNum_);
	}
	else
	{
		QString title = "Error";
		QString msg = "Please Input Page Num!!!";
		QMessageBox::information(this, title, msg);
	}
}

void GWPttAppMain::onGroupListDataReady(const QList<Group> &list) 
{
	ui.listGroup->clear();
	for (struct Group grp : list) {
		ui.listGroup->addItem(grp.name+"("+QString::number(grp.author)+")");
	}
	currentPageGrouplist_.append(list);
	int pageNum = (totalGroupSize_ % PTT_QUERY_PAGE_SIZE == 0) ? (totalGroupSize_ / PTT_QUERY_PAGE_SIZE) : (totalGroupSize_ / PTT_QUERY_PAGE_SIZE + 1);
	ui.labelGrpTotalPage->setText("/"+QString::number(pageNum));
}

void GWPttAppMain::createGroup()
{
	GWPttClient::getPtt()->createGroup();
}

void GWPttAppMain::shareGroup()
{
	int row = ui.listGroup->currentRow();
	if (row != -1)
	{
		struct Group selectGrp = currentPageGrouplist_.at(row);
		GWPttClient::getPtt()->shareGroup(selectGrp.gid);
	}
}

void GWPttAppMain::onGroupToken(const GroupOperate &token)
{
	if (token.name != "")
	{
		// new group token
		QString title = "New group:" + token.name;
		QString msg = "Group token:" + QString::number(token.token) + "\nValid time:" + QString::number(token.ttl) + "s";
		QMessageBox::information(this, title, msg);
	}
	else
	{
		// token
		QString title = "Info";
		QString msg = "Group token:" + QString::number(token.token) + "\nValid time:" + QString::number(token.ttl) + "s";
		QMessageBox::information(this, title, msg);
	}
}

void GWPttAppMain::showEnterGroupDialog()
{
	GWPttEnterGrpDialog *dialog = new GWPttEnterGrpDialog(this);
	dialog->init();
	if (dialog->exec() == QDialog::Accepted)
	{
		int type = dialog->getType();
		int token = dialog->getToken();
		GWPttClient::getPtt()->enterGroup(token, type);
	}
	else
	{
		
	}
	delete dialog;
}

void GWPttAppMain::deleteGroup()
{
	int row = ui.listGroup->currentRow();
	if (row != -1)
	{
		struct Group selectGrp = currentPageGrouplist_.at(row);
		if (selectGrp.author == GWPttClient::getPtt()->getUid())
		{
			GWPttClient::getPtt()->deleteGroup(selectGrp.gid);
		}
		else
		{
			GWPttClient::getPtt()->exitGroup(selectGrp.gid);
		}
	}
	else
	{
		QString title = "Error";
		QString msg = "Please select a group";
		QMessageBox::information(this, title, msg);
	}
}

void GWPttAppMain::onDeletOrExit(const GroupOperate &data)
{
	if (data.result == 0)
	{
		for (auto iter = currentPageGrouplist_.begin(); iter != currentPageGrouplist_.end(); ++iter)
		{
			if (data.gid == iter->gid)
			{
				currentPageGrouplist_.erase(iter);
				totalGroupSize_--;
				break;
			}
		}
		ui.listGroup->clear();
		for (struct Group grp : currentPageGrouplist_) {
			ui.listGroup->addItem(grp.name + "(" + QString::number(grp.author) + ")");
		}
		int pageNum = (totalGroupSize_ % PTT_QUERY_PAGE_SIZE == 0) ? (totalGroupSize_ / PTT_QUERY_PAGE_SIZE) : (totalGroupSize_ / PTT_QUERY_PAGE_SIZE + 1);
		ui.labelGrpTotalPage->setText("/"+QString::number(pageNum));
		QString title = "Info";
		QString msg = "Delete success";
		QMessageBox::information(this, title, msg);
	}
	else
	{
		// token
		QString title = "Error";
		QString msg = "Delete group error:" + QString::number(data.result);
		QMessageBox::information(this, title, msg);
	}
}

void GWPttAppMain::switchGroup()
{
	int row = ui.listGroup->currentRow();
	if (row != -1)
	{
		struct Group selectGrp = currentPageGrouplist_.at(row);
		GWPttClient::getPtt()->switchGroup(selectGrp.gid);
	}
	else
	{

	}
}

void GWPttAppMain::onCurrentGrpChange(const QString &name)
{
	ui.labelGroup->setText(name);
}

void GWPttAppMain::onSpeakerInfo(const Speaker &speaker)
{
	if (speaker.name != "")
	{
		ui.labelSpeaker->setText(speaker.name + " is speaking");
	}
	else
	{
		ui.labelSpeaker->setText("IDLE");
	}
}

void GWPttAppMain::onOperateFail(const int code)
{
	QString title = "Error";
	QString msg;
	if (code == 0)
	{
		msg = "Operate fail";
	}
	else
	{
		msg = "Operate fail error code:" + QString::number(code);
	}
	QMessageBox::information(this, title, msg);
}

void GWPttAppMain::logout()
{
	GWPttClient::getPtt()->logout();
}

void GWPttAppMain::tempCall()
{
	if (!isTempCall)
	{
		int row = ui.listUser->currentRow();
		if (row != -1)
		{
			struct Member selectMem = currentPageUserlist_.at(row);
			GWPttClient::getPtt()->tempCall(selectMem.uid);
		}
		else
		{
			QString title = "Error";
			QString msg = "Please select a user";
			QMessageBox::information(this, title, msg);
		}
	}
	else
	{
		int uid = 0;
		GWPttClient::getPtt()->tempCall(uid);
	}
}

void GWPttAppMain::onTempCall(const int data)
{
	if (data == 0)
	{
		ui.labelGroup->setText("Temp Call.");
		isTempCall = true;
	}
	else
	{
		isTempCall = false;
	}
}

void GWPttAppMain::queryUser()
{
	QString num = ui.pageNumUsr->text();
	if (num != "")
	{
		int currentPageNum_ = num.toInt();
		int row = ui.listGroup->currentRow();
		if (row != -1)
		{
			struct Group selectGrp = currentPageGrouplist_.at(row);
			GWPttClient::getPtt()->queryUser(selectGrp.gid, currentPageNum_);
		}
		else
		{
			QString title = "Error";
			QString msg = "Please select a group";
			QMessageBox::information(this, title, msg);
		}
	}
	else
	{
		QString title = "Error";
		QString msg = "Please Input Page Num!!!";
		QMessageBox::information(this, title, msg);
	}
}

void GWPttAppMain::onUserListDataReady(const QList<Member> &list)
{
	ui.listUser->clear();
	for (struct Member member : list) {
		ui.listUser->addItem(member.name + "(" + QString::number(member.online) + ")");
	}
	currentPageUserlist_.append(list);
	int pageNum = (totalUserSize_ % PTT_QUERY_PAGE_SIZE == 0) ? (totalUserSize_ / PTT_QUERY_PAGE_SIZE) : (totalUserSize_ / PTT_QUERY_PAGE_SIZE + 1);
	ui.labelUsrTotalPage->setText("/"+QString::number(pageNum));
}

void GWPttAppMain::onPttPressed()
{
	GWPttClient::getPtt()->pttSpeak(0);
}

void GWPttAppMain::onPttReleased()
{
	GWPttClient::getPtt()->pttSpeak(1);
}
