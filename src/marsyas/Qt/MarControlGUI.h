#ifndef MARCONTROLGUI_H
#define MARCONTROLGUI_H

#include "common.h"

#ifdef MARSYAS_QT

#include <QtCore>
#include <QtGui>

#include <string>

#include "MarControl.h"

namespace Marsyas
{

class MarControlGUI : public QWidget
{
	Q_OBJECT

private:
	MarControlPtr control_;
	std::string cname_;

	QLabel* nullWidget_;
	QLineEdit* realWidget_;
	QCheckBox* boolWidget_;
	QLineEdit* naturalWidget_;
	QLineEdit* stringWidget_;
	QToolButton* vecWidget_;
	QTableWidget* vecTable_;

	bool editing_;

	QSize sizeHint_;

	void createWidget();

	void nullWidget();
	void realWidget();
	void boolWidget();
	void naturalWidget();
	void stringWidget();
	void vecWidget();

public:
	MarControlGUI(MarControlPtr control, QWidget *parent = 0);
	~MarControlGUI();

	virtual QSize sizeHint() const;

	std::string getCtrlName() const {return cname_;};

private slots:
	void editingStarted();
	
	void showVectorTable();

	void double2MarControl();
	void toMarControl(bool val);
	void int2MarControl();
	void string2MarControl();
	void toMarControl(QTableWidgetItem* cell);

private slots:
	void vecTableDestroyed();

public slots:
	void updControl(MarControlPtr control);

//signals:
	//void controlChanged(std::string cname, MarControlPtr control);

};

}//namespace Marsyas

#endif //MARSYAS_QT

#endif // MARCONTROLGUI_H
