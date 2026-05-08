#include "GWPttEnterGrpDialog.h"
#include <QIntValidator>
#include <QButtonGroup>

GWPttEnterGrpDialog::GWPttEnterGrpDialog(QWidget *parent)
	: QDialog(parent) , type_(-1) , token_(0)
{
	ui = new Ui::Dialog;
	ui->setupUi(this);
	btnEnterType_ = new QButtonGroup(this);
}

GWPttEnterGrpDialog::~GWPttEnterGrpDialog()
{
	delete ui;
	delete btnEnterType_;
}

void GWPttEnterGrpDialog::init()
{
	ui->ediaToken->setValidator(new QIntValidator(100000, 999999, this));
	connect(ui->btnOK, &QPushButton::clicked, this, &GWPttEnterGrpDialog::onOkBtnClick);

	btnEnterType_->addButton(ui->radioSave, 0);
	btnEnterType_->addButton(ui->radioReplace, 1);
	connect(ui->radioSave, &QRadioButton::clicked, this, &GWPttEnterGrpDialog::onRadioButtonClicked);
	connect(ui->radioReplace, &QRadioButton::clicked, this, &GWPttEnterGrpDialog::onRadioButtonClicked);
}

void GWPttEnterGrpDialog::onOkBtnClick()
{
	QString data = ui->ediaToken->text();
	if (data == "" || type_ == -1)
	{
		reject();
	}
	else
	{
		token_ = data.toInt();
		accept();
	}
}

void GWPttEnterGrpDialog::onRadioButtonClicked()
{
	type_ = btnEnterType_->checkedId();
}
