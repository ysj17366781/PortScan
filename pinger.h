#ifndef PINGER_H
#define PINGER_H

#include <QDialog>
#include <QProcess>
#include <QLabel>
#include <QMovie>
#include <QThread>
#include <QStringList>
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QtNetwork>
#include <QModelIndex>
#include <QStandardItemModel>
#include "debugger.h"
#include "myipdelegate.h"
#include "myipmodel.h"

enum READY_RESULT
{
	READY_RESULT_CANCEL = -1,
	READY_RESULT_FAILURE = 0,
	READY_RESULT_SUCCESS = 1
};

enum PROCESSTYPE
{
	PT_PING,
	PT_SETIP,
};

class ShellProcess;
class CPingerDlg : public QDialog
{
	Q_OBJECT
	enum
	{
		COLWIDTH_BOARD	=80,
		COLWIDTH_SEL	=30,
		COLWIDTH_IP		=200,
	};
public:
	explicit CPingerDlg(QDialog *parent = 0);
	virtual ~CPingerDlg(){};
	ShellProcess *shellProcess;
	ShellProcess *shellProcessSetIp;	

public slots:
	void OnPingCommandSuccess(QString ip);
	void OnPingCommandFailed(QString ip);
	void UpdateScanProgress(QString ip);
	void OnSetIpCommand(QString retStr, int retcode);
	void OnDevChanged(QString devName);
	void OnBtnInit();
	bool ModifyLocalIp();
	void ShowInfo(const QString &info, QString color="#0000FF");
	void ShowInfo(QStringList &strlist);
	void onDataChanged(const QModelIndex &ltIndex, const QModelIndex &rbIndex);
	void StartScan();
	void OnIpIllegal(QString);

private:
	void InitTable();
	void GetLocalmachineInfo(QString name="ALL");
	QStringList GetDevConfig();
	void InitState();

	QStringList		m_ethNameList;
	QStringList		m_ipAddressStrList;
	QStringList		m_ipRange;
	QComboBox		*m_cbxNet;
	QComboBox		*m_cbxDev;
	QTableView		*m_table;//模型视图及代理
	//CMyIpTableModel		*m_model;
	QStandardItemModel	*m_model;
	CMyIPDelegate		*m_ipDelegate;

	QProgressBar	*m_progressBar;
	QPushButton		*m_btnInit;
	QPushButton		*m_btnPing;
	QMovie			*m_loadingImage;
	QLabel			*m_loadingLabel;
	QListWidget		*m_lstLog;
	QLabel			*m_lblDisconnectNum;

	QSettings		*m_devInfs;
	QMap<UINT, QList<SBoardInfos>>	m_mapDevInfos;
	QMap<QString, QModelIndex>		m_mapIpIndex;
	QList<SBoardInfos >		m_devInfo;

	double		m_currentScanNum;
	double		m_totalScanNum;
	int			m_offlineCount;
	QString		m_targetIp;
};

//  Ping IP的线程子类
class ShellProcess : public QThread
{
    Q_OBJECT
public:
    explicit ShellProcess(QObject *parent = 0, PROCESSTYPE type=PT_PING, int maxPorts=6);
 
    QStringList GetIpRangle(); //获取需要扫描的IP列表
    void SetIpRange(QStringList ipRange); //设置需要扫描的IP列表
	void SetLocEthName(QString name);
 
protected:
    void run();
 
signals:
    void PingCommandSuccessed(QString ip);
    void PingCommandFailed(QString ip);
	void SetIpCommand(QString outstr, int retcode);
 
public slots:

private:
    QStringList m_ipRange; //需要扫描的IP列表
	PROCESSTYPE	m_type;
	QString m_ethName;
	int m_maxPorts;
};


CPingerDlg* GetMainWindow();
#endif//PINGER_H