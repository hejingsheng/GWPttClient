#include "GWPttQRCodeDialog.h"
#include "qrcodegen.hpp"
#include <QImage>

using namespace qrcodegen;

GWPttQRCodeDialog::GWPttQRCodeDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

GWPttQRCodeDialog::~GWPttQRCodeDialog()
{

}

void GWPttQRCodeDialog::setQRCodeString(const QString &data)
{
	std::vector<QrSegment> segs = QrSegment::makeSegments(data.toLatin1());
	QrCode qr1 = QrCode::encodeSegments(
		segs, QrCode::Ecc::HIGH, 5, 5, 2, false);
	//创建二维码画布
	QImage QrCode_Image = QImage(qr1.getSize(), qr1.getSize(), QImage::Format_RGB888);

	for (int y = 0; y < qr1.getSize(); y++) 
	{
		for (int x = 0; x < qr1.getSize(); x++) 
		{
			if (qr1.getModule(x, y) == 0)
			{
				QrCode_Image.setPixel(x, y, qRgb(255, 255, 255));
			}
			else
			{
				QrCode_Image.setPixel(x, y, qRgb(0, 0, 0));
			}
		}
	}
	QrCode_Image = QrCode_Image.scaled(ui.labelQRCode->width(), ui.labelQRCode->height(),Qt::KeepAspectRatio);
	ui.labelQRCode->setPixmap(QPixmap::fromImage(QrCode_Image));
}
