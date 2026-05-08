#include "GWPTTLoginWidget.h"
#include "GWPttManager.h"
#include "GWPttAppMain.h"
#include "GWPttConfig.h"
#include "GWPttQRCodeDialog.h"

const QString GWAPP_VERSION = "GW_APP_V1.0.2";

GWPTTLoginWidget::GWPTTLoginWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::GWPttLogin)
{
	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    ui->setupUi(this);
	ui->labelVersion->setText(GWAPP_VERSION);
	initEvent();
}

void GWPTTLoginWidget::initEvent()
{
	ui->editPort->setValidator(new QIntValidator(1024, 65535, this));
	connect(ui->btnCancel, &QPushButton::clicked, this, &GWPTTLoginWidget::close);
	connect(ui->btnLogin, &QPushButton::clicked, this, &GWPTTLoginWidget::loginPtt);
	connect(ui->btnQRCode, &QPushButton::clicked, this, []() {
		ConfigReader config;
		QString deviceid = config.readValue<QString>("Device", "Id", "865223047568037");
		GWPttQRCodeDialog dialog;
		dialog.setQRCodeString(deviceid);
		dialog.exec();
	});

	connect(this, &GWPTTLoginWidget::sendSignalToUI, this, &GWPTTLoginWidget::onLoginReport);

	ConfigReader config;

	QString address = config.readValue<QString>("Server", "Address", "chn-access2c.hawk-sight.com");
	int port = config.readValue<int>("Server", "Port", 23003);
	QString deviceid = config.readValue<QString>("Device", "Id", "865223047568037");

	ui->editAddress->setText(address);
	ui->editPort->setText(QString::number(port));
	ui->editDeviceId->setText(deviceid);
}

void GWPTTLoginWidget::onPttClientEvent(int event, void *data)
{
	emit sendSignalToUI(event, data);
}

void GWPTTLoginWidget::onLoginReport(int event, void *data)
{
	printf("should close this windows and open new windows\n");
	int ret = (int)data;
	if (ret == 0)
	{
		GWPttClient::getPtt()->registerObserver(nullptr);
		GWPttAppMain *mainUi = new GWPttAppMain();
		ConfigReader config;
		config.writeValue("Server", "Address", ui->editAddress->text());
		config.writeValue("Server", "Port", ui->editPort->text());
		config.writeValue("Device", "Id", ui->editDeviceId->text());
		mainUi->init();
		mainUi->show();
	}
	else
	{
		printf("login fail\n");
	}
	close();
}

void GWPTTLoginWidget::loginPtt()
{
	QString address = ui->editAddress->text();
	QString portstr = ui->editPort->text();
	QString deviceId = ui->editDeviceId->text();
	QString password = "111111";
	if (address.isEmpty() || deviceId.isEmpty())
	{
		QString title = "Error";
		QString msg = "Please input server address";
		QMessageBox::information(this, title, msg);
		return;
	}
	if (portstr.isEmpty())
	{
		QString title = "Error";
		QString msg = "Please input server port";
		QMessageBox::information(this, title, msg);
		return;
	}
	if (deviceId.isEmpty())
	{
		QString title = "Error";
		QString msg = "Please input device id(IMEI or MAC)";
		QMessageBox::information(this, title, msg);
		return;
	}

	bool ok;
	int port = portstr.toInt(&ok);
	if (!ok)
	{
		port = 23003;
	}
	//qDebug() <<"data1:" << deviceId;
	//qDebug() <<"data2:" << address;
	//qDebug() <<"data3:" << password;
	//qDebug() <<"data4" << port;
	//address = "chn-access2c.hawk-sight.com";
	//port = 23003;
	//deviceId = "865223047568037";
	GWPttClient::getPtt()->registerObserver(this);
	GWPttClient::getPtt()->initPttMain(deviceId, password, address, port);
	GWPttClient::getPtt()->pttStart();
}

GWPTTLoginWidget::~GWPTTLoginWidget()
{
	delete ui;
}
