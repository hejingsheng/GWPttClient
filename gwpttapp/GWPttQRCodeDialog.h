#pragma once

#include <QDialog>
#include "ui_GWPttQRCodeDialog.h"

class GWPttQRCodeDialog : public QDialog
{
	Q_OBJECT

public:
	GWPttQRCodeDialog(QWidget *parent = nullptr);
	~GWPttQRCodeDialog();

public:
	void setQRCodeString(const QString &data);

private:
	Ui::GWPttQRCodeDialogClass ui;
};
