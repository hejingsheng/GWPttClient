#pragma once

#include <QDialog>
#include "ui_GWPttEnterGrpDialog.h"

class GWPttEnterGrpDialog  : public QDialog
{
	Q_OBJECT

public:
	GWPttEnterGrpDialog(QWidget *parent);
	~GWPttEnterGrpDialog();

public:
	void init();

public:
	int getToken() const {
		return token_;
	}

	int getType() const {
		return type_;
	}

private:
	void onOkBtnClick();
	void onRadioButtonClicked();

private:
	int token_;
	int type_;

private:
	Ui::Dialog *ui;
	QButtonGroup *btnEnterType_;
};
