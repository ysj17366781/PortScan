#include "myipmodel.h"
#include <QMutableMapIterator>
#include <QDebug>
#include <QPixmap>
#include <QIcon>

CMyIpTableModel::CMyIpTableModel(int rows, int cols, QObject *parent):QAbstractTableModel(parent)
{
	this->rows = rows;
	this->cols = cols;
	this->insertRows(0, rows);
	this->insertColumns(0, cols);
	m_mapIp.clear();
	m_mapSel.clear();
}


CMyIpTableModel::~CMyIpTableModel(void)
{
}

int CMyIpTableModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return rows;
}

int CMyIpTableModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return cols;
}

QVariant CMyIpTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= rows || index.row() < 0 || index.column() >= cols || index.column() < 0)
		return QVariant();

	switch (role)
	{
	case Qt::BackgroundRole:
		if (index.column() != 0)
		{
			if( !isMatchReg(index))
				return QColor(Qt::gray);
		}
		else
		{
			return QColor(Qt::green);
		}
		break;
	case Qt::DecorationRole:
		{
			if (index.column() != 0)
			{
				if( !isMatchReg(index))
					return QVariant();
				return m_mapIcon[index];
			}
		}
		break;
	case Qt::DisplayRole:
	case Qt::EditRole:
		if (index.column() == 0)
		{
			return m_mapBoardName[index];
		}
		else 
		{
			return m_mapIp[index];
		}
		break;
	case Qt::CheckStateRole:

		if (index.column() != 0)
		{
			if( !isMatchReg(index))
				return QVariant();

			return m_mapSel[index]; 
		}
		break;
	default:
		break;
	}
	return QVariant();
}

QVariant CMyIpTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal) {
		if (section == 0)
		{
			return QString(tr("插件id"));
		}
		else
		{
			return QString(tr("网口%1")).arg(section);
		}
	}
	return QVariant();
}

Qt::ItemFlags CMyIpTableModel::flags( const QModelIndex &index ) const  
{  
	if (!index.isValid())  
		return 0;

	if( !isMatchReg(index))
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable ;

	if (index.column() == 0)
		return  Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
	else
	{
		return  Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
	}
}

bool CMyIpTableModel::setData( const QModelIndex &index, const QVariant &value, int role )  
{  
	int col = index.column();
	if(!index.isValid())  
		return false; 

	switch(role)
	{
	case Qt::CheckStateRole:
		if (index.column() != 0)  
		{ 
			QMutableMapIterator<QModelIndex, Qt::CheckState> i(m_mapSel);

			while (i.hasNext()) {
				i.next();
				if (i.value() == Qt::Checked)
				{
					QModelIndex ltidx = i.key();
					emit dataChanged(ltidx, ltidx);	//刷新项
					i.setValue(Qt::Unchecked);
				}
			}

			if (value == Qt::Checked) 
			{
				m_mapSel[index] = Qt::Checked;
			}  
			else  
			{  
				m_mapSel[index] = Qt::Unchecked;
			}
			emit dataChanged(index, index);	//获取事件
		}
		break;
	case Qt::DisplayRole:
		{
			QString str = value.toString();
			m_mapBoardName[index] = str;
		}
		break;
	case Qt::EditRole:
		{
			QRegExp rx ("^(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|[1-9])\\.(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\.(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\.(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)$");
			QString str = value.toString();
			if( rx.exactMatch(str) )
			{
				m_mapIp[index] = str;
				emit dataChanged(index, index);	//获取事件
			}
			else
			{
				emit ipIllegal(str);
			}
		}
		break;
	case Qt::DecorationRole:
		m_mapIcon[index] = value.value<QIcon>();
		emit dataChanged(index, index);	//刷新
		break;
	default:
		break;
	}
	return true;
}  

bool CMyIpTableModel::isMatchReg(const QModelIndex &index)const
{
	bool ret = false;
	QRegExp rx ("((Q910)|(Q909)|(Q21)|(Q919))");
	const QModelIndex idxHdr = this->index(index.row(), 0);
	const QString str = idxHdr.data().toString();
	if( rx.exactMatch(str))
		ret = true;
	return ret;
}
