#include <QItemDelegate>
#include <QWidget>
#include <QtGUI>

struct SMyData 
{
	QString			str;
	Qt::CheckState	state;
	int				pingState; //-1为常态，0为通，1为断
};
Q_DECLARE_METATYPE( SMyData)


class CIpCheckWidget: public QWidget
{
	Q_OBJECT
	
public:
	CIpCheckWidget(QWidget *parent);

	SMyData data(){return m_data;};
	void SetData(SMyData &data);
	void SetText(const QString &);
	void SetCheckState(Qt::CheckState state=Qt::Unchecked);
	void SetIcon(const QIcon &);
	

signals:
	void myWidgetIllegal(QString);

private slots:
	void onClicked();
	void onEditingFinished();

private:
	QCheckBox	*m_pCheck;
	QLineEdit	*m_pEdit;
	QWidget		*m_parent;

	SMyData		m_data;
};

class CMyIPDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	CMyIPDelegate(QObject *parent=0);
	~CMyIPDelegate();

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	QSize sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
 	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent ( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index );
	void paintWidget(QPainter* painter,const QRect& rect,const QString& catchKey,QWidget* w) const;

signals:
	void ipdelegateIllegal(QString) const;
	void dedataChanged(const QModelIndex &, const QModelIndex &) const;

private slots:
	void commitAndCloseEditor();
	void onIpWidgetMessage(QString);
};
