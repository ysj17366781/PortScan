#include <QApplication>
#include "pinger.h"
#include "nucommon.h"

#include <qDebug>
#include <QTextCodec>
#include <QProcess>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	app.setWindowIcon(QIcon(":/images/a.png"));
	setCodec();
	loadTranslator("pinger");

	CPingerDlg dlg;
	dlg.show();

	return app.exec();
}