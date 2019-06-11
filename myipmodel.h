#include <QAbstractTableModel>
#include <QPair>
#include <QList>
#include <QStringList>

typedef struct _SBoardInfos
{
	std::string	type;	//装置类型
	int components;		//插件数量，主要是网口插件
	int	locate;			//网卡插件在插槽的位置,从1开始
	int iports;			//插件网口数
	std::string alias;	//网卡别名
	int	base;			//网卡别名编号基于0还是基于1
}SBoardInfos;


class CMyIpTableModel:public QAbstractTableModel
{
	Q_OBJECT
public:
	CMyIpTableModel(int row=0, int col=0, QObject *parent=0);
	~CMyIpTableModel(void);


	int rowCount(const QModelIndex &parent=QModelIndex() ) const;
	int columnCount(const QModelIndex &parent=QModelIndex() ) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role); 

private:
	bool isMatchReg(const QModelIndex &index) const ;

signals:
	void ipIllegal(QString);

private:
	int rows;
	int cols;
	QMap<QModelIndex, QString>		m_mapIp;
	QMap<QModelIndex, Qt::CheckState>			m_mapSel;
	QMap<QModelIndex, QVariant>		m_mapColor;
	QMap<QModelIndex, QIcon>		m_mapIcon;
	QMap<QModelIndex, QString>		m_mapBoardName;
};

