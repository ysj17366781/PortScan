#include <QAbstractTableModel>
#include <QPair>
#include <QList>
#include <QStringList>

typedef struct _SBoardInfos
{
	std::string	type;	//װ������
	int components;		//�����������Ҫ�����ڲ��
	int	locate;			//��������ڲ�۵�λ��,��1��ʼ
	int iports;			//���������
	std::string alias;	//��������
	int	base;			//����������Ż���0���ǻ���1
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

