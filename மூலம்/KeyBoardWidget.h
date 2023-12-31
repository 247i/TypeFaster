/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as published by
 *  the Free Software Foundation;
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <qwidget.h>
#include <qvbox.h>
#include <qstring.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qstring.h>

#include "TextWidget.h"
#include "MyButton.h"
#include "MyParser.h"
#include "MyButton.h"
#include "MyLayout.h"
#include "wordsforgen.h"

class MyMainWindow;

class KeyBoardWidget : public QWidget
{
	Q_OBJECT
public:
	KeyBoardWidget(QString,QWidget* parent,TextWidget*, QLabel* auth,QComboBox* les,QComboBox* lay,MyMainWindow* ,const char* name=0);
	~KeyBoardWidget();
	void parseFile(QString f,bool h,QString="");
//	void combowidget(QComboBox*);
	QString currentlayout;
	QString currentlesson;
	int currentlessoni;
	void previous();

	QComboBox* lessoncombo;

public slots:
	void layoutActivated(int);
	void lessonActivated(int);
	void next();
	void redo();
private:
	MyMainWindow* mymainwindow;

	std::vector<QString> lesson;
	QLabel* author;
	TextWidget* text;
	bool lefttoright;
//	int fontsize;
	std::vector<MyButton*> v;
	float ratio; //width()/height() of whole layout
	MyLayout* l;
	
	QComboBox* layoutcombo;

	QString layoutslocation;


	void doLesson(QString layoutdir,QString lessonwanted);
	void removeLesson();
	//void genLessons();
	QString all();
	QString slowest();
	QString leastaccurate();
	QString custom(QString req);

	QString homekeys(QString);
	struct lessonandnum
	{
		QString lesson;
		int num;
		lessonandnum(QString l,int n)
		{
			lesson = l;
			num = n;
		}
	};

	vector<wordsforgen*> wordsforgens;
	int wordsforgensi;

		struct letandind
		{
			QString let;
			int ind;
			int homeindex;
			bool ishome;
			letandind(QString a,int b)
			{
				ishome = false;
				let = a;
				ind = b;

			}
		};
	QString buildlesson(QString);
};

#endif
