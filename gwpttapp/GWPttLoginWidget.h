#pragma once

#include <QtWidgets>
#include "ui_gwpttlogin.h"
#include "GWPttManager.h"

class GWPTTLoginWidget : public QWidget, public GWPttClientCallback
{
    Q_OBJECT

public:
	GWPTTLoginWidget(QWidget *parent = nullptr);
    ~GWPTTLoginWidget();

signals:
	void sendSignalToUI(int event, void *param);

protected:
	virtual void onPttClientEvent(int event, void *data);

private:
	void initEvent();
	void loginPtt();
	void onLoginReport(int event, void *data);

private:
    Ui::GWPttLogin *ui;
};
