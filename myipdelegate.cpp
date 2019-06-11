#include "myipdelegate.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDebug>

CMyIPDelegate::CMyIPDelegate(QObject *parent)
	:QItemDelegate(parent)
{

}

CMyIPDelegate::~CMyIPDelegate()
{

}

void CMyIPDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	//qDebug()<<"delegate::paint";
	if (index.column()== 0)
	{
		QString str = index.model()->data(index, Qt::DisplayRole).toString();
		QStyleOptionViewItem myOption = option;
		myOption.displayAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
		drawDisplay(painter, myOption, myOption.rect, str);    
	}
	else //if (index.column() %2 == 0)
	{
		bool seleted = option.state & QStyle::State_Selected;
		CIpCheckWidget *wdg = new CIpCheckWidget(NULL);
		SMyData data = index.data().value<SMyData>();
		wdg->SetData(data);
		//直接绘制
// 		QPixmap pixmap(QSize(option.rect.width(), option.rect.height()));
// 		wdg->render(&pixmap);
// 		painter->drawPixmap(option.rect, pixmap);
		//缓冲绘制
		paintWidget(painter
                ,option.rect
                ,QString("%1-%2-%3-%4-%5,%6")
                .arg(seleted)
                .arg(wdg->data().str)
                .arg(wdg->data().state)
                .arg(wdg->data().pingState)
                .arg(wdg->size().width())
                .arg(wdg->size().height())
                ,wdg);
	}
}

void CMyIPDelegate::paintWidget(QPainter* painter,const QRect& rect,const QString& catchKey,QWidget* w) const
{
	QPixmap pixmap(QSize(rect.width(), rect.height()));
    if(!QPixmapCache::find(catchKey,&pixmap))
    {
        w->render(&pixmap);
        QPixmapCache::insert(catchKey,pixmap);
    }
    painter->drawPixmap(rect,pixmap);
}

QSize CMyIPDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	qDebug()<<"delegate::sizeHint";
	return QItemDelegate::sizeHint(option, index);
}

QWidget * CMyIPDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	qDebug()<<"delegate::createEditor";
	if (index.isValid() && index.column() != 0)
	{
		CIpCheckWidget *editor =  new  CIpCheckWidget(parent);
//  		SMyData data = index.model()->data(index, Qt::EditRole).value<SMyData>();
// 		qDebug()<<QString("%1_%2_%3").arg(data.pingState).arg(data.state).arg(data.str);
//  		editor->SetData(data);
		connect(editor, SIGNAL(myWidgetIllegal(QString)), this, SLOT(onIpWidgetMessage(QString)));
		return editor;
	}
	else
	{
		return NULL;
	}
}

void CMyIPDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
	qDebug()<<"delegate::setEditorData";
	if (index.isValid() && index.column() != 0)
	{
		SMyData data = index.model()->data(index, Qt::EditRole).value<SMyData>();
		qDebug()<<QString("%1_%2_%3").arg(data.pingState).arg(data.state).arg(data.str);
		CIpCheckWidget *myEditor = static_cast <CIpCheckWidget*>(editor);
		myEditor->SetData(data);
	}
	else
	{
		QItemDelegate::setEditorData(editor, index);
	}
}

void CMyIPDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
	qDebug()<<"delegate::setModelData";
	if (index.isValid() && index.column() != 0)
	{
		CIpCheckWidget *lineEdit =  static_cast <CIpCheckWidget*>(editor);

		SMyData text = lineEdit->data();
		qDebug()<<QString("%1_%2_%3").arg(text.pingState).arg(text.state).arg(text.str);
		QVariant v;
		v.setValue(text);
		model->setData(index, v, Qt::EditRole);
		emit dedataChanged(index, index);	//获取事件
	}
}

void CMyIPDelegate::commitAndCloseEditor()
{
	qDebug()<<"delegate::commitAndClose";
	QCheckBox *ipEditor = qobject_cast<QCheckBox*>(sender());
	emit commitData(ipEditor);
	emit closeEditor(ipEditor);
}

void CMyIPDelegate::updateEditorGeometry(QWidget *editor,
	const QStyleOptionViewItem &option, const QModelIndex & index ) const
{
	qDebug()<<"delegate::updateEditorGeometry";
	editor->setGeometry(option.rect);
	// 	dynamic_cast<CIpCheckWidget*>(editor)->setFixedSize(option.rect.width(), option.rect.height());
	//  dynamic_cast<CIpCheckWidget*>(editor)->move(option.rect.topLeft());
}

bool CMyIPDelegate::editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index )
{
	qDebug()<<"delegate::editorEvent";
	return QItemDelegate::editorEvent(event, model, option, index);
}

void CMyIPDelegate::onIpWidgetMessage(QString str)
{
	qDebug()<<"delegate::onIpWidgetMessage";
	emit ipdelegateIllegal(str);
}



CIpCheckWidget::CIpCheckWidget( QWidget *parent )
	:QWidget(parent),m_parent(parent)
{
	QHBoxLayout *pHlytBox = new QHBoxLayout;
	m_pCheck = new QCheckBox(this);
	m_pCheck->setIcon(QIcon(":/images/nport_normal.png"));
	pHlytBox->addWidget(m_pCheck);
	connect(m_pCheck, SIGNAL(clicked()), this, SLOT(onClicked()));

	m_pEdit = new QLineEdit(this);
	m_pEdit->setText("127.0.0.1");
	pHlytBox->addWidget(m_pEdit);
	connect(m_pEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

	//pHlytBox->setSizeConstraint(QLayout::SetFixedSize);
	pHlytBox->setSpacing(1);
	pHlytBox->setMargin(1);

	setLayout(pHlytBox);
}

void CIpCheckWidget::onClicked()
{
	m_data.state = m_pCheck->checkState();
}

void CIpCheckWidget::onEditingFinished()
{
	QString str = m_pEdit->text();
	QRegExp rx ("^(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|[1-9])\\.(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\.(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\.(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)$");
	if( rx.exactMatch(str) )
	{
		m_data.str = str;
	}
	else
	{
		emit myWidgetIllegal(str);
	}
}

void CIpCheckWidget::SetCheckState( Qt::CheckState state/*=Qt::Unchecked*/ )
{
	m_pCheck->setCheckState(state);
}

void CIpCheckWidget::SetText(const QString &text)
{
	m_pEdit->setText(text);
}

void CIpCheckWidget::SetIcon( const QIcon &icon )
{
	m_pCheck->setIcon(icon);
}

void CIpCheckWidget::SetData(SMyData &data)
{
	m_data = data;
	m_pCheck->setCheckState(m_data.state);
	if (m_data.pingState == 0)
	{
		m_pCheck->setIcon(QIcon(":/images/nport_pass.png"));
	}
	else if (m_data.pingState == 1)
	{
		m_pCheck->setIcon(QIcon(":/images/nport_failed.png"));
	}
	else
	{
		m_pCheck->setIcon(QIcon(":/images/nport_normal.png"));
	}
	m_pEdit->setText(m_data.str);
	//update();
}


