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
#include "TextWidget.h"
#include <qpainter.h>
#include <qpixmap.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qstring.h>
#include <qevent.h>
//#include "helpfile.h"
#include "MyMainWindow.h"

#include <iostream>
using namespace std;

MyMainWindow* mymainwindow;
extern QString USERNAME;
extern bool MULTI;

extern QDict<bool>* lettersonlydict;

#ifdef FEST
#include <qthread.h>
#include "flite.h"
extern QString helpfile;
//bool festerror = false;
TextWidget* twpointer = 0;
int readers = 0;
bool stopsayinghelp = false;
bool donotstart = false;
bool gotfocus = false;
bool resultfocus;

extern "C"
{
	cst_voice* register_cmu_us_kal();
}
cst_voice* voice;

#ifdef WIN
//__declspec(dllimport) void initialize_festival(void);
//__declspec(dllimport) void speak(const char* );

#include <windows.h>
#include <process.h>
/*
void(*fliteSay)(const char*); //these are the function pointers, they will point at the right methods in the dll
FARPROC fliteInit;
FARPROC fliteCleanup;

HMODULE hTME=NULL;
*/
HANDLE freadblock = CreateSemaphore(NULL,1,1,NULL);

HANDLE fwriteblock = CreateSemaphore(NULL,1,1,NULL);
	
HANDLE fmutex1 = CreateSemaphore(NULL,1,1,NULL);

HANDLE fmutex2 = CreateSemaphore(NULL,1,1,NULL);

//int readers = 0; //this is quite similar to reader writers problem, I want many threads
//to be able to say stuff but as soon as I shut down, no more saying threads must be 
//allowed and the destructor must wait for all of the saying threads to finish
//bool stopsayinghelp;
//bool donotstart = false;
//bool gotfocus = false;
//bool resultfocus;

DWORD WINAPI threadSay(LPVOID x)  //when new threads are created to say something, this is where they start
{
	while(WaitForSingleObject(freadblock,INFINITE)!=WAIT_OBJECT_0);
		while(WaitForSingleObject(fmutex1,INFINITE)!=WAIT_OBJECT_0);
			readers++;
			if(readers==1)
			{
				while(WaitForSingleObject(fwriteblock,INFINITE)!=WAIT_OBJECT_0);
			}
		ReleaseSemaphore(fmutex1,1,NULL);
	ReleaseSemaphore(freadblock,1,NULL);


	QCustomEvent* evstop = new QCustomEvent(346797); //arb big number
	QThread::postEvent(twpointer,evstop);

	if(x!=0 && ((QString *)(x))->length() > 0)
	{
		QString* qs = (QString*)x;
		if(gotfocus || resultfocus)
			flite_text_to_speech(qs->latin1(),voice,"play");
		/*{
			cst_utterance *u;
			cst_wave *w;
			u = flite_synth_text(qs->latin1(),voice);
			if (u != NULL)
			{
				w = utt_wave(u);
				play_wave(w);
				delete_utterance(u);
		//		delete_wave(w);
			}
			
		}*/
		//by a whole lot of code that surrounds it
		delete qs;
	}

	while(WaitForSingleObject(fmutex1,INFINITE)!=WAIT_OBJECT_0);
		if(!donotstart)
		{
			QCustomEvent* evstart = new QCustomEvent(346798); //arb big number
			QThread::postEvent(twpointer,evstart);
		}
		readers--;
		if(readers==0)
			ReleaseSemaphore(fwriteblock,1,NULL);
	ReleaseSemaphore(fmutex1,1,NULL);

	return 0;
}

DWORD WINAPI threadSayHelpFile(LPVOID)
{
  while(WaitForSingleObject(fmutex2,INFINITE)!=WAIT_OBJECT_0);

	while(WaitForSingleObject(freadblock,INFINITE)!=WAIT_OBJECT_0);
		while(WaitForSingleObject(fmutex1,INFINITE)!=WAIT_OBJECT_0);
			readers++;
			if(readers==1)
				while(WaitForSingleObject(fwriteblock,INFINITE)!=WAIT_OBJECT_0);

			donotstart = true;
			QCustomEvent* evstop = new QCustomEvent(346797); //arb big number
			QThread::postEvent(twpointer,evstop);
		ReleaseSemaphore(fmutex1,1,NULL);
	ReleaseSemaphore(freadblock,1,NULL);


	int index=0;
	int previndex=0;
	QString htemp = helpfile;
	while(index < htemp.length()-1)
	{
		while((index < htemp.length()) && (QChar(htemp[index])!="." ))//|| QChar(htemp[index])!="!" || QChar(htemp[index])!="?"))
		{
			if(QChar(htemp[index])=="<")
			{
				int tagind = index;
				while(tagind < htemp.length() && QChar(htemp[tagind])!=">")
					tagind++;
				QString tag = htemp.mid(index,tagind-index+1);
				//cout << tag.latin1() << endl;
				if(tag=="</h1>" || tag=="</h2>")
				{
					htemp.replace(index,tagind-index+1,". ");
					break;
				}
				else
				{
					htemp.replace(index,tagind-index+1," ");
				}
			}
			index++;
		}
		if(stopsayinghelp)
			break;
		QString say = htemp.mid(previndex,index-previndex+1);
		previndex = ++index;
		//fliteSay(say.latin1());
		//flitesaytext(say.latin1());
		flite_text_to_speech(say.latin1(),voice,"play");
	}

	if(!stopsayinghelp && mymainwindow!=0 && mymainwindow->helpbrowser!=0) //got here because finished
	{
		mymainwindow->helpbrowser->hide();
		mymainwindow->setActiveWindow();
	}

	while(WaitForSingleObject(fmutex1,INFINITE)!=WAIT_OBJECT_0);
		donotstart = false;
		QCustomEvent* evstart = new QCustomEvent(346798); //arb big number
		QThread::postEvent(twpointer,evstart);

		readers--;
		if(readers==0)
			ReleaseSemaphore(fwriteblock,1,NULL);
	ReleaseSemaphore(fmutex1,1,NULL);

  ReleaseSemaphore(fmutex2,1,NULL);
  return 0;
}

#else //unix style

#include <pthread.h>

pthread_mutex_t freadblock;
pthread_mutex_t fwriteblock;
pthread_mutex_t fmutex1;
pthread_mutex_t fmutex2;

void* threadSay(void* x)  //when new threads are created to say something, this is where they start
{
	pthread_mutex_lock(&freadblock);
		pthread_mutex_lock(&fmutex1);
			readers++;
			if(readers==1)
				pthread_mutex_lock(&fwriteblock);
		pthread_mutex_unlock(&fmutex1);
	pthread_mutex_unlock(&freadblock);


	QCustomEvent* evstop = new QCustomEvent(346797); //arb big number
	//QThread::postEvent(twpointer,evstop);
	QApplication::postEvent(twpointer,evstop);

	if(x!=0 && ((QString *)(x))->length() > 0)
	{
		QString* qs = (QString*)x;
		if(gotfocus || resultfocus)
			flite_text_to_speech(qs->latin1(),voice,"play");
		delete qs;
	}
	pthread_mutex_lock(&fmutex1);
		if(!donotstart)
		{
			QCustomEvent* evstart = new QCustomEvent(346798); //arb big number
			//QThread::postEvent(twpointer,evstart);
			QApplication::postEvent(twpointer,evstart);
		}
		readers--;
		if(readers==0)
			pthread_mutex_unlock(&fwriteblock);
	pthread_mutex_unlock(&fmutex1);
		
	return 0;
}

void* threadSayHelpFile(void* unused)
{
	pthread_mutex_lock(&fmutex2);
	
	pthread_mutex_lock(&freadblock);
		pthread_mutex_lock(&fmutex1);
			readers++;
			if(readers==1)
				pthread_mutex_lock(&fwriteblock);

			donotstart = true;
			QCustomEvent* evstop = new QCustomEvent(346797); //arb big number
			//QThread::postEvent(twpointer,evstop);
			QApplication::postEvent(twpointer,evstop);
			
		pthread_mutex_unlock(&fmutex1);
	pthread_mutex_unlock(&freadblock);
			


	int index=0;
	int previndex=0;
	QString htemp = helpfile;
	while(index < htemp.length()-1)
	{
		while((index < htemp.length()) && (QChar(htemp[index])!="." ))//|| QChar(htemp[index])!="!" || QChar(htemp[index])!="?"))
		{
			if(QChar(htemp[index])=="<")
			{
				int tagind = index;
				while(tagind < htemp.length() && QChar(htemp[tagind])!=">")
					tagind++;
				QString tag = htemp.mid(index,tagind-index+1);
				if(tag=="</h1>" || tag=="</h2>")
				{
					htemp.replace(index,tagind-index+1,". ");
					break;
				}
				else
				{
					htemp.replace(index,tagind-index+1," ");
				}
			}
			index++;
		}
		if(stopsayinghelp)
			break;
		QString say = htemp.mid(previndex,index-previndex+1);
		previndex = ++index;
		flite_text_to_speech(say.latin1(),voice,"play");
	}

	if(!stopsayinghelp && mymainwindow!=0 && mymainwindow->helpbrowser!=0) //got here because finished
	{
		mymainwindow->helpbrowser->hide();
		mymainwindow->setActiveWindow();
	}

	pthread_mutex_lock(&fmutex1);
		donotstart = false;
		QCustomEvent* evstart = new QCustomEvent(346798); //arb big number
		//QThread::postEvent(twpointer,evstart);
		QApplication::postEvent(twpointer,evstart);

		readers--;
		if(readers==0)
			pthread_mutex_unlock(&fwriteblock);
	pthread_mutex_unlock(&fmutex1);
		
  pthread_mutex_unlock(&fmutex2);
  return 0;
}
#endif

#endif

#ifdef FEST
void TextWidget::customEvent(QCustomEvent * a)
{
	//static int studentatlag = 0;
	if(a->type() == 346798)
//		delay->start(2000,true);
	{
		if(delay->isActive())
			delay->changeInterval(2000);
		else
			delay->start(2000,true);
	}
	else
		if(a->type() == 346797)
			delay->stop();
	
}

void TextWidget::sayhelpfile()
{
//  if(!festerror)
//  {
	    stopsayinghelp = false;
#ifdef WIN
		DWORD targetThreadID;
		CloseHandle((HANDLE)_beginthreadex(NULL,0,(unsigned(_stdcall*)(void*))threadSayHelpFile,0,0,(unsigned*)&targetThreadID));
	//	CloseHandle(&targetThreadID);
#else
	  pthread_t mythread;
	  pthread_create(&mythread,0,threadSayHelpFile,0);
	  pthread_detach(mythread);
#endif
//  }
}
void TextWidget::stopsayhelpfile()
{
	stopsayinghelp = true;
}

void TextWidget::focusInEvent(QFocusEvent*)
{
	gotfocus = true;
}
void TextWidget::focusOutEvent(QFocusEvent*)
{
	gotfocus = false;
}
#endif

TextWidget::TextWidget(int fs,bool p,QWidget* parent,QToolButton* inc,QToolButton* dec,QToolButton* pchart,QProgressBar* prog,MyMainWindow* m,bool shb,bool alb,const char* name):QWidget(parent,name)
{
	
	allowbackspace = alb;
	mymainwindow = m;

#ifdef FEST
	twpointer = this;
	saiduntil = 0;

	showhelpb = shb;

	flite_init();
	voice=register_cmu_us_kal();
#ifdef WIN
	/*
	hTME=LoadLibrary("flite1.2unofficial.dll"); 

	if (hTME!=NULL) 
	{ 

		fliteInit=GetProcAddress(hTME,"fliteinitialize");
		fliteCleanup=GetProcAddress(hTME,"flitecleanup");
		fliteSay=(void(*)(const char*))GetProcAddress(hTME,"flitesaytext"); 

	
		if(fliteInit==NULL || fliteCleanup==NULL || fliteSay==NULL) 
		{ 
			QMessageBox::critical(0,"TypeFaster error","Error getting function addresses in speaking dll");
			festerror = true;
		} 
		else
			fliteInit();		
	}
	else
	{
		//QMessageBox::critical(0,"TypeFaster error loading dll","Error loading dll for speaking");
		festerror = true;
	}*/
#else
	pthread_mutex_init(&freadblock,0);
	pthread_mutex_init(&fwriteblock,0);
	pthread_mutex_init(&fmutex1,0);
	pthread_mutex_init(&fmutex2,0);
	
/*	flite_init();
	voice=register_cmu_us_kal();
	*/
#endif
	delay = new QTimer(this); //will be deleted when this is deleted
	connect(delay,SIGNAL(timeout()),this,SLOT(timeEvent()));
#endif

	scoredialog = new MyScoreDialog(this);
	connect(pchart,SIGNAL(clicked()),this,SLOT(showScoreDialog()));
	connect(scoredialog->history,SIGNAL(clicked()),this,SLOT(clearHistory()));

	enterhighlighted = new std::vector<HighlightDetails>;

	increasefontwidget = inc;
	connect(inc, SIGNAL(clicked()), this, SLOT(increaseFontSize()));
	inc->setAccel(Key_F3);
	decreasefontwidget = dec;
	connect(dec, SIGNAL(clicked()), this, SLOT(decreaseFontSize()));
	dec->setAccel(Key_F4);
	progress = prog;

	totalerrors = 0;
//	totalspaces = 0;
	mydialog = new MyDialog();
	redo = new QPushButton(QPushButton::tr("Redo Lesson"),mydialog->grid);
	redo->setDefault(true);
	redo->setFocus();
	redo->setAccel(Key_F1);
	next = new QPushButton(QPushButton::tr("Next Lesson"),mydialog->grid);
	next->setAccel(Key_F2);

	withinlesson = false;
	newlesson = false;
	timer = new QTime();
	timer->start();

	keypresstimer = new QTime();
	keypresstimer->start(); //this is actually unnec

	blackendindex = 0;
	blackstartindex = 0;
	studentatindex = 0;
	studentatline = 0; 
	charwidth = 1;
	charheight = 1;
//	setMinimumWidth(QApplication::desktop()->width());
	widgetwidth = QApplication::desktop()->width();
	widgetheight = QApplication::desktop()->height();
	//setBaseSize(widgetwidth,widgetheight);
//	setGeometry(0,0,QApplication::desktop()->width(),QApplication::desktop()->height());
//	updateGeometry();

	blacklength = 0;
	greys = 0;

	rtl = false;

	blacksubstring = "";
	greysubstring = "";
	lesson = "";

	//	setBackgroundMode( QWidget::PaletteBase );
	setBackgroundColor("white");

	setFocusPolicy(QWidget::WheelFocus);
	setFocus();

	khighlights = new QDict<highlightvector>(109);
	khighlights->setAutoDelete(true);
#ifdef FEST
	valuetosay = new QDict<QString>(109);
#endif

	//lesson = 0;
//	fontsize = 18; //overwritten anyway
	fontsize = fs;
    playsound = p;
 
	QDir d("sounds"); //do not translate
    if (!d.exists())
	{
		//std::cout << "sounds directory not found" << std::endl;
        //QMessageBox::warning(0,"Sound Warning","sounds directory not found will not play sounds");
	}

	good = 0;
	bad = 0;
	good = new QSound("./sounds/good.wav"); //do not translate these
	bad =  new QSound("./sounds/bad.wav");
	/*
	if(good==0 || bad==0)
	{
	//	std::cout << "warning, some sound file not found" << std::endl;
		//QMessageBox::warning(0,"Warning","some sound file not found, will not play sounds");
	} */
	
  
}
void TextWidget::initialize(std::vector<MyButton*> ve,QString lay) //called each time keyboard layout changes e.g. from us to numeric keypad
{
	//this stuff is to time each keypress, each letter in layout has a number of times
	//it's been pressed and the total time
	int i;
	bool lfound = false;
	for(i=0;i<layoutScores.size();i++)
	{
		if(layoutScores[i]->layoutname == lay.mid(0,lay.length()-4))
		{
			lfound = true; //already in mem
			layoutScoreIndex = i;
			break;
		}
	}
	
	if(!lfound)
	{
		layoutScores.push_back(new layoutScore(lay.mid(0,lay.length()-4)));
		layoutScoreIndex = layoutScores.size()-1;
		QString path;
		if(MULTI)
			path = "users/"+USERNAME+"/"+lay.mid(0,lay.length()-4)+".txt";
		else
			path = "defaultuser/"+lay.mid(0,lay.length()-4)+".txt";

		QFile scorefile(path); 
		if(scorefile.exists())
		{
			if(scorefile.open(IO_ReadOnly))
			{
				lfound = true; //read in from file
				QTextStream scorefilestream(&scorefile);
				scorefilestream.setEncoding(QTextStream::Unicode);
				while(!scorefilestream.atEnd())
				{
					QString l = scorefilestream.readLine();
					if(l==0)
						break;
					QString num = scorefilestream.readLine();
					if(num==0)
						break;
					QString totaltime = scorefilestream.readLine();
					if(totaltime==0)
						break;
					QString missed = scorefilestream.readLine();
					if(missed==0)
						break;
					numandtime* u = new numandtime(l.at(0),num.toULong(),totaltime.toULong(),missed.toULong());
					layoutScores[layoutScoreIndex]->letters->insert(l,u);
				}
				scorefile.close();
			}
		}
	}


	v = ve;

	studentatline = 0;

	studentkeys.clear();

	khighlights->clear();
#ifdef FEST
	valuetosay->clear();
#endif
	lasthighlighted = 0;

	//widgetwidth = width();

	leftshifti = -1;
	rightshifti = -1;
	altgri = -1;
	tabi = -1;
	capslocki = -1;
	enteri = -1;
	backspacei = -1;
	numlocki = -1;
	forwardaccenti = -1;
	doubledoti = -1;
	hati = -1;
	backwardaccenti = -1;
	squigglei = -1;
	cedillai = -1;
	caroni = -1;
	brevei = -1;
	degreesigni = -1;
	ogoneki = -1;
	dotabovei = -1;
	doubleacuteaccenti = -1;

	controli.clear();
	alti.clear();


	int x;
	for(i=0;i<v.size();i++)
	{ //find the indexes of certain modifier keys so that can be highlighted
	  for(x=0;x<v[i]->type.size();x++)
	  {
		if(v[i]->type[x] == MyButton::leftshift)
			leftshifti = i;
		else
		if(v[i]->type[x] == MyButton::rightshift)
			rightshifti = i;
		else
		if(v[i]->type[x] == MyButton::altgr)
			altgri = i;
		else
		if(v[i]->type[x] == MyButton::tab)
			tabi = i;
		else
		if(v[i]->type[x] == MyButton::capslock)
			capslocki = i;
		else
		if(v[i]->type[x] == MyButton::control)
			controli.push_back(i);
		else
		if(v[i]->type[x] == MyButton::alt)
			alti.push_back(i);
		else
		if(v[i]->type[x] == MyButton::enter)
			enteri = i;
		else
		if(v[i]->type[x] == MyButton::backspace)
			backspacei = i;
		else
		if(v[i]->type[x] == MyButton::numlock)
			numlocki = i;
		else
		if(v[i]->type[x] == MyButton::forwardaccent)
			forwardaccenti = i;
		else
		if(v[i]->type[x] == MyButton::doubledot)
			doubledoti = i;
		else
		if(v[i]->type[x] == MyButton::hat)
			hati = i;
		else
		if(v[i]->type[x] == MyButton::backwardaccent)
			backwardaccenti = i;
		else
		if(v[i]->type[x] == MyButton::squiggle)
			squigglei = i;
		else
		if(v[i]->type[x] == MyButton::cedilla)
			cedillai = i;
		else
		if(v[i]->type[x] == MyButton::caron)
			caroni = i;
		else
		if(v[i]->type[x] == MyButton::breve)
			brevei = i;
		else
		if(v[i]->type[x] == MyButton::degreesign)
			degreesigni = i;
		else
		if(v[i]->type[x] == MyButton::ogonek)
			ogoneki = i;
		else
		if(v[i]->type[x] == MyButton::dotabove)
			dotabovei = i;
		else
		if(v[i]->type[x] == MyButton::doubleacuteaccent)
			doubleacuteaccenti = i;
	  }
	}

	enterhighlighted->clear();
	if(enteri != -1)
		enterhighlighted->push_back(HighlightDetails(enteri,SolidPattern));
	
#ifdef FEST
	for(i=0;i<v.size();i++)
	{
		for(int x=0;x<v[i]->values.size();x++)
			valuetosay->insert(v[i]->values[x].val,&(v[i]->values[x].say));
	}
#endif
	for(i=0;i<v.size();i++)
	{
		for(int x=0;x<v[i]->values.size();x++)
		{
			std::vector<HighlightDetails>* tmp = new std::vector<HighlightDetails>;
			BrushStyle currentstyle = SolidPattern;

			for(int y=0;y<v[i]->values[x].when.size();y++)
			{
				switch (v[i]->values[x].when[y])
				{
				case MyButton::normal:
					break;
				case MyButton::leftshift:
					if(leftshifti != -1)
						tmp->push_back(HighlightDetails(leftshifti,currentstyle));
					break;
				case MyButton::rightshift:
					if(rightshifti != -1)
						tmp->push_back(HighlightDetails(rightshifti,currentstyle));
					break;
				case MyButton::altgr:
					if(altgri != -1)
						tmp->push_back(HighlightDetails(altgri,currentstyle));
					break;
				case MyButton::forwardaccent:
					if(forwardaccenti != -1)
						tmp->push_back(HighlightDetails(forwardaccenti,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::doubledot:
					if(doubledoti != -1)
						tmp->push_back(HighlightDetails(doubledoti,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::hat:
					if(hati != -1)
						tmp->push_back(HighlightDetails(hati,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::backwardaccent:
					if(backwardaccenti != -1)
						tmp->push_back(HighlightDetails(backwardaccenti,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::squiggle:
					if(squigglei != -1)
						tmp->push_back(HighlightDetails(squigglei,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::cedilla:
					if(cedillai != -1)
						tmp->push_back(HighlightDetails(cedillai,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::caron:
					if(caroni != -1)
						tmp->push_back(HighlightDetails(caroni,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::breve:
					if(brevei != -1)
						tmp->push_back(HighlightDetails(brevei,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::degreesign:
					if(degreesigni != -1)
						tmp->push_back(HighlightDetails(degreesigni,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::ogonek:
					if(ogoneki != -1)
						tmp->push_back(HighlightDetails(ogoneki,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::dotabove:
					if(dotabovei != -1)
						tmp->push_back(HighlightDetails(dotabovei,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::doubleacuteaccent:
					if(doubleacuteaccenti != -1)
						tmp->push_back(HighlightDetails(doubleacuteaccenti,currentstyle));
					currentstyle = Dense2Pattern;
					break;
				case MyButton::capslock:
					if(capslocki != -1)
						tmp->push_back(HighlightDetails(capslocki,currentstyle));
					break;
				default:
					//cout << "value when is not normal,leftshift,rightshift,altgr,forwardaccent,doubledot,hat,backwardaccent,squiggle" << endl;
					exit(1);
					break;
				}
			}
			tmp->push_back(HighlightDetails(i,currentstyle));
			khighlights->insert(v[i]->values[x].val,tmp);

			if(!lfound)
			{
				numandtime* u = new numandtime(v[i]->values[x].val.at(0));
				numandtime* t = layoutScores[layoutScoreIndex]->letters->find(v[i]->values[x].val);
				if(t==0)
				layoutScores[layoutScoreIndex]->letters->insert(v[i]->values[x].val,u);
			}
		}
	}

	scoredialog->drawon->heading = lay.mid(0,lay.length()-4);
	if(lay==tr("Numeric-Keypad.xml"))
	{
		scoredialog->drawon->setletters(false);
		scoredialog->drawon->letters->setDisabled(true);
	}
	else
	{
		scoredialog->drawon->letters->setDisabled(false);
		bool* u = lettersonlydict->find(lay.mid(0,lay.length()-4));
		if(u!=0 && *u==true)
		{
			scoredialog->drawon->setletters(true);
		}
		else
		{
			scoredialog->drawon->setletters(false);
		}
	}
//	if(fontsize != fs)
//		fontsizeChanged(fs);
	fontsizeChanged(fontsize);

}
//display lesson happens once per lesson
void TextWidget::displayLesson(std::vector<QString>* l,bool ltr,bool num)
{
	numeric = num;
	numericfinished = false;

	totalerrors = 0;
	errinstud = 0;
	lessonvec = l;
	int i;
	wholelessonlength = 0;

	wordssection = false;
	switchwords.clear();

	for(i=0;i<lessonvec->size();i++)
	{
		int ind;
		while((ind = (*lessonvec)[i].find("<words>"))!=-1)
		{
			if(ind == 0)
				wordssection = true;
			else
				switchwords.push_back(wholelessonlength + ind);

			(*lessonvec)[i].remove(ind,7);
		}
		wholelessonlength += (*lessonvec)[i].length();
	}

	progress->setTotalSteps(wholelessonlength);
	progress->setProgress(0);

	for(i=0;i<v.size();i++) //quite an unelegant thing but safe
		v[i]->setDown(false);

	withinlesson = true;
	newlesson = true;
	rtl = !ltr;

	studentkeys.clear();

#ifdef FEST
	level = oneword;
  if(!showhelpb)
  {
	QString word;
	for(i=0;i<30;i++)
	{
		if(QString((*lessonvec)[0][i])==" ")
		{
			word = (*lessonvec)[0].mid(0,i);
			break;
		}
	}
	if(word.length() > 0)
	{
		if(wordssection)
		{				
			saysomething(new QString(word));
		}
		else
		{
				QString* tmp = new QString();
				for(i=0;i<word.length();i++)
					*tmp += word[i]+",";
				saysomething(tmp);
		}
	}
  }
  else
	  showhelpb = false;
#endif


	lessonveci = 0;
	displayLessonLine((*lessonvec)[0]);

	
}
void TextWidget::displayLessonLine(QString s) 
{
	enterexpected = false;

	lesson = s;
	if(rtl)
	{
		blackstartindex = lesson.length()-1;
		blackendindex = blackstartindex;
		studentatindex = blackstartindex;
	//	if(lesson.length()>0) //remove space at front, so don't end in a space
	//		if(QString(lesson.at(0))==" ")
	//			lesson = lesson.mid(1);
	}
	else
	{
		blackendindex = 0;
		blackstartindex = 0;
		studentatindex = 0;
	//	if(lesson.length()>0)
	//	if(QString(lesson.at(lesson.length()-1))==" ") //remove space at end
	//		lesson = lesson.mid(0,lesson.length()-1);
	}

	studentatline = 0;

	int i;
	for(i=0;i<studentkeys.size();i++)
		if(studentkeys[i].error)
			errinstud++;

	studentkeys.clear();
	
	
	if(lasthighlighted != 0)
	for(i=0;i<lasthighlighted->size();i++)
	{
			v[(*lasthighlighted)[i].index]->unHighlight();
			v[(*lasthighlighted)[i].index]->repaint();
	}

	if(lesson.length()>0)
	{
		if(rtl)
			lasthighlighted = khighlights->find(QString(lesson.at(lesson.length()-1)));
		else
			lasthighlighted = khighlights->find(QString(lesson[0]));
		if(lasthighlighted != 0)
		for(i=0;i<lasthighlighted->size();i++)
		{
		//	cout << (*lasthighlighted)[i] <<endl;
			v[(*lasthighlighted)[i].index]->highlight((*lasthighlighted)[i].style);
			v[(*lasthighlighted)[i].index]->repaint();
		}
	}
	calcSubstrings();
	repaint();
	
}
void TextWidget::calcWidthAndHeight(int fs,int& w,int& h)
{
	w = 0;
	QFont f("Arial",fs); //do not translate
	QFontMetrics metric(f);
	h = metric.height();
	for(int i=0;i<v.size();i++)
		for(int x=0;x<v[i]->values.size();x++)
			if(metric.width(v[i]->values[x].val) > w)
				w = metric.width(v[i]->values[x].val);
}
void TextWidget::fontsizeChanged(int fs)
{
//	int i;
	//cout << "fontsizechanged"<<endl;
	if(fs > fontsize)
	{
		decreasefontwidget->setDisabled(false);
		int w;
		int h;
		calcWidthAndHeight(fs+3,w,h);
		if(widgetwidth/w < 5)
			increasefontwidget->setDisabled(true);
	}
	else
	{
		increasefontwidget->setDisabled(false);
		if(fs-3<=0)
			decreasefontwidget->setDisabled(true);
		else
		{
			QFont f("Arial",fs-3); //do not translate
			QFontMetrics metric(f);
			if(metric.height() < 5)
				decreasefontwidget->setDisabled(true);
		}
	}

	fontsize = fs;
	calcWidthAndHeight(fontsize,charwidth,charheight);

	//cout << "inside fontsizeChanged, widgetwidth"<<widgetwidth << endl;
	
	while(fontsize>0) //just to handle bad stuff in the settings file
	{
		if(widgetwidth/charwidth < 5)
		{
			fontsize -= 3;
			calcWidthAndHeight(fontsize,charwidth,charheight);
		}
		else break;
	}
	/*
	charwidth = 0; 
	QFont f("Arial",fontsize);
	QFontMetrics metric(f);
	charheight = metric.height();
	
	for(i=0;i<v.size();i++)
	{
		for(int x=0;x<v[i]->values.size();x++)
			if(metric.width(v[i]->values[x].val) > charwidth)
				charwidth = metric.width(v[i]->values[x].val);
	}
	//charwidth = metric.width(widestchar);*/


	widgetheight = charheight*2 + 2;
	setMinimumHeight(widgetheight);
	setMaximumHeight(widgetheight);
	//this forces a resize event which in turn calls calcSubstrings()

}
TextWidget::~TextWidget()
//if you want to destruct this and then create a new one, you should release
//readblock and writeblock at the end of this method and have some global bool so
//that a new saying thread doesn't start after you have released readblock and
//writeblock
{
#ifdef FEST
#ifdef WIN
//	if(!festerror)
//	{

		WaitForSingleObject(freadblock,25000);
		WaitForSingleObject(fwriteblock,25000);//got 25 seconds to finish the sentence

//		fliteCleanup();

		CloseHandle(freadblock);
		CloseHandle(fwriteblock);
		CloseHandle(fmutex1);
		CloseHandle(fmutex2);

//		FreeLibrary(hTME);
//	}
#else
	pthread_mutex_lock(&freadblock);
	pthread_mutex_lock(&fwriteblock);
	//flite_clean_up
	pthread_mutex_destroy(&freadblock);
	pthread_mutex_destroy(&fwriteblock);
	pthread_mutex_destroy(&fmutex1);
	pthread_mutex_destroy(&fmutex2);
#endif
	valuetosay->clear();
	delete valuetosay;
#endif
	khighlights->clear();
	delete khighlights;
	if(good!=0)
		delete good;
	if(bad!=0)
		delete bad;

	enterhighlighted->clear();
	delete enterhighlighted;
	if(redo!=0)
		delete redo;
	if(next!=0)
		delete next;
	if(mydialog!=0)
		delete mydialog;
	delete timer;
	delete keypresstimer;

	delete scoredialog;

	
}
void TextWidget::calcSubstrings()
{
	//cout << "calcsubstrings"<<endl;
	//if(charwidth == 0)
	//	fontsizeChanged(fontsize);

	int chars = widgetwidth/charwidth;
	if(chars<2)
	{
		//cout << "too few chars" << endl;
		exit(1);
	}
	if(chars>20)
		greys = 5;
	else
		greys = 3;
	if(chars<15)
		greys = 2;
	if(chars<10)
		greys = 1;
	blacklength = chars-greys;

	if(rtl)
	{
	  blackendindex = blackstartindex-blacklength+1;
	  if(studentatindex < blackendindex)
	  {

		blackendindex = studentatindex;
		blackstartindex = blackendindex+blacklength-1;

		studentatline = studentkeys.size() - (lesson.length()-1-blackstartindex);
	  }
	  if(blackendindex < 0)
		  blacksubstring = lesson.mid(0,blackstartindex+1);
	  else
		blacksubstring = lesson.mid(blackendindex,blacklength);
	  if(blackendindex-greys < 0 && blackendindex>0)
		  greysubstring = lesson.mid(0,blackendindex);
	  else
		greysubstring = lesson.mid(blackendindex-greys,greys);
	}
	else
	{
	  blackendindex = blackstartindex+blacklength-1;
	  if(blackendindex < studentatindex)
	  {

		blackendindex = studentatindex;
		blackstartindex = blackendindex-blacklength+1;
	
		studentatline = studentkeys.size()-blackstartindex;
	  }
	  blacksubstring = lesson.mid(blackstartindex,blacklength);
	  greysubstring = lesson.mid(blackstartindex+blacklength,greys);
	}

}

void TextWidget::backspace()
{
	//cout << "got a backspace" << endl;

	if((!rtl && studentatindex > 0) || (rtl && studentatindex < lesson.length()-1))
	{
		progress->setProgress(progress->progress()-1);
		enterexpected = false;
		studentkeys.pop_back();

		if(studentatindex == blackstartindex) //so we have to go to previous line
		{
			if(rtl)
			{
				studentatindex++;
				if(lesson.length()-1-blackstartindex >= blacklength)
				{
					blackendindex = blackstartindex + 1;
					blackstartindex = blackendindex + blacklength - 1;
					//if(blackendindex < 0)
					//	blacksubstring = lesson.mid(0,blackstartindex+1);
					//else
					blacksubstring = lesson.mid(blackendindex,blacklength);
					if(blackendindex-greys < 0 && blackendindex>0)
						greysubstring = lesson.mid(0,blackendindex);
			    	else
						greysubstring = lesson.mid(blackendindex-greys,greys);

					studentatline = blackstartindex - blackendindex;
				}
				else
				{
					blackstartindex = lesson.length()-1;
					blackendindex = lesson.length() - blacklength;
					if(blackendindex < 0)
						blacksubstring = lesson.mid(0,blacklength);
					else
						blacksubstring = lesson.mid(blackendindex,blacklength);

					if(blackendindex-greys < 0 && blackendindex>0)
						greysubstring = lesson.mid(0,blackendindex);
			    	else
						greysubstring = lesson.mid(blackendindex-greys,greys);

					studentatline = lesson.length()-1-studentatindex;
				}
			}
			else //show previous line and we're left to right
			{
				studentatindex--;
				if(blackstartindex >= blacklength) 
				{
					blackendindex = blackstartindex - 1;
					blackstartindex = blackendindex - blacklength + 1;
					blacksubstring = lesson.mid(blackstartindex,blacklength);
					greysubstring = lesson.mid(blackstartindex+blacklength,greys);	
	
					studentatline = blackendindex - blackstartindex;
				}
				else
				{
					blackstartindex = 0;
					blackendindex = 0 + blacklength - 1;
					blacksubstring = lesson.mid(0,blacklength);
					greysubstring = lesson.mid(blackstartindex+blacklength,greys);
	
					studentatline = studentatindex;
				}
			}
			repaint();
		}
		else //stay on same line
		{
			if(rtl)
			{
				studentatindex++;
				studentatline--;
				this->erase(widgetwidth-(studentatline+1)*charwidth,charheight+2,charwidth,charheight);
			}
			else
			{
				studentatindex--;
				studentatline--;
				this->erase(studentatline*charwidth,charheight+2,charwidth,charheight);
			}

			QPainter p;
			p.begin(this);
			p.setPen("white");
			if(rtl)
				p.drawLine(widgetwidth-(studentatline+1)*charwidth,2*charheight+1,widgetwidth-(studentatline+2)*charwidth,2*charheight+1);
			else
				p.drawLine((studentatline+1)*charwidth,2*charheight+1,(studentatline+2)*charwidth,2*charheight+1);
			p.setPen("black");
			if(rtl)
				p.drawLine(widgetwidth-(studentatline+1)*charwidth,2*charheight+1,widgetwidth-studentatline*charwidth,2*charheight+1);
			else
				p.drawLine(studentatline*charwidth,2*charheight+1,(studentatline+1)*charwidth,2*charheight+1);
			p.end();
		}

		int i;
		if(lasthighlighted != 0)
		for(i=0;i<lasthighlighted->size();i++) 
		{
				v[(*lasthighlighted)[i].index]->unHighlight();
				v[(*lasthighlighted)[i].index]->repaint();
		}

		if((!rtl && studentatindex < lesson.length())||(rtl&&studentatindex>=0)) //should always be
			lasthighlighted = khighlights->find(QString(lesson[studentatindex]));
		if(lasthighlighted != 0)
		for(i=0;i<lasthighlighted->size();i++)
		{
			v[(*lasthighlighted)[i].index]->highlight((*lasthighlighted)[i].style);
			v[(*lasthighlighted)[i].index]->repaint();
		}

#ifdef FEST
		//delay->start(2000,true);
		if(delay->isActive())
			delay->changeInterval(2000);
		else
			delay->start(2000,true);
#endif

	}
}
void TextWidget::paintEvent(QPaintEvent* q)
{
	//cout << "got a paint event" << endl;

	QPixmap* pixmap = new QPixmap(widgetwidth,widgetheight);
//	pixmap.fill();
	pixmap->fill(this,0,0);
	QPainter p(pixmap); //,this
	p.setPen("black");
	if(rtl)
		p.drawLine(widgetwidth-studentatline*charwidth,2*charheight+1,widgetwidth-(studentatline+1)*charwidth,2*charheight+1);
	else
		p.drawLine(studentatline*charwidth,2*charheight+1,(studentatline+1)*charwidth,2*charheight+1);
	QFont f("Arial",fontsize); //do not translate
	p.setFont(f);
	int i;
	

	for(i=0;i<blacksubstring.length();i++)
		if(rtl)
			p.drawText(QRect(widgetwidth-(i+1)*charwidth,0,charwidth,charheight),AlignHCenter|AlignBottom,QString(blacksubstring.at(blacksubstring.length()-1-i)));
		else
			p.drawText(QRect(i*charwidth,0,charwidth,charheight),AlignHCenter|AlignBottom,QString(blacksubstring.at(i)));

	p.setPen("grey");
	for(int x=0;x<greysubstring.length();x++)
	{
		if(rtl)
			p.drawText(QRect(widgetwidth-(i+1)*charwidth,0,charwidth,charheight),AlignHCenter|AlignBottom,QString(greysubstring.at(greysubstring.length()-1-x)));
		else
			p.drawText(QRect(i*charwidth,0,charwidth,charheight),AlignHCenter|AlignBottom,QString(greysubstring.at(x)));
		i++;
	}
	int count = 0;
	int start;
	if(rtl)
		start = lesson.length()-1-blackstartindex; 
	else
		start = blackstartindex;
	for(i=start;i<studentkeys.size();i++)
	{
		if(studentkeys[i].error)
			p.setPen("red");
		else
			p.setPen("black");

		if(studentkeys[i].line)
		{
			if(rtl)
				p.drawLine(widgetwidth-(count+1)*charwidth,2*charheight+1,widgetwidth-count*charwidth,2*charheight+1);
			else
				p.drawLine(count*charwidth,2*charheight+1,(count+1)*charwidth,2*charheight+1);
		}
		else
		{
			if(rtl)
				p.drawText(QRect(widgetwidth-(count+1)*charwidth,charheight+2,charwidth,charheight),AlignHCenter|AlignBottom,studentkeys[i].key);
			else
				p.drawText(QRect(count*charwidth,charheight+2,charwidth,charheight),AlignHCenter|AlignBottom,studentkeys[i].key);
		}

		count++;
	}
	p.end();
//	p.begin(this);
   // p.drawPixmap(0,0,pixmap);
	//p.end();
	bitBlt( this, QPoint(0,0), pixmap );
	delete pixmap;
}
/*
int TextWidget::heightHint()
{
	return widgetheight;
}*/

void TextWidget::resizeEvent(QResizeEvent* r)
{
	//cout << "got a resize event, width "<<width()<<" height "<<height() << endl;

	widgetheight = height();
	if(width() < widgetwidth)
	{
		widgetwidth = width();

		if(widgetwidth/charwidth < 5 )
		{
			while(true)
			{
				fontsize -= 5;
				calcWidthAndHeight(fontsize,charwidth,charheight);
				if(widgetwidth/charwidth >= 5)
				{
					widgetheight = charheight*2 + 2;
					increasefontwidget->setDisabled(true);
					setMinimumHeight(widgetheight);
					setMaximumHeight(widgetheight);
					//this now causes another resizeEvent which will then call calcSubstrings
					break;
				}
			}
		}
		else
		{
			if(increasefontwidget->isEnabled())
			{
				int w;
				int h;
				calcWidthAndHeight(fontsize+5,w,h);
				if(w!=0)
				if(widgetwidth/w < 5)
					increasefontwidget->setDisabled(true);
			}
			calcSubstrings();
		}
	}
	else
	{
		widgetwidth = width();
		if(!increasefontwidget->isEnabled())
		{
			int w;
			int h;
			calcWidthAndHeight(fontsize+5,w,h);
			if(widgetwidth/w >= 5)
				increasefontwidget->setEnabled(true);
		}
		calcSubstrings();
	}
}
void TextWidget::fromKeyboard(QString k,int key) //k will always only be a single letter
{
	
/*	if(rtl)
	{
		if(studentatindex <= -1)
			return;
	}
	else
	if(studentatindex >= lesson.length())
	{
		return;
	}*/

	progress->setProgress(progress->progress()+1);

	if(!newlesson)  //ignore length of time it takes to press first key
	{
		int mi = keypresstimer->elapsed();
		if((k.at(0) == lesson.at(studentatindex)))
		{
			if(mi>0)
			{
				if(mi>3000)
					mi=3000;
				numandtime* tmp = layoutScores[layoutScoreIndex]->letters->find(k);
				if(tmp!=0)
				{
					if(tmp->totaltime>(ulong)4294960000) //highly unlikely but time mustn't wrap around
					{
						tmp->totaltime /= 2;
						tmp->num /=2;
						tmp->missed /=2;
					}
					tmp->totaltime += mi;
					tmp->num++;
				}
			}
		}
		else
		{
			numandtime* tmp = layoutScores[layoutScoreIndex]->letters->find(QString(lesson.at(studentatindex)));
			if(tmp!=0)
			{
				if(tmp->missed>(ulong)4294960000)
				{
					tmp->totaltime /= 2;
					tmp->num /=2;
					tmp->missed /=2;
				}
				tmp->missed++;
			}
		}
	}
	keypresstimer->restart();

	if(newlesson)
	{
		timer->restart();
		newlesson = false;
	}
	bool error = false;
	bool line = false;

	if(k.at(0) != lesson.at(studentatindex))
	{
		totalerrors++;
		if(playsound)
			bad->play();
		error = true;
		if(key==Key_Space) //put a space where there should be a letter,
		//this enables a red line to be drawn
			line = true;
	}
	else
		if(playsound)
			good->play();

	if(rtl)
		studentatindex--;
	else
		studentatindex++;

	studentkeys.push_back(keypressed(k,error,line));

	int i;

	if(lasthighlighted != 0)
	for(i=0;i<lasthighlighted->size();i++)
	{
			v[(*lasthighlighted)[i].index]->unHighlight();
			v[(*lasthighlighted)[i].index]->repaint();
	}
	if(rtl)
	{
		if(studentatindex >= 0)
			lasthighlighted = khighlights->find(QString(lesson[studentatindex]));
	}
	else
	if(studentatindex < lesson.length())
		lasthighlighted = khighlights->find(QString(lesson[studentatindex]));

	if(lasthighlighted != 0)
	for(i=0;i<lasthighlighted->size();i++)
	{
			v[(*lasthighlighted)[i].index]->highlight((*lasthighlighted)[i].style);
			v[(*lasthighlighted)[i].index]->repaint();
	}


	if((!rtl && studentatindex > blackendindex)||(rtl && studentatindex < blackendindex)) //so paint next line
	{
		studentatline = 0;

		if(rtl)
		{
			blackstartindex = blackendindex - 1;
			blackendindex = blackstartindex-blacklength+1;
			if(blackendindex < 0)
				blacksubstring = lesson.mid(0,blackstartindex+1);
			else
				blacksubstring = lesson.mid(blackendindex,blacklength);
			if(blackendindex-greys < 0 && blackendindex>0)
				greysubstring = lesson.mid(0,blackendindex);
			else
				greysubstring = lesson.mid(blackendindex-greys,greys);
		}
		else
		{
			blackstartindex = blackendindex+1;
			blackendindex = blackstartindex+blacklength-1;
			blacksubstring = lesson.mid(blackstartindex,blacklength);			
			greysubstring = lesson.mid(blackstartindex+blacklength,greys);
		}

		repaint();
	}
	else
	{
		QPainter p;
		p.begin(this);
	
		QFont f("Arial",fontsize); //do not translate
		p.setFont(f);
		if(line)
		{
			p.setPen("red"); //as a side effect, paints over black cursor
			if(rtl)
				p.drawLine(widgetwidth-(studentatline+1)*charwidth,2*charheight+1,widgetwidth - studentatline*charwidth,2*charheight+1);
			else
				p.drawLine(studentatline*charwidth,2*charheight+1,(studentatline+1)*charwidth,2*charheight+1);
		}
		else
		{
			p.setPen("white");
			if(rtl)
				p.drawLine(widgetwidth-(studentatline+1)*charwidth,2*charheight+1,widgetwidth - studentatline*charwidth,2*charheight+1);
			else
				p.drawLine(studentatline*charwidth,2*charheight+1,(studentatline+1)*charwidth,2*charheight+1);
			if(error)
				p.setPen("red");
			else
				p.setPen("black");
			if(rtl)
				p.drawText(QRect(widgetwidth-(studentatline+1)*charwidth,charheight+2,charwidth,charheight),AlignHCenter|AlignBottom,k);
			else
				p.drawText(QRect(studentatline*charwidth,charheight+2,charwidth,charheight),AlignHCenter|AlignBottom,k);
		}

		studentatline++;

		p.setPen("black"); //now draw the cursor
		if(rtl)
			p.drawLine(widgetwidth-(studentatline+1)*charwidth,2*charheight+1,widgetwidth - studentatline*charwidth,2*charheight+1);
		else
			p.drawLine(studentatline*charwidth,2*charheight+1,(studentatline+1)*charwidth,2*charheight+1);
		p.end();
	}

#ifdef FEST

	if(switchwords.size()>0 && studentatindex==switchwords[0])
	{
				switchwords.erase(switchwords.begin());
				wordssection = !wordssection;
	}

	if(studentatindex>0 && QString(lesson.at(studentatindex-1))==" ")
	{
		
			if(wordssection)
			{
				if(level < threewords || (level >= threewords && studentatindex > saiduntil))
				{
					if(level < oneword)
						level = oneword;
					else
					{
						float  strokespermin = (float)(60*studentatindex)*(1000.0/(float)timer->elapsed());
						if(strokespermin > 120)
							level = sentence;
						else
						{
							if(strokespermin > 90) //rather arb numbers
								level = threewords;
							else
								level = oneword;
						}
					}
				
					saysomething(new QString(getnextwords()));
				}
			}
			else
			{
				level = oneword;

				QString* tmp = new QString();
				//QString orig(lesson.mid(studentatindex,ind-studentatindex+1));
				QString orig = getnextwords();
				for(int i=0;i<orig.length();i++)
					*tmp += orig[i]+",";
				saysomething(tmp);
			}
	}
//	else
//		delay->start(2000,true);
		if(delay->isActive())
			delay->changeInterval(2000);
		else
			delay->start(2000,true);
#endif


}
	
#ifdef FEST

QString TextWidget::getnextwords() //based on level
{
	int i;
  if(level==oneword)
  {
	for(i=studentatindex;i<studentatindex+30;i++)
		if(QString(lesson[i])==" ")
		{
			saiduntil = i;
			return QString(lesson.mid(studentatindex,i-studentatindex));
		}
	saiduntil = lesson.length()-1;
	return lesson.mid(studentatindex); //the rest of the string
  }
  else
  {
	  if(level==sentence)
	  {
		  for(i=studentatindex;i<studentatindex+100;i++)
			  if(QString(lesson[i])=="."||QString(lesson[i])=="?"||QString(lesson[i])=="!")
			  {
				  saiduntil = i;
				  return lesson.mid(studentatindex,i-studentatindex);
			  }
		  level=threewords; //if it is such a long sentence, just do three at a time
	  }

	  if(level==threewords)
	  {
		  int studentat = studentatindex;
		  QString threesome = "";
		  bool found;

		  for(int x=0;x<3;x++)
		  {
			    found = false;
				for(i=studentat;i<studentat+30;i++)
				{
					if(QString(lesson[i])==" ")
					{
						threesome += lesson.mid(studentat,i-studentat)+" ";
						studentat = i+1;
						found = true;
						saiduntil = i;
						break;
					}
					else
						if(QString(lesson[i])=="." || QString(lesson[i])=="?" || QString(lesson[i])=="!")
						{
							saiduntil = i;
							return lesson.mid(studentatindex,i-studentatindex);
						}
				}
				if(!found)
				{
					saiduntil = lesson.length()-1;
					return lesson.mid(studentatindex);
					//break;
				}
		  }
		  return threesome;
	  }
	  else
	  {
		  saiduntil = studentatindex;
		  return QString("");
	  }
  }
 
}
#endif


#ifdef FEST

void TextWidget::saysomething(QString* tosay)
{
//	if(!festerror)
//	{
#ifdef WIN
		DWORD targetThreadID;
		CloseHandle((HANDLE)_beginthreadex(NULL,0,(unsigned(_stdcall*)(void*))threadSay,(void*)tosay,0,(unsigned*)&targetThreadID));
		//CloseHandle(&targetThreadID);
#else
		pthread_t mythread;
		pthread_create(&mythread,0,threadSay,(void*)tosay);
		pthread_detach(mythread);
#endif
//	}
}

#endif

void TextWidget::keyPressEvent(QKeyEvent* k)
{
	//	cout << "got a key press event " <<endl;
	//	cout << k->key() << endl;
/*		if(k->key()==Key_Escape)
		{
		}
		else*/
		if(k->key()==Key_Backspace)
		{
		  if(allowbackspace)
		  {
			if(backspacei != -1 && !v[backspacei]->isDown())
				v[backspacei]->setDown(true);
			for(int x=0;x<k->count();x++)
				backspace();
			keypresstimer->restart();
		  }
		}
		else
		if(k->key() == Key_Shift)
		{
			if(leftshifti != -1)
				v[leftshifti]->setDown(true);
			if(rightshifti != -1)
				v[rightshifti]->setDown(true);
		}
	/*	else
		if(k->state() == 48) //control and alt or'd together
		{
			if(altgri != -1)
				v[altgri]->setDown(true);
		} */
		else
		if(k->key() == Key_Control)
		{
			for(int i=0;i<controli.size();i++)
				v[controli[i]]->setDown(true);
		}
		else
		if(k->key() == Key_Enter || k->key() == Key_Return)
		{
			if(enteri != -1)
				v[enteri]->setDown(true);

			if(enterexpected)
			{
				enterexpected = false;
				if(playsound)
					good->play();
				if(lessonveci == lessonvec->size()-1)
					numericfinished = true;
				else
				{
					lessonveci++;
					displayLessonLine((*lessonvec)[lessonveci]);
				//	repaint();
					return;
				}
			}
		}
		else
		if(k->key() == Key_Alt)
		{
			for(int i=0;i<alti.size();i++)
				v[alti[i]]->setDown(true);
		}
		else
		if(k->key() == Key_CapsLock)
		{
			if(capslocki != -1)
				v[capslocki]->setDown(true);
		}
		else
		if(k->key() == Key_Tab)
		{
			if(tabi != -1)
				v[tabi]->setDown(true);
		}
		else
		if(k->key() == Key_NumLock)
		{
			if(numlocki != -1)
				v[numlocki]->setDown(true);
		}
//#ifdef FEST
		else
		if(k->key() == Key_F1)
		{
			if(mymainwindow!=0 && mymainwindow->keyboardwidget != 0)
				mymainwindow->keyboardwidget->previous();
		}
		else
		if(k->key() == Key_F2)
		{
			if(mymainwindow!=0 && mymainwindow->keyboardwidget != 0)
				mymainwindow->keyboardwidget->next();
		}
//#endif
		else
		{
			if(enterexpected)
			{
				enterexpected = false;
				if(playsound)
					bad->play();
				if(lessonveci == lessonvec->size()-1)
					numericfinished = true;
				else
				{
					lessonveci++;
					displayLessonLine((*lessonvec)[lessonveci]);
				//	repaint();
					return;
				}
			}
			else
			for(int i=0;i<k->text().length();i++)
			{
				QString letter = QString(k->text().at(i));
				std::vector<HighlightDetails>* tmp = khighlights->find(letter);
				if(tmp != 0 && tmp->size()>0)
					v[(*tmp)[tmp->size()-1].index]->setDown(true);
				//the keys index is put on last, look in initialize()
				fromKeyboard(letter,k->key());
			}
		}

	if(((!rtl && studentatindex==lesson.length()) || (rtl && studentatindex==-1))&&withinlesson)
	{
		if((lessonveci == lessonvec->size()-1 && !numeric) || (numeric && numericfinished))
		{

    		float elapsed = (float)timer->elapsed();
    
    		progress->setProgress(wholelessonlength+1); //just to make sure 100 percent
    
    		withinlesson = false;
    		QString stats;
    		float accuracy;
    		float trueaccuracy;
    		float strokespermin;
    		float wordspermin;
    
    
    	//	int errors = 0;
    		int spaces = 0;
    		int i;
    		for(i=0;i<studentkeys.size();i++)
    				if(studentkeys[i].error)
    					errinstud++;
    					//errors++;
    		int x;
    		for(x=0;x<lessonvec->size();x++)
    		{
    			for(i=0;i<(*lessonvec)[x].length();i++)
    				if(QString((*lessonvec)[x][i])==" ")
    					spaces++;
    			spaces++; //for each lessonline
    		}
    		//spaces++;
    		if(totalerrors>=wholelessonlength)
    			trueaccuracy = 0.0;
    		else
    			trueaccuracy = 100 - 100.0*(float)totalerrors/(float)wholelessonlength;
    
    	//	accuracy = 100 - 100.0*(float)errors/(float)lesson.length();
    
    		accuracy = 100 - 100.0*(float)errinstud/(float)wholelessonlength;
    		strokespermin = (60*wholelessonlength)*(1000.0/elapsed);
    		wordspermin = (60*spaces)*(1000.0/elapsed);
#ifdef SPANISH
			QString fixed = "Precisión:\n"
    					//	"True Accuracy:\n"
    						"Pulsaciones por minuto:\n"
    					//	"Words per minute (for this lesson):";
							"Palabras por minuto:\n";
			stats.sprintf("%2.2f por ciento\n"
    					  "%2.2f\n"
    					  "%2.2f\n",
    					  accuracy,strokespermin,wordspermin);
#else
    		QString fixed = "Accuracy (allows backspace):\n"
    						"True Accuracy:\n"
    						"Keystrokes per minute:\n"
    						"Words per minute (for this lesson):";
    		stats.sprintf("%2.2f percent\n"
    					  "%2.2f percent\n"
    					  "%2.2f\n"
    					  "%2.2f",
    					  accuracy,trueaccuracy,strokespermin,wordspermin);
#endif
    		mydialog->text1->setText(fixed);
    		mydialog->text2->setText(stats);
#ifdef FEST

			resultfocus = true;
			QString* tosay = new QString();
			tosay->sprintf("Lesson Results: %d percent accuracy. %d keystrokes per minute. %d words per minute. Press F1 to redo the lesson. Press F2 to do the next lesson.",(int)accuracy,(int)strokespermin,(int)wordspermin);
			saysomething(tosay);

#endif
    		mydialog->show();

		}
		else
		{
			if(numeric)
			{
				enterexpected=true;
				int i;
				if(lasthighlighted != 0)
				{
					for(i=0;i<lasthighlighted->size();i++)
					{
						v[(*lasthighlighted)[i].index]->unHighlight();
						v[(*lasthighlighted)[i].index]->repaint();
					}
					//lasthighlighted = 0;
				}

				lasthighlighted = enterhighlighted;
				//if(lasthighlighted != 0)
					for(i=0;i<lasthighlighted->size();i++) //will only ever loop 1 or 0 times
					{
						v[(*lasthighlighted)[i].index]->highlight((*lasthighlighted)[i].style);
						v[(*lasthighlighted)[i].index]->repaint();
					}
				
			
			}
			else
			{
				lessonveci++;
				displayLessonLine((*lessonvec)[lessonveci]);
				//repaint();
			}
		}
	}

}

void TextWidget::keyReleaseEvent(QKeyEvent* k)
{
		if(k->key()==Key_Escape)
		{
		}
		else
		if(k->key()==Key_Backspace)
		{
			if(backspacei != -1)
				v[backspacei]->setDown(false);
		}
		else
		if(k->key() == Key_Shift)
		{
			if(leftshifti != -1)
				v[leftshifti]->setDown(false);
			if(rightshifti != -1)
				v[rightshifti]->setDown(false);
		}
		else
		if(k->key() == Key_Control)
		{
			for(int i=0;i<controli.size();i++)
				v[controli[i]]->setDown(false);
		}
		else
		if(k->key() == Key_Enter || k->key() == Key_Return)
		{
			if(enteri != -1)
				v[enteri]->setDown(false);
		}
		else
		if(k->key() == Key_Alt)
		{
			for(int i=0;i<alti.size();i++)
				v[alti[i]]->setDown(false);
		}
		else
		if(k->key() == Key_CapsLock)
		{
			if(capslocki != -1)
				v[capslocki]->setDown(false);
		}
		else
		if(k->key() == Key_Tab)
		{
			if(tabi != -1)
				v[tabi]->setDown(false);
		}
		else
		if(k->key() == Key_NumLock)
		{
			if(numlocki != -1)
				v[numlocki]->setDown(false);
		}
		else
		{
			//cout << "got a normal key release "<<k->text().latin1() <<endl;
		//	cout << "k->text().length() "<<k->text().length()<<endl;
		//	cout << "k->text().latin1() " << k->text().latin1() << endl;
		//	cout << "k->text().at(0) "<<k->text().at(0).latin1() << endl;
			for(int i=0;i<k->text().length();i++)
			{
				std::vector<HighlightDetails>* tmp = khighlights->find(QString(k->text().at(i)));
				if(tmp != 0 && tmp->size()>0)
					v[(*tmp)[tmp->size()-1].index]->setDown(false);
			}
		}

	
}
void TextWidget::clearHistory()
{


	if( QMessageBox::information( scoredialog, "TypeFaster",
                                      "Are you sure you want to delete all statistics for this layout?",
                                      "&Yes", "&No", QString::null,
                                      0,      
                                      1 ) == 0)
	{

		QDictIterator<numandtime> it(*(layoutScores[layoutScoreIndex]->letters)); // iterator for dict
		while ( it.current() != 0 ) 
		{
			it.current()->reset();
			++it;
		}
		scoredialog->dodone(4);
	}
    
}
void TextWidget::doScoreCalc()
{
	QDictIterator<numandtime> it(*(layoutScores[layoutScoreIndex]->letters)); // iterator for dict
	passon.clear();
	while ( it.current() != 0 ) 
	{
		passon.push_back(it.current());
		++it;
	}
	scoredialog->drawon->doCalc(passon);
}
void TextWidget::showScoreDialog()
{
	
	doScoreCalc();
	
//	scoredialog->heading = layoutScores[layoutScoreIndex]->layoutname.mid(0,layoutScores[layoutScoreIndex]->layoutname.length()-4)
	int index = 0;
	int i;
	switch(scoredialog->exec())
	{
		
	case 0: //ok
		keypresstimer->restart();
		break;
	case 1: //practise slowest 5
		for(i=0;i< mymainwindow->keyboardwidget->lessoncombo->count();i++)
		{
			if(mymainwindow->keyboardwidget->lessoncombo->text(i)==tr("Slowest"))
			{
				index = i;
				break;
			}
		}
		mymainwindow->keyboardwidget->lessonActivated(index);
		break;
	case 2: //practise least acc 5
		for(i=0;i< mymainwindow->keyboardwidget->lessoncombo->count();i++)
		{
			if(mymainwindow->keyboardwidget->lessoncombo->text(i)==tr("Least accurate"))
			{
				index = i;
				break;
			}
		}
		mymainwindow->keyboardwidget->lessonActivated(index);
		break;
	case 3: //practise custom
		for(i=0;i< mymainwindow->keyboardwidget->lessoncombo->count();i++)
		{
			if(mymainwindow->keyboardwidget->lessoncombo->text(i)==tr("Custom"))
			{
				index = i;
				break;
			}
		}
		mymainwindow->keyboardwidget->lessonActivated(index);
		break;
	case 4: //clear history (see slot clearHistory)
		keypresstimer->restart();
		break;
	}
}
void TextWidget::increaseFontSize()
{
	fontsizeChanged(fontsize+3);
}
void TextWidget::decreaseFontSize()
{
	fontsizeChanged(fontsize-3);
}

void TextWidget::timeEvent()
{
#ifdef FEST

  static int studentatlag = 0;

  if(studentatindex < lesson.length()) //this sort of stuff is ltr only
  {
	if(level > oneword  && studentatindex>0 && QString(lesson[studentatindex-1])==" ")
	{
		level = oneword;
		saysomething(new QString(getnextwords()));
	}
	else
	{
    
    	QString* tmp;
    
    	if(QString(lesson[studentatindex])==" ")
    		tmp = new QString("space");
    	else
    	if(QString(lesson[studentatindex])==".")
    		tmp = new QString("full stop");
    	else
    	if(QString(lesson[studentatindex])==",")
    		tmp = new QString("comma");
    	else
    	if(QString(lesson[studentatindex])==";")
    		tmp = new QString("semi colon");
    	else
    	if(QString(lesson[studentatindex])==":")
    		tmp = new QString("colon");
    	else
    	if(QString(lesson[studentatindex])=="\"")
    		tmp = new QString("quotation mark");
    	else
    	if(QString(lesson[studentatindex])=="?")
    		tmp = new QString("question mark");
    	else
    	if(QString(lesson[studentatindex])=="!")
    		tmp = new QString("exclamation mark");
		else
		if(QString(lesson[studentatindex])=="(")
			tmp = new QString("left round bracket");
		else
		if(QString(lesson[studentatindex])==")")
			tmp = new QString("right round bracket");
		else
		if(QString(lesson[studentatindex])=="[")
			tmp = new QString("left square bracket");
		else
		if(QString(lesson[studentatindex])=="]")
			tmp = new QString("right square bracket");
		else
		if(QString(lesson[studentatindex])=="{")
			tmp = new QString("left curly bracket");
		else
		if(QString(lesson[studentatindex])=="}")
			tmp = new QString("right curly bracket");
		else
		if(QString(lesson[studentatindex])=="-")
			tmp = new QString("dash");
		else
		if(QString(lesson[studentatindex])=="'")
			tmp = new QString("apostrophe");
		else
		if(QString(lesson[studentatindex])=="<")
			tmp = new QString("less than");
		else
		if(QString(lesson[studentatindex])==">")
			tmp = new QString("greater than");
    	else
    		tmp = new QString(lesson[studentatindex]);
    		
    	if(lesson[studentatindex].category() == QChar::Letter_Uppercase)
    			*tmp = "capital "+*tmp+": ";
    
    	if(level > letter || (level==letter && studentatlag!=studentatindex))
    	{
    		level = letter;	
    		saysomething(tmp);
    	}
    	else //say the position also
    	{
    		level = position;
    		*tmp += ": " + *(valuetosay->find(QString(lesson[studentatindex])));
    
    		saysomething(tmp); //fires off a thread
    	}
    	
    	studentatlag = studentatindex;
   	}
  }
#endif

}
