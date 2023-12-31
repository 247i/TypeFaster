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

#ifdef WIN
#include <windows.h>
#endif

#include "SettingsDialog.h"
#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <iostream>
using namespace std;

extern bool MULTI;
extern QString USERNAME;

bool READGAMEFILES = false;

SettingsDialog::SettingsDialog(QComboBox* l,QComboBox* l2,QWidget* p):QDialog(p,"settings",true,WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu)//  | WStyle_Tool) //| WStyle_NoBorder
{
	layoutcombo = l;
	reallessoncombo = l2;

	setCaption(tr("TypeFaster"));


	setFixedSize(280,260);

	

	layoutsenabledb = new QButtonGroup(tr("Layouts enabled"),this);
	layoutsenabledb->setGeometry(20,25,240,60);
	layoutsenabled = new QPushButton(tr("Change layouts enabled"),layoutsenabledb);
	layoutsenabled->setGeometry(20,20,200,30);
	connect(layoutsenabled,SIGNAL(clicked()),this,SLOT(layoutsenabledpressed()));

	lessonsb = new QButtonGroup(tr("Edit lessons of a layout"),this);
	lessonsb->setGeometry(20,90,240,60);
	layouts = new QComboBox(lessonsb);
	layouts->setGeometry(20,20,120,20);

	editlessons = new QPushButton(tr("Edit lessons"),lessonsb);
	editlessons->setGeometry(150,20,70,30);
	connect(editlessons,SIGNAL(clicked()),this,SLOT(editlessonspressed()));


	gamesettingsb = new QButtonGroup(tr("Game settings"),this);
	gamesettingsb->setGeometry(20,155,240,60);
	gamesettings = new QPushButton(tr("Game settings"),gamesettingsb);
	gamesettings->setGeometry(20,20,200,30);
	connect(gamesettings,SIGNAL(clicked()),this,SLOT(gamesettingspressed()));

	ok = new QPushButton(tr("Ok"),this);
	ok->setGeometry(170,220,90,30);
	connect(ok,SIGNAL(clicked()),this,SLOT(okpressed()));
	ok->setDefault(true);

	layoutsd = new QDialog(this,"layoutchecks",true,WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu);
	layoutsd->setFixedWidth(280);
	layoutsd->setCaption(tr("TypeFaster"));
	layoutsdb = new QButtonGroup(tr("Layouts enabled"),layoutsd);

	savelayouts = new QPushButton(tr("Save"),layoutsd);
	savelayouts->setDefault(true);
	cancellayouts = new QPushButton(tr("Cancel"),layoutsd);

	connect(savelayouts,SIGNAL(clicked()),this,SLOT(savelayoutspressed()));
	connect(cancellayouts,SIGNAL(clicked()),this,SLOT(cancellayoutspressed()));

	connect(this,SIGNAL(somethingpressed(int)),layoutsd,SLOT(done(int)));

	connect(this,SIGNAL(activatelayout(int)),layoutcombo,SIGNAL(activated(int)));


	//now for the edit lessons button
	lessonsd = new QDialog(this,"edit lessons",true,WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu);
	lessonsd->setCaption("TypeFaster");
	lessonsd->setFixedHeight(230);
	lessonsd->setFixedWidth(320);

	lessonslabel = new QLabel(lessonsd);
	lessonslabel->setGeometry(20,10,280,20);

	lessonsgroup = new QButtonGroup(tr("Edit or delete lessons"),lessonsd);
	lessonsgroup->setGeometry(20,50,280,60);

	lessonscombo = new QComboBox(lessonsgroup);
	lessonscombo->setGeometry(20,20,120,20);

	lessonsedit = new QPushButton(tr("Edit"),lessonsgroup);
	lessonsedit->setGeometry(150,20,40,30);
	connect(lessonsedit,SIGNAL(clicked()),this,SLOT(lessonseditpressed()));
	
	lessonsdelete = new QPushButton(tr("Delete"),lessonsgroup);
	lessonsdelete->setGeometry(200,20,60,30);
	connect(lessonsdelete,SIGNAL(clicked()),this,SLOT(lessonsdeletepressed()));

	lessonsgroup2 = new QButtonGroup(tr("Add lesson"),lessonsd);
	lessonsgroup2->setGeometry(20,115,280,60);
	lessonsnew = new QPushButton(tr("New lesson"),lessonsgroup2);
	lessonsnew->setGeometry(20,20,120,30);
	connect(lessonsnew,SIGNAL(clicked()),this,SLOT(lessonsnewpressed()));

	lessonsok = new QPushButton(tr("Ok"),lessonsd);
	lessonsok->setGeometry(210,190,90,30);
	lessonsok->setDefault(true);
	connect(lessonsok,SIGNAL(clicked()),lessonsd,SLOT(done(int)));


	//now for actually editing the text of a lesson, or making a new one
	lessoneditd = new QDialog(lessonsd,"edit one lesson",true,WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WStyle_Maximize);
	int w = QApplication::desktop()->width();
	int h = QApplication::desktop()->height();
	lessoneditd->setGeometry(w/6,h/6,w-w/3,h-h/3);
	grid = new QGridLayout(lessoneditd,6,2,10,10);
	grid->setColStretch(1,1);

	lessoneditd->setCaption(tr("TypeFaster"));
	lessoneditlabel = new QLabel(tr("Lesson name: "),lessoneditd);
	grid->addWidget(lessoneditlabel,0,0,AlignLeft);
	lessoneditline = new QLineEdit(lessoneditd);
	lessoneditline->setMinimumWidth(200);
	lessoneditline->setMaxLength(120);
	grid->addWidget(lessoneditline,0,1,AlignLeft);

	lessoneditlabel2 = new QLabel(tr("Lesson author: "),lessoneditd);
	grid->addWidget(lessoneditlabel2,1,0,AlignLeft);
	lessoneditline2 = new QLineEdit(lessoneditd);
	lessoneditline2->setMinimumWidth(200);
	lessoneditline2->setMaxLength(200);
	grid->addWidget(lessoneditline2,1,1,AlignLeft);

	lessoneditlabel3 = new QLabel(tr("Lesson type: "),lessoneditd);
	grid->addWidget(lessoneditlabel3,2,0,AlignLeft);
	lessoneditcombo = new QComboBox(lessoneditd);
	lessoneditcombo->insertItem(tr("Normal"));
	lessoneditcombo->insertItem(tr("Poem"));
	lessoneditcombo->insertItem(tr("Numeric"));
	grid->addWidget(lessoneditcombo,2,1,AlignLeft);

	lessoneditcheck = new QCheckBox(tr("Left to right"),lessoneditd);
	grid->addWidget(lessoneditcheck,3,0,AlignLeft);

	lessoneditmulti = new QMultiLineEdit(lessoneditd);
	lessoneditmulti->setWordWrap(QMultiLineEdit::NoWrap);
	grid->addMultiCellWidget(lessoneditmulti,4,4,0,1);

	lessoneditsave = new QPushButton(tr("Save"),lessoneditd);
	lessoneditsave->setDefault(true);
	grid->addWidget(lessoneditsave,5,0,AlignLeft);
	connect(lessoneditsave,SIGNAL(clicked()),this,SLOT(lessoneditsavepressed()));

	lessoneditcancel = new QPushButton(tr("Cancel"),lessoneditd);
	grid->addWidget(lessoneditcancel,5,1,AlignRight);
	connect(lessoneditcancel,SIGNAL(clicked()),lessoneditd,SLOT(accept()));

	newlessonwanted = false;


	gamed = new QDialog(this,"game dialog",true,WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WStyle_Maximize);
	gamed->setGeometry(w/6,h/6,w-w/3,h-h/3);
	gamegrid = new QGridLayout(gamed,5,2,10,10);
	gamegrid->setColStretch(1,1);
	gamed->setCaption(tr("TypeFaster"));


	gamelabel = new QLabel(tr("Edit game words:"),gamed);
	gamegrid->addWidget(gamelabel,0,0,AlignLeft);

	gamemulti = new QMultiLineEdit(gamed);
	gamemulti->setWordWrap(QMultiLineEdit::NoWrap);
	gamegrid->addMultiCellWidget(gamemulti,1,1,0,1);

	gamelabel2 = new QLabel(tr("Initial Word Speed: "),gamed);
	gamegrid->addWidget(gamelabel2,2,0,AlignLeft);

	gamedial = new QDial(1,100,10,10,gamed);
	gamedial->setLineStep(10);
	gamedial->setNotchesVisible(true);
	gamedial->setBackgroundMode(PaletteDark);
	gamedial->setFixedSize(60,60);
	gamegrid->addWidget(gamedial,2,1,AlignLeft);

	gamelabel3 = new QLabel(tr("Final word speed: "),gamed);
	gamegrid->addWidget(gamelabel3,3,0,AlignLeft);

	gamedial2 = new QDial(1,100,10,90,gamed);
	gamedial2->setLineStep(10);
	gamedial2->setNotchesVisible(true);
	gamedial2->setBackgroundMode(PaletteDark);
	gamedial2->setFixedSize(60,60);
	gamegrid->addWidget(gamedial2,3,1,AlignLeft);

	gamesave = new QPushButton(tr("Save"),gamed);
	gamesave->setDefault(true);
	gamegrid->addWidget(gamesave,4,0,AlignLeft);
	connect(gamesave,SIGNAL(clicked()),this,SLOT(gamesavepressed()));

	gamecancel = new QPushButton(tr("Cancel"),gamed);
	gamegrid->addWidget(gamecancel,4,1,AlignRight);
	connect(gamecancel,SIGNAL(clicked()),this,SLOT(gamecancelpressed()));

	
}
void SettingsDialog::editlessonspressed() //this is the edit lessons button next to a layout name
{
  if(layouts->count()>0)
  {
	lessonslabel->setText(tr("Edit, add or delete ")+layouts->currentText()+tr(" lessons"));

	lessonscombo->clear(); //populate this with the lesson files in the dir
	
	if(MULTI)
		lessonsloc = "users/" + USERNAME + "/layouts/" + layouts->currentText();
	else
		lessonsloc = "defaultuser/layouts/" + layouts->currentText();

	QDir d(lessonsloc);

        if ( !d.exists() )
			d.mkdir(d.absPath());  
        else
        {

            d.setFilter(QDir::Files);
            
            std::vector<lessonandnum> lessons;
            std::vector<QString> othernames;
            int i;
            for (i=0; i<d.count(); i++ )
            {
                if(d[i]!="."&&d[i]!=".."&&d[i].mid(d[i].length()-4,4)==".txt")
                {
                    if(d[i].mid(0,7) == "Lesson " || d[i].mid(0,8) == "lecci�n ")
                    {
                        bool ok = false;
                        int num = d[i].mid(7,d[i].length()-11).toInt(&ok);
                        if(ok)
                            lessons.push_back(lessonandnum(d[i].mid(0,d[i].length()-4),num));
                        else
                            othernames.push_back(d[i].mid(0,d[i].length()-4));
                    }
                    else
                        othernames.push_back(d[i].mid(0,d[i].length()-4));
                }
            }
            //now sort the lessons by number
            for(int x=0;x < (int)lessons.size()-1 ;x++)  
            {
                for(int y = x+1; y<lessons.size(); y++)
                    if(lessons[x].num > lessons[y].num)
                    {
                        lessonandnum tmp = lessons[x]; //default copy constructor
                        lessons[x] = lessons[y];
                        lessons[y] = tmp;
                    }
            }
        
            
                
            for(i=(int)othernames.size()-1;i>=0;i--)
            {
                lessonscombo->insertItem(othernames[i],0);
            }
            for(i=(int)lessons.size()-1;i>=0;i--)
                lessonscombo->insertItem(lessons[i].lesson,0);
            
                
            
        } //end dir didn't exist

	lessonsd->exec();
  }
}
void SettingsDialog::populatelessoneditd()
{
		lessoneditdefaults();
		lessoneditline->setText(lessonscombo->currentText());
		lessoneditline->setCursorPosition(0);
		QFile lessonfile(lessonsloc+"/"+lessonscombo->currentText()+".txt");
		//no translation necessary here
		
		if(lessonfile.exists())
		{
			lessonfile.open(IO_ReadOnly);
		
			QTextStream lessonstream(&lessonfile);
			lessonstream.setEncoding(QTextStream::Unicode);
			QString line;
			if((line=lessonstream.readLine())!=0)
			{
				if(line=="version=1")
				{
					if((line=lessonstream.readLine())!=0)
					{
						if(line.mid(0,12)=="lefttoright=")
						{
							QString tmp = line.mid(12,5);
							if(tmp=="true" || tmp=="false")
							{
								if(tmp=="true")
									lessoneditcheck->setChecked(true);
								else
									lessoneditcheck->setChecked(false);

								if((line=lessonstream.readLine())!=0)
								{
									if(line.mid(0,7)=="author=")
									{
										lessoneditline2->setText(line.mid(7,line.length()));
										lessoneditline2->setCursorPosition(0);
										if((line=lessonstream.readLine())!=0)
										{
											if(line=="numeric=true"||line=="numeric=false"||line=="poem=true")
											{
												
												if(line=="numeric=false")
													lessoneditcombo->setCurrentItem(0);//normal
												else
												if(line=="poem=true")
													lessoneditcombo->setCurrentItem(1);
												else
													lessoneditcombo->setCurrentItem(2);
												
												QString whole;
												while((line=lessonstream.readLine())!=0)
												{
													if(line!="")
														whole += line+'\n';
												}
												lessoneditmulti->setText(whole);
											}
											else
												QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file seems corrupt (has invalid numeric line)"));
										}
										else
											QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file seems corrupt (has no numeric line)"));
									}
									else
										QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file seems corrupt (has no author line)"));
								}
								else
									QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file seems corrupt (has no author line)"));
							}
							else
								QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file seems corrupt (lefttoright is neither true nor false)"));
						}
						else
							QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file seems corrupt (has no lefttoright line)"));
					}
					else
						QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file seems corrupt (has no lefttoright line)"));
				}
				else
					QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file version is not 1, try upgrading the software"));
			}
			else
				QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, lesson file seems to be corrupt"));
		}
		else
			QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Warning, could not read lesson file since it no longer exists"));
}
void SettingsDialog::lessonseditpressed()
{
	if(lessonscombo->count()>0)
	{
		newlessonwanted = false;
		populatelessoneditd();
		lessoneditline->setDisabled(true); //can't change filename of a lesson that you are just editing
		lessoneditd->show();
	}
}
void SettingsDialog::lessonsdeletepressed()
{
	if(lessonscombo->count()>0)
	if( QMessageBox::warning(lessonsd, tr("TypeFaster"),
                                "Are you sure you want to delete a lesson file?\n",
                          "Yes", "No",QString::null,0,1) == 0)
	{
		QFile::remove(lessonsloc+"/"+lessonscombo->currentText()+".txt");
		
		//don't like this because then there could be a display inconsistency
		//ie it will say you are doing lesson 3 in the Lesson: combobox but you
		//will actually be doing lesson 2. Don't have to remove it from combobox
		//since it will just say file doesn't exist if clicked on
		/*
		if(layoutcombo->currentText()==layouts->currentText())
		{
			int i;
			for(i=0;i<reallessoncombo->count();i++)
				if(reallessoncombo->text(i)==lessonscombo->currentText())
				{
					reallessoncombo->removeItem(i);
					break;
				}
		}*/

		lessonscombo->removeItem(lessonscombo->currentItem());
	}
  
}
void SettingsDialog::lessoneditsavepressed()
{
	QString n = lessoneditline->text();
	if(n.length()>0)
	{
	  if(lessoneditmulti->text().length()>0)
	  {
		if(n.contains("/")>0 || n.contains("\\")>0 || n.contains(":")>0 || n.contains("*")>0 || n.contains("?")>0 || n.contains("\"")>0 || n.contains("<")>0 || n.contains(">")>0 || n.contains("|")>0)
		{
			QMessageBox::warning(0,"TypeFaster",tr("A lesson name cannot contain any of the following characters: \n \\ / : * ? \" < > |"));
		}
		else
		{ //write the file out
			QFile lfile(lessonsloc+"/"+lessoneditline->text()+".txt"); 
		
			lfile.open(IO_WriteOnly);	
			QTextStream lstream(&lfile);
			lstream.setEncoding(QTextStream::Unicode);
			lstream << "version=1" << endl;
			if(lessoneditcheck->isChecked())
				lstream << "lefttoright=true" << endl;
			else
				lstream << "lefttoright=false" << endl;
			lstream << "author=" << lessoneditline2->text() << endl;
			switch(lessoneditcombo->currentItem())
			{
				case 0:
					lstream << "numeric=false" << endl;
					break;
				case 1:
					lstream << "poem=true" << endl;
					break;
				case 2:
					lstream << "numeric=true" << endl;
					break;
			}
			lstream << endl;
			lstream << lessoneditmulti->text();

			lfile.close();

			if(newlessonwanted)
			{
				lessonscombo->insertItem(lessoneditline->text());
				if(layoutcombo->currentText()==layouts->currentText())
					reallessoncombo->insertItem(lessoneditline->text());
			}
			lessoneditd->hide();
		}
	  }
	  else
		QMessageBox::warning(0,"TypeFaster",tr("Lesson must be at least 1 character long"));
	}
	else
		QMessageBox::warning(0,"TypeFaster",tr("Please key in a lesson name"));

}
void SettingsDialog::lessoneditdefaults()
{
	lessoneditline->setText("");
	lessoneditline2->setText("");
	lessoneditcombo->setCurrentItem(0);
	lessoneditcheck->setChecked(true);
	lessoneditmulti->setText("");
}
void SettingsDialog::lessonsnewpressed()
{
	newlessonwanted = true;
	lessoneditdefaults();
	lessoneditline->setDisabled(false);
	lessoneditd->show();

}
void SettingsDialog::savelayoutspressed()
{
	emit somethingpressed(1);
}
void SettingsDialog::cancellayoutspressed()
{
	emit somethingpressed(0);
}
void SettingsDialog::dothere() //finds which layouts are already there, and populates combobox
{
	int i;
	for(i=0;i<there.size();i++)
		delete there[i];
	there.clear();

	QString theredir;
	if(MULTI)
		theredir = "users/" + USERNAME + "/layouts";
	else
		theredir = "defaultuser/layouts";

	layouts->clear();

	QDir d(theredir.latin1()); 
	d.setFilter(QDir::Files);
    if ( d.exists() )
	{
		unsigned int i;
		for (i=0; i<d.count(); i++ )
			if(d[i]!="."&&d[i]!="..")
			{
				QCheckBox* tmp = new QCheckBox(d[i].mid(0,d[i].length()-4),layoutsdb);
				tmp->setChecked(true);
				there.push_back(tmp);
				layouts->insertItem(tmp->text());
			}
	}
}

SettingsDialog::~SettingsDialog()
{

	delete lessoneditlabel;
	delete lessoneditline;
	delete lessoneditlabel2;
	delete lessoneditline2;
	delete lessoneditlabel3;
	delete lessoneditcombo;
	delete lessoneditcheck;
	delete lessoneditmulti;
	delete lessoneditsave;
	delete lessoneditcancel;
	delete grid;
	delete lessoneditd;


	delete lessonslabel;

	delete lessonscombo;
	delete lessonsedit;
	delete lessonsdelete;
	delete lessonsgroup;

	delete lessonsnew;
	delete lessonsgroup2;

	delete lessonsok;
	delete lessonsd;

	delete savelayouts;
	delete cancellayouts;

	int i;
	for(i=0;i<there.size();i++)
		delete there[i];
	there.clear();
	for(i=0;i<available.size();i++)
		delete available[i];
	available.clear();

	delete layoutsdb;
	delete layoutsd;

	delete ok;

	delete layoutsenabled;
	delete layoutsenabledb;

	delete layouts;
	delete editlessons;
	delete lessonsb;

	delete gamesettings;
	delete gamesettingsb;
}

void SettingsDialog::doavailable()
{
	int i;
	for(i=0;i<available.size();i++)
		delete available[i];
	available.clear();

	QDir d("layouts"); 
	d.setFilter(QDir::Files);
    if ( d.exists() )
	{
		unsigned int i;
		for (i=0; i<d.count(); i++ )
			if(d[i]!="."&&d[i]!="..")
			{
				bool found = false;
				int x;
				for(x=0;x<there.size();x++)
					if(there[x]->text()==d[i].mid(0,d[i].length()-4))
					{
						found = true;
						break;
					}
				if(!found)
				{
					QCheckBox* tmp = new QCheckBox(d[i].mid(0,d[i].length()-4),layoutsdb);
					tmp->setChecked(false);
					available.push_back(tmp);
				}
				
			}
	}

}

void SettingsDialog::getLayout(QString l) //layoutname, username
{
#ifdef WIN
	
	/*
	STARTUPINFO startInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&startInfo,sizeof(startInfo));
	startInfo.cb = sizeof(startInfo);
	if(CreateProcess(NULL,(char*)s.latin1(),NULL,NULL,false,HIGH_PRIORITY_CLASS|CREATE_NO_WINDOW,NULL,NULL,&startInfo,&processInfo))
	{
		CloseHandle(&processInfo.hThread);
		CloseHandle(&processInfo.hProcess);
	}*/
	QString from = "layouts\\"+l+".xml";
	QString to;
	if(MULTI)
		to = "users\\"+USERNAME+"\\layouts\\"+l+".xml";
	else
		to = "defaultuser\\layouts\\"+l+".xml";

	CopyFile(from.latin1(),to.latin1(),false);

	QString dir;
	if(MULTI)
		dir = "users\\"+USERNAME+"\\layouts\\"+l;
	else
		dir = "defaultuser\\layouts\\"+l;

	CreateDirectory(dir.latin1(),false);

	QString src = "layouts\\"+l;
	QDir d(src.latin1()); 
	d.setFilter(QDir::Files);
    if ( d.exists() )
	{
		unsigned int i;
		for (i=0; i<d.count(); i++ )
			if(d[i]!="."&&d[i]!="..")
			{
				from = "layouts\\"+l+"\\"+d[i];
				if(MULTI)
					to = "users\\"+USERNAME+"\\layouts\\"+l+"\\"+d[i];
				else
					to = "defaultuser\\layouts\\"+l+"\\"+d[i];

				CopyFile(from.latin1(),to.latin1(),false);
			}
	}
	
#else
	QString cmd;
	if(MULTI)
		cmd = "cp -r layouts/"+l+" users/"+USERNAME+"/layouts";
	else
		cmd = "cp -r layouts/"+l+" defaultuser/layouts";

	system(cmd.latin1());

	if(MULTI)
		cmd = "cp layouts/"+l+".xml users/"+USERNAME+"/layouts";
	else
		cmd = "cp layouts/"+l+".xml defaultuser/layouts";

	system(cmd.latin1());
#endif
}

void SettingsDialog::layoutsenabledpressed()
{
	//doavailable();
	int tot = there.size()+available.size();
	layoutsd->setFixedHeight(tot*20+100);
	layoutsdb->setGeometry(20,20,240,tot*20+30);
	int i;
	for(i=0;i<there.size();i++)
		there[i]->setGeometry(20,20+i*20,120,20);
	int x;
	for(x=0;x<available.size();x++)
		available[x]->setGeometry(20,20+there.size()*20+x*20,120,20);

	savelayouts->setGeometry(20,tot*20+60,90,30);
	cancellayouts->setGeometry(170,tot*20+60,90,30);
	switch(layoutsd->exec())
	{
		case 0: //just make sure what's there is checked and what's not isn't
			for(i=0;i<there.size();i++)
				there[i]->setChecked(true);
			for(i=0;i<available.size();i++)
				available[i]->setChecked(false);
			break;
		case 1: //delete the there stuff that isn't checked, 
			//and copy the availble stuff that is checked, and repopulate combobox
			for(i=0;i<there.size();i++)
				if(!there[i]->isChecked())
				{
					QString theredir;
					if(MULTI)
						theredir = "users/" + USERNAME + "/layouts";
					else
						theredir = "defaultuser/layouts";
					QFile::remove(theredir+"/"+there[i]->text()+".xml");

					QDir d(theredir+"/"+there[i]->text());
					d.setFilter(QDir::Files);
					if ( d.exists() )
					{
						int x;
						for (x=0; x<d.count(); x++ )
							if(d[x]!="."&&d[x]!="..")
							{
								QFile::remove(d.absPath()+"/"+d[x]);
							}
						d.rmdir(d.absPath());
					}
					int x;
					for(x=0;x<layoutcombo->count();x++)
						if(layoutcombo->text(x)==there[i]->text())
						{
							if(layoutcombo->count()>1 && (layoutcombo->currentText()==there[i]->text()))
							{
								if(x!=0)
									emit activatelayout(0);
								else
									emit activatelayout(1);
							}

							layoutcombo->removeItem(x);
							break;
						}
				}

			for(i=0;i<available.size();i++)
			{
				if(available[i]->isChecked())
				{
					getLayout(available[i]->text());
					layoutcombo->insertItem(available[i]->text());
				}
			}
			dothere();
			doavailable();
			
			break;
	}
}

void SettingsDialog::gamesettingspressed()
{
	QString gameloc;
	if(MULTI) //it cannot be a student of a teacher since the settings button will not appear
		gameloc = "users/"+USERNAME+"/game";
	else
		gameloc = "defaultuser/game";
	int start = 10;
	int end = 90;
	QFile sfile(gameloc+"/settings.txt");
	if(sfile.open(IO_ReadOnly))
	{
		QTextStream sstream(&sfile);
		QString line = sstream.readLine();
		if(line!=0)
		{
			bool ok;
			start = line.toInt(&ok);
			if(!ok || start<1 || start > 100)
				start = 10;
			line = sstream.readLine();
			end = line.toInt(&ok);
			if(!ok || end < 1 || end > 100)
				end = 90;
		}
		sfile.close();
	}
	
	gamedial->setValue(start);
	gamedial2->setValue(end);

	QFile wfile(gameloc+"/words.txt");
	QString words;
	if(wfile.open(IO_ReadOnly))
	{
		QTextStream wstream(&wfile);
		QString line;
		while((line=wstream.readLine())!=0)
		{
			words += line + '\n';
		}
		wfile.close();
	}
	if(words.length()>0)
		gamemulti->setText(words);
	else
		gamemulti->setText("test");
	gamed->show();
}
void SettingsDialog::gamesavepressed()
{
	if(gamemulti->text().length()>0)
	{
		READGAMEFILES = true;
		QString gameloc;
		if(MULTI) //it cannot be a student of a teacher since the settings button will not appear
			gameloc = "users/"+USERNAME+"/game";
		else
			gameloc = "defaultuser/game";
		int start = gamedial->value();
		int end = gamedial2->value();

		QFile sfile(gameloc+"/settings.txt");
		if(sfile.open(IO_WriteOnly))
		{
			QTextStream sstream(&sfile);
			sstream << start << endl;
			sstream << end << endl;
			sfile.close();
		}
		QFile wfile(gameloc+"/words.txt");
		if(wfile.open(IO_WriteOnly))
		{
			QTextStream wstream(&wfile);
			wstream << gamemulti->text();
			wfile.close();
		}
		gamed->hide();
	}
	else
		QMessageBox::warning(0,QMessageBox::tr("TypeFaster Typing Tutor"),QMessageBox::tr("Please key in a word"));
}
void SettingsDialog::gamecancelpressed()
{
	gamed->hide();
}
void SettingsDialog::okpressed()
{
	done(0);
}
