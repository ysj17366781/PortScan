#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <string>
#include <QDir>
#include <QTime>
#include <QCoreApplication>
#include <qDebug>
#include <QtCore/QProcess>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollArea>
#include "pinger.h"
#include "debugger.h"
#include "util.h"


using std::string;

const int DLG_WIDTH				= 1200;
const int DLG_HEIGHT			= 800;
#define CONFIGPATH				"./pingconfig.ini"
#define NETOKSTR				"TTL="
#define CONNECTIP				"192.178.111.%1"

#include <QApplication>
CPingerDlg *GetMainWindow()
{
	QWidgetList list = qApp->allWidgets();
	foreach (QWidget* widget, list)
	{
		if (widget->inherits("CPingerDlg"))
		{
			return qobject_cast<CPingerDlg*>(widget);
		}
	}

	return NULL;
}
 
ShellProcess::ShellProcess(QObject *parent, PROCESSTYPE type, int maxPorts) :
    QThread(parent)
{
	m_type = type;
	m_maxPorts = maxPorts;
}
 
QStringList ShellProcess::GetIpRangle()
{
    return this->m_ipRange;
}
 
void ShellProcess::SetIpRange(QStringList ipRange)
{
    this->m_ipRange = ipRange;
}

void ShellProcess::run()
{
	QProcess exc;
	QTextCodec *codec = QTextCodec::codecForName("GBK");
	switch(m_type)
	{
	case PT_PING:
		//����ɨ��Ip�б��еĸ���IP
		foreach(QString ip, m_ipRange )
		{
		//��ÿ��Ipִ��ping���������Ƿ�����
			qDebug() << "ping " + ip << endl;
			QString strArg = "ping " + ip + " -n 1 -i 1";//ping���Σ�ÿ�γ�ʱ1��
			exc.start(strArg);
			exc.waitForFinished(-1);
			QString  outstr=codec->toUnicode(exc.readAll());
			qDebug() << outstr;
			int retCode = outstr.indexOf(NETOKSTR);
			if(-1 != retCode)//���ؽ���ַ����д��ڡ�=�������ж�Ϊ��pingͨ
			{
				//it's alive
				qDebug() << "shell ping " + ip + " sucessed!";
				//�����IP���ߵ��ź�
				emit PingCommandSuccessed(ip);
			} else {
				qDebug() << "shell ping " + ip + " failed!";
				//����IP���ߵ��ź�
				emit PingCommandFailed(ip);
			}
			emit SetIpCommand(outstr, retCode);
		}
		break;
	case PT_SETIP:
		for(int i=0; i<m_maxPorts; i++ )
		{
			QString ip = QString("192.168.%1.188").arg(i+1);
			QString mask = QString("255.255.255.0");
			QString strArg = QString("netsh interface ip add address %1 %2 %3").arg(m_ethName).arg(ip).arg(mask);
			qDebug() << strArg;
			exc.start(strArg);
			exc.waitForFinished(1000);
			QString  outstr=codec->toUnicode(exc.readAll());
			qDebug() << outstr;
			strArg = strArg + outstr;
			SetIpCommand(strArg, 0);
		}
		break;
    }
}

void ShellProcess::SetLocEthName( QString name )
{
	m_ethName = name;
}

CPingerDlg::CPingerDlg(QDialog *parent):QDialog(parent)
{
	m_model = NULL;
	GetLocalmachineInfo();

 	QVBoxLayout *pLytMain = new QVBoxLayout;
	//��һ��
 	QVBoxLayout *pLytgroup = new QVBoxLayout;
 	QGroupBox *infoGroupBox = new QGroupBox(this);
 	infoGroupBox->setLayout(pLytgroup);
 	QHBoxLayout *pHlytBox = new QHBoxLayout;
	pLytgroup->addLayout(pHlytBox);
 	QLabel *netName = new QLabel(QObject::tr("ѡ�񱾵�����:"), this);
	pHlytBox->addWidget(netName);
	m_cbxNet = new QComboBox(this);
	m_cbxNet->insertItems(0, m_ethNameList);
	m_cbxNet->setCurrentIndex(0);
	pHlytBox->addWidget(m_cbxNet);

	QLabel *devName = new QLabel(QObject::tr("Select Device:"), this);
	pHlytBox->addWidget(devName);
	QStringList devList = GetDevConfig();	//��ȡ�ⲿ�����ļ�
	m_cbxDev = new QComboBox(this);
	m_cbxDev->insertItems(0, devList);
	m_cbxDev->setCurrentIndex(0);
	pHlytBox->addWidget(m_cbxDev);
	connect(m_cbxDev, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnDevChanged(QString)));

	pHlytBox->addStretch(1);
	m_currentScanNum = 0.0;
	m_totalScanNum = 0.0;
	m_progressBar = new QProgressBar(this);
	m_progressBar->setRange(0, 100);
	m_progressBar->setValue(0);
	m_progressBar->setVisible(false);
	pHlytBox->addWidget(m_progressBar);

	m_loadingImage = new QMovie(":/images/loading.gif");
	m_loadingLabel = new QLabel(this);
	m_loadingLabel->setMovie(m_loadingImage);
	pHlytBox->addWidget(m_loadingLabel);
	pLytgroup->addLayout(pHlytBox);

	// �ڶ���
	m_table = new QTableView(this);
	//m_table->setSortingEnabled(true);//ʹ������
	m_table->setEditTriggers(QTableView::AllEditTriggers);
	//m_table->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);//��ͷ����Զ�����δ�ַ����ݳ���
	//m_table->setSelectionBehavior(QAbstractItemView::SelectRows);//��ѡ��
 	m_table->setShowGrid(true);
 	m_table->setStyleSheet("selection-background-color:lightblue;");
	//m_table->horizontalHeader()->setStretchLastSection(true);//��ͷ���һ�п�����������ಿ��
	m_table->showMaximized();
	
	//m_table->verticalHeader()->hide();//��ͷ����
	pLytgroup->addWidget(m_table);

	InitTable();

	m_lstLog = new QListWidget(this);
	pLytgroup->addWidget(m_lstLog);
	pLytMain->addWidget(infoGroupBox);

	// ������
	pHlytBox = new QHBoxLayout;
	m_lblDisconnectNum = new QLabel(this);
	pHlytBox->addWidget(m_lblDisconnectNum);
	pHlytBox->addStretch(1);
	m_btnInit = new QPushButton(QObject::tr("Modify IP"));
	connect(m_btnInit,SIGNAL(clicked()),this,SLOT(OnBtnInit()));
	pHlytBox->addWidget(m_btnInit);

	m_btnPing = new QPushButton(QObject::tr("Ping"));
	connect(m_btnPing,SIGNAL(clicked()),this,SLOT(StartScan()));
	pHlytBox->addWidget(m_btnPing);
	//m_btnPing->setDisabled(true);
	
	pLytMain->addLayout(pHlytBox);

	setLayout(pLytMain);
	setFixedSize(DLG_WIDTH, DLG_HEIGHT);
	//showMaximized();
}

QStringList CPingerDlg::GetDevConfig()
{
	QSettings devInfs(CONFIGPATH, QSettings::IniFormat);
	QStringList lst = devInfs.childGroups();
	QList<SBoardInfos > devs;
	for (int i=0; i<lst.size(); i++)
	{
		devs.clear();
		QString key = QString("%1/components").arg(lst[i]);
		int compNum = devInfs.value(key, 0).toInt();
		for (int j=1; j<=compNum; j++)
		{
			SBoardInfos dev;
			dev.type = lst[i].toStdString();
			dev.components = compNum;
			key = QString("%1/comp%2/locate").arg(lst[i]).arg(j);
			dev.locate = devInfs.value(key, 1).toInt();
			key = QString("%1/comp%2/iports").arg(lst[i]).arg(j);
			dev.iports = devInfs.value(key, 1).toInt();
			key = QString("%1/comp%2/alias").arg(lst[i]).arg(j);
			dev.alias = devInfs.value(key, "eth").toString().toStdString();
			key = QString("%1/comp%2/base").arg(lst[i]).arg(j);
			dev.base = devInfs.value(key, 0).toInt();			
			devs.append(dev);
			m_devInfo.append(devs);
		}
		m_mapDevInfos.insert(i, devs);

	}
	return lst;
}

//����һ��ʹ���Զ������
void CPingerDlg::InitTable()
{
	m_totalScanNum = 0;
	int devId = m_cbxDev->currentIndex();
	QList<SBoardInfos> boardsInfo = m_mapDevInfos[devId];
	int rows = m_mapDevInfos[devId][0].components;
	int cols = m_mapDevInfos[devId][0].iports;
 	m_ipRange.clear();
 	m_mapIpIndex.clear();
	
	m_ipDelegate = new CMyIPDelegate(m_table);
	m_table->setItemDelegate(m_ipDelegate);
	m_model= new QStandardItemModel(rows, cols+1);
	connect(m_ipDelegate, SIGNAL(dedataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &)));
	connect(m_ipDelegate, SIGNAL(ipdelegateIllegal(QString)),this, SLOT(OnIpIllegal(QString)));
	m_table->setModel(m_model);
	for (int row=0; row<rows; row++)
	{
		QModelIndex idx = m_model->index(row, 0, QModelIndex());
		m_model->setData(idx, QVariant(QString(tr("���%1")).arg(row)), Qt::DisplayRole);
		m_table->setColumnWidth(0, COLWIDTH_BOARD);
		for (int col=1; col<=cols; col++)
		{
			idx = m_model->index(row, col, QModelIndex());
			{
				QString ip = QString("192.168.%1.%2").arg(col).arg(10*(m_mapDevInfos[devId][row].locate-1)+1);
				SMyData data;
				data.str = ip;
				data.state = Qt::Unchecked;
				data.pingState = -1;
				QVariant v;
				v.setValue(data);
				m_model->setData(idx, v, Qt::EditRole);
				m_table->setColumnWidth(col, COLWIDTH_IP);
				m_mapIpIndex.insert(ip, idx);
				m_ipRange.append(ip);
				m_totalScanNum++;
			}
		}
	}
}

void CPingerDlg::onDataChanged(const QModelIndex &ltIndex, const QModelIndex &rbIndex)
{
	int devId = m_cbxDev->currentIndex();
	int row = ltIndex.row();
	int col = ltIndex.column();
	m_mapIpIndex.clear();
	m_ipRange.clear();
	m_targetIp = "";
	int rows = m_mapDevInfos[devId][0].components;
	int cols = m_mapDevInfos[devId][0].iports;

	for (int row=0; row < rows; row++)
	{
		for (int col=0; col <= cols; col++)
		{
			if (col == 0)
				continue;
			else
			{
				QModelIndex idx = m_model->index(row, col);
				SMyData data = m_model->data(idx, Qt::DisplayRole).value<SMyData>();
				if (data.state == Qt::Checked)
				{
					m_targetIp = data.str;
				}
				m_ipRange.append(data.str);
				m_mapIpIndex.insert(data.str, idx);
			}	
		}
	}
}


void CPingerDlg::OnPingCommandSuccess( QString ip )
{
	SMyData data = m_mapIpIndex[ip].data().value<SMyData>();
	data.pingState = 0;
	QVariant v;
	v.setValue(data);
	m_model->setData(m_mapIpIndex[ip], v, Qt::EditRole);
}

void CPingerDlg::OnPingCommandFailed( QString ip )
{
	SMyData data = m_mapIpIndex[ip].data().value<SMyData>();
	data.pingState = 1;
	QVariant v;
	v.setValue(data);
	m_model->setData(m_mapIpIndex[ip], v, Qt::EditRole);
	m_offlineCount++;
}

//��������ʹ���Զ���ģ��
// void CPingerDlg::InitTable()
// {
// 	m_totalScanNum = 0;
// 	QList<SBoardInfos> boardsInfo = m_devInfo;
// 	int rows = m_devInfo.size();
// 	int cols = m_devInfo[0].iports;
// 	m_ipRange.clear();
// 	m_mapIpIndex.clear();
// 
// 	m_model = new CMyIpTableModel(rows, cols+1);
// 	m_table->setModel(m_model);
// 	connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &)));
// 	connect(m_model, SIGNAL(ipIllegal(QString)),this, SLOT(OnIpIllegal(QString)));
// 
// 	for (int row=0; row<rows; row++)
// 	{
// 		QModelIndex idx = m_model->index(row, 0, QModelIndex());
// 		m_model->setData(idx, QVariant(QString(m_devInfo[row].type.c_str())), Qt::DisplayRole);
// 		m_model->setData(idx, QVariant(QColor(Qt::green)), Qt::BackgroundRole);
// 		m_table->setColumnWidth(0, COLWIDTH_BOARD);
// 		for (int col=1; col<cols+1; col++)
// 		{
// 			if (m_devInfo[row].iports > 0)
// 			{
// 				idx = m_model->index(row, col, QModelIndex());
// 				QString ip = QString("192.168.%1.%2").arg(col).arg(10*(m_devInfo[row].locate-1)+1);
// 				m_model->setData(idx, QVariant(ip), Qt::EditRole);
// 				m_model->setData(idx, QVariant(QIcon(":/images/nport_normal.png")), Qt::DecorationRole);
// 				m_table->setColumnWidth(col, COLWIDTH_IP);
// 				m_mapIpIndex.insert(ip, idx);
// 				m_ipRange.append(ip);
// 				m_totalScanNum++;
// 			}
// 			else
// 			{
// 				// 				idx = m_model->index(row, col, QModelIndex());
// 				// 				m_model->setData(idx, QVariant(QColor(Qt::Gray)), Qt::BackgroundRole);
// 			}
// 		}
// 	}
// }
// 
// void CPingerDlg::onDataChanged(const QModelIndex &ltIndex, const QModelIndex &rbIndex)
// {
// 	int row = ltIndex.row();
// 	int col = ltIndex.column();
// 	m_mapIpIndex.clear();
// 	m_ipRange.clear();
// 	m_targetIp = "";
// 	for (int row=0; row < m_model->rowCount(ltIndex); row++)
// 	{
// 		for (int col=0; col < m_model->columnCount(ltIndex); col++)
// 		{
// 			if (col == 0)
// 				continue;
// 
// 			if (m_devInfo[row].iports > 0)
// 			{
// 				QModelIndex idx = m_model->index(row, col);
// 				QString ip = m_model->data(idx, Qt::EditRole).toString();
// 				m_ipRange.append(ip);
// 				m_mapIpIndex.insert(ip, idx);
// 				QVariant val = m_model->data(idx, Qt::CheckStateRole);
// 				if (val == Qt::Checked)
// 				{
// 					m_targetIp = ip;
// 				}
// 			}
// 		}
// 	}
// }
// 
// void CPingerDlg::OnPingCommandSuccess( QString ip )
// {
// 	m_model->setData(m_mapIpIndex[ip], QVariant(QIcon(":/images/nport_pass.png")), Qt::DecorationRole);
// }
// 
// void CPingerDlg::OnPingCommandFailed( QString ip )
// {
// 	m_model->setData(m_mapIpIndex[ip], QVariant(QIcon(":/images/nport_failed.png")), Qt::DecorationRole);
// 	m_offlineCount++;
// }

void CPingerDlg::OnIpIllegal(QString strIp)
{
	QString ret = QString("Set IP %1 Failer!\n").arg(strIp);
	ShowInfo(ret, "#FF0000");
}

void CPingerDlg::GetLocalmachineInfo(QString name)
{
	m_ipAddressStrList.clear();
//	�����ȡIP��ַΪrunning״̬������IP��ַ
// 	QString localHostName=QHostInfo::localHostName();
// 	QHostInfo info=QHostInfo::fromName(localHostName);
// 	QString hostName = info.hostName();
// 	foreach(QHostAddress address,info.addresses())
// 	{
// 		if(address.protocol()==QAbstractSocket::IPv4Protocol)//ֻȡipv4Э��ĵ�ַ
// 		{
// 			qDebug()<<address.toString();
// 			m_ipAddressStrList.append(address.toString());
// 		}
// 	}
//��ȡ��������IP�Լ�ϵͳĬ�Ϸ����IP
// 	QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
// 	for (int i = 0; i < ipAddressesList.size(); ++i) {
// 		if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
// 			ipAddressesList.at(i).toIPv4Address()) {
// 				ipAddress = ipAddressesList.at(i).toString();
// 				m_ipAddressStrList.append(ipAddress);
// 		}
// 	}
	QString ipAddress;
	QString ipMask;
	QString ethRegName;
	QString ethName;
//	ʹ�����淽�����ȡ��Ĭ��dhcp��ȡʧ�ܵ�ip��ַ��169.254.*.*
	QList<QNetworkInterface> networkInterfaces = QNetworkInterface::allInterfaces();
	foreach (QNetworkInterface iNetworkInterface, networkInterfaces)
	{
		ethRegName = iNetworkInterface.name();//������Ϊ��̫������ע����еĵ�ַ����"{AE8E83B7-9BF0-490D-BABB-5D7FDB268544}","{B2A77E25-2F81-4B41-9BF7-A0A34DB6066C}"
		ethName = iNetworkInterface.humanReadableName();
		m_ethNameList.append(ethName);
		if (name != "ALL" && ethName != name)
		{
			continue;
		}
		
		QList<QNetworkAddressEntry> addressEntriesList = iNetworkInterface.addressEntries();
		foreach(QNetworkAddressEntry iAddressEntriesList, addressEntriesList)
		{
			if (iAddressEntriesList.ip().toIPv4Address())
			{
				ipAddress = iAddressEntriesList.ip().toString();
				ipMask = iAddressEntriesList.netmask().toString();
				m_ipAddressStrList.append(ipAddress);
			}
		}
	}
	// if we did not find one, use IPv4 localhost
	if (m_ipAddressStrList.isEmpty())
	{
		ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
		m_ipAddressStrList.append(ipAddress);
	}
}

void CPingerDlg::UpdateScanProgress(QString ip)
{
	m_currentScanNum += 1.0;
	double scanProgress = (m_currentScanNum/m_totalScanNum)*100;
	qDebug() << "ip" << ip << " scan finished!";
	qDebug() << "scan progress: " << scanProgress <<endl;
	m_progressBar->setValue(scanProgress);
	if(100 == (int)scanProgress)
	{//IPɨ�����
		//ɨ������˺�loadingͼƬֹͣ�������ؽ�������loadingͼƬ
		m_progressBar->setVisible(false);
		m_loadingLabel->setVisible(false);
	}
}

void CPingerDlg::StartScan()
{
	m_currentScanNum = 0.0;
	m_offlineCount = 0;
	m_lstLog->clear();
	InitState();
	shellProcess= new ShellProcess(this);
	connect(shellProcess, SIGNAL(PingCommandSuccessed(QString)), this, SLOT(OnPingCommandSuccess(QString)));
	connect(shellProcess, SIGNAL(PingCommandFailed(QString)), this, SLOT(OnPingCommandFailed(QString)));
	connect(shellProcess, SIGNAL(finished()), shellProcess, SLOT(deleteLater()));
	//����ɨ������ź�(�ɹ�����ʧ��)��ɨ����Ȳۺ���
	connect(shellProcess, SIGNAL(PingCommandSuccessed(QString)), this, SLOT(UpdateScanProgress(QString)));
	connect(shellProcess, SIGNAL(PingCommandFailed(QString)), this, SLOT(UpdateScanProgress(QString)));
	connect(shellProcess, SIGNAL(SetIpCommand(QString,int)), this, SLOT(OnSetIpCommand(QString,int)));
	shellProcess->SetIpRange(m_ipRange);
	shellProcess->start();

	m_loadingLabel->show();
	m_loadingImage->start();
	m_progressBar->setVisible(true);
	m_lblDisconnectNum->setText(QString("�쳣��������:%1").arg(m_offlineCount));
}

void CPingerDlg::OnDevChanged(QString devName)
{
	//m_table->clearContents();
	InitTable();
}

void CPingerDlg::ShowInfo(const QString &info, QString color)
{
	QString dateTag = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss : ");
	QString data;
	QListWidgetItem * pItem = new QListWidgetItem(dateTag + info);
	pItem->setTextColor(QColor(color));
	m_lstLog->addItem(pItem);
	qApp->processEvents();
}

void CPingerDlg::ShowInfo(QStringList &strlist)
{
	QString ret = strlist.join(",");
	ShowInfo(ret, "#FF0000");
}

void CPingerDlg::OnBtnInit()
{
	QString ret;
	m_lstLog->clear();
	bool bSuc = false;
	bSuc = ModifyLocalIp();
	if (!bSuc)
	{
		ret = "Set local IP Failer!\n";
		ShowInfo(ret, "#FF0000");
	}
	else
	{
		ret = "Set local IP OK!\n";
		m_btnPing->setEnabled(true);
		ShowInfo(ret, "#0000FF");
	}

	ret = "--------------------------------------------";
	ShowInfo(ret, "#0000FF");
	// ִ��ping����
	StartScan();
}

bool CPingerDlg::ModifyLocalIp()
{
	shellProcessSetIp= new ShellProcess(this, PT_SETIP, m_model->columnCount()/2);
	connect(shellProcessSetIp, SIGNAL(finished()), shellProcessSetIp, SLOT(deleteLater()));
	connect(shellProcessSetIp, SIGNAL(SetIpCommand(QString,int)), this, SLOT(OnSetIpCommand(QString,int)));
	QString ethName = m_cbxNet->currentText();
	shellProcessSetIp->SetLocEthName(ethName);
	shellProcessSetIp->start();

	return true;
}

void CPingerDlg::OnSetIpCommand( QString retStr , int retcode)
{
	if (retcode != -1)
	{
		ShowInfo(retStr, "#0000FF");
	}
	else
	{
		ShowInfo(retStr, "#FF0000");
	}
}

void CPingerDlg::InitState()
{
	int devId = m_cbxDev->currentIndex();
	QList<SBoardInfos> boardsInfo = m_mapDevInfos[devId];
	int rows = m_mapDevInfos[devId][0].components;
	int cols = m_mapDevInfos[devId][0].iports;

	QModelIndex idx;
	for (int row=0; row<rows; row++)
	{
		for (int col=1; col<=cols+1; col++)
		{
			idx = m_model->index(row, col, QModelIndex());
			SMyData data = idx.data().value<SMyData>();
			data.pingState = -1;
			QVariant v;
			v.setValue(data);
			m_model->setData(idx, v, Qt::EditRole);
		}
	}
}

