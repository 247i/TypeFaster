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
#include "MyParser.h"


#include <qmessagebox.h>

#include <qstring.h>
#include <qstringlist.h>
//#include <cstdlib>
//#include <iostream>
//using namespace std;

//MyParser::MyParser(std::vector<MyButton*>* vec,KeyBoardWidget* main,float* r)
MyParser::MyParser(std::vector<MyButton*>* vec,QWidget* main,float* r,bool* ltr)
{
	// QXmlDefaultHandler::QXmlDefaultHandler();
	v = vec;
	m = main;
	ratio = r;
	lefttoright = ltr;
}
bool MyParser::startDocument()
{
	//cout << "parser startdoc" <<endl;
	//return false;
	layout=false;
	version1=false;
	withinrow=false;
	withinkey=false;
	withinvalue=false;

	horizgap=0.0;
	vertgap=0.0;
	
	va=0.0;
	vd=0.0;

	kwidth=0.0;
	kheight=0.0;

	tkwidth=0.0;

    return true;
}
bool MyParser::endDocument()
{
	//cout << "enddoc parser "<<kheight << " "<<kwidth << endl;
	lastbutton->kwidth = kwidth;
	lastbutton->kheight = kheight;
	lastbutton->horizgap = horizgap;
	lastbutton->vertgap = vertgap;

	*ratio = kwidth/kheight;
	return true;
	//return false;
}

bool MyParser::startElement(const QString&, const QString& name, const QString& qName, const QXmlAttributes& att)
{
	//cout << "parser startElement" << endl;
	if(!layout)
	{
		if(name!="layout")
		{
			//std::cout << "Error, first element not layout"<<std::endl;
			QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, first element not layout");
			//don't bother translating
			exit(1);
		}
		else
		{
			if(att.length()>0)
			{
				if(att.localName(0)=="version")
				{
					if(att.value(0)=="1.0")
					{
						version1=true;
						if(att.length()>1)
						{
							if(att.localName(1)=="horizgap")
							{
								bool ok=false;
								horizgap = att.value(1).toFloat(&ok);
								if(!ok)
								{
									//std::cout << "Error, horizgap attribute cannot be converted to a float"<<std::endl;
									QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error,horizgap attribute cannot be converted to a float");
									//don't bother translating
									exit(1);
								}
								if(att.length() > 2)
								{
									if(att.localName(2)=="vertgap")
									{
										bool ok=false;
										vertgap = att.value(2).toFloat(&ok);
										if(!ok)
										{
											//std::cout << "Error, vertgap is non-float"<<std::endl;
											QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, vertgap attribute cannot be converted to a float");
											//don't bother translating
											exit(1);
										}
										if(att.length()>3)
										{
											if(att.localName(3)=="ltr")
											{
												if(att.value(3)=="true" || att.value(3)=="false")
												{
													if(att.value(3)=="true")
														*lefttoright = true;
													else
														*lefttoright = false;
												}
												else
													*lefttoright = true;
											}
											else
												*lefttoright = true;
										}
										else
											*lefttoright = true;
									}
									else
									{
										//std::cout << "Error, vertgap is not the third attribute in layout"<<std::endl;
										QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error vertgap is not third attribute in layout");
										//don't bother translating
										exit(1);
									}
								}
								else
								{
									//std::cout << "Error, not enough attributes in layout element"<<std::endl;
									QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, not enough attributes in layout element");
									//don't bother translating
									exit(1);
								}
							}
							else
							{
								//std::cout << "Error, layout element does not have horizgap attribute"<<std::endl;
								QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, layout element does not have horizgap attribute");
								//don't bother translating
								exit(1);
							}
						}
						else
						{
							//std::cout << "Error, layout does not have 2 elements"<<std::endl;
							QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, layout element does not have 2 attributes");
							//don't bother translating
							exit(1);
						}
					}
					else
					{
						//std::cout << "Error, this program does not understand this version of the layout format, upgrade the software"<<std::endl;
						QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, this program does not understand this version of layout - upgrade the software");
						//don't bother translating
						exit(1);
					}

				}
				else
				{
					//std::cout << "Error, layout element does not have version attribute"<<std::endl;
					QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, layout element does not have version attribute");
					//don't bother translating
					exit(1);
				}
			}
			else
			{
				//std::cout << "Error, layout element does not have version attribute"<<std::endl;
				QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, layout element does not have version attribute");
				//don't bother translating
				exit(1);
			}
				
		}
		layout = true;
		
	}
	else
	{
		if(version1 && name=="row" && !withinrow)
		{
			withinrow=true;
		//	lastrow.clear();
		
			if(att.length()==1 && att.localName(0)=="scale")
			{
				bool ok=false;
				lastrowscale = att.value(0).toFloat(&ok);
				if(ok)
				{
					va=0.0;
					if(kheight == 0.0)
						kheight = lastrowscale;
					else
						kheight += horizgap + lastrowscale; 
				}
				else
				{
					//std::cout << "Error, scale attribute in row is non-float"<<std::endl;
					QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, scale attribute in row is not a float");
					//don't bother translating
					exit(1);
				}
			}
			else
			{
				//std::cout << "Error, row element's first attribute is not scale"<<std::endl;
				QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, row element's first attribute is not scale");
				//don't bother translating
				exit(1);
			}
		}
		else
			if(version1 && name=="key" && !withinkey && withinrow)
			{
				withinkey = true;
				
				float width=0.0;

				MyButton::shapetype shapet;
				//MyButton::keytype keyt;
				std::vector<MyButton::keytype> keyt;
				bool homekey;
				//bool autorepeat;
				QString size;
				int homeindex;
			//	bool botborder=true;

				if(att.length() > 3)
				{
					if(att.localName(0)=="shape" && att.localName(1)=="type" && att.localName(2)=="homekey" && att.localName(3)=="size")
					{
						if(att.value(0)=="rect")
							shapet = MyButton::rect;
							else 
							if(att.value(0)=="square")
								shapet = MyButton::square;
								else
								if(att.value(0)=="irregular")
									shapet = MyButton::irregular;
									else
									{
										//std::cout << "Error, key element shape attribute value is not rect, square or irregular"<<std::endl;
										QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, key element shape attribute value is not rect, square or irregular");
										//don't bother translating
										exit(1);
									}
	
					QStringList l = QStringList::split(";",att.value(1));
					if(l.count() < 1)
						keyt.push_back(MyButton::normal);
					else
					for(int i=0;i<l.count();i++)
					{
						if(l[i]=="normal")
							keyt.push_back(MyButton::normal);
						else
						if(l[i]=="altgr")
							keyt.push_back(MyButton::altgr);
						else
						if(l[i]=="alt")
							keyt.push_back(MyButton::alt);
						else
						if(l[i]=="enter")
							keyt.push_back(MyButton::enter);
						else
						if(l[i]=="backspace")
							keyt.push_back(MyButton::backspace);
						else
						if(l[i]=="tab")
							keyt.push_back(MyButton::tab);
						else
						if(l[i]=="capslock")
							keyt.push_back(MyButton::capslock);
						else
						if(l[i]=="control")
							keyt.push_back(MyButton::control);
						else
						if(l[i]=="leftshift")
							keyt.push_back(MyButton::leftshift);
						else
						if(l[i]=="rightshift")
							keyt.push_back(MyButton::rightshift);
						else
						if(l[i]=="numlock")
							keyt.push_back(MyButton::numlock);
						else
						if(l[i]=="forwardaccent")
							keyt.push_back(MyButton::forwardaccent);
						else
						if(l[i]=="doubledot")
							keyt.push_back(MyButton::doubledot);
						else
						if(l[i]=="hat")
							keyt.push_back(MyButton::hat);
						else
						if(l[i]=="backwardaccent")
							keyt.push_back(MyButton::backwardaccent);
						else
						if(l[i]=="squiggle")
							keyt.push_back(MyButton::squiggle);
						else
						if(l[i]=="cedilla")
							keyt.push_back(MyButton::cedilla);
						else
						if(l[i]=="caron")
							keyt.push_back(MyButton::caron);
						else
						if(l[i]=="breve")
							keyt.push_back(MyButton::breve);
						else
						if(l[i]=="degreesign")
							keyt.push_back(MyButton::degreesign);
						else
						if(l[i]=="ogonek")
							keyt.push_back(MyButton::ogonek);
						else
						if(l[i]=="dotabove")
							keyt.push_back(MyButton::dotabove);
						else
						if(l[i]=="doubleacuteaccent")
							keyt.push_back(MyButton::doubleacuteaccent);
						else
						{
							//std::cout << "Error, key element type attribute value is not known"<<std::endl;
							QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, key element type attribute value is not known");
							//don't bother translating
							exit(1);
						}
					}

						if(att.value(2)=="true")
							homekey=true;
						else
						if(att.value(2)=="false")
							homekey=false;
						else
						{
							//std::cout << "Error, key element, homekey attribute value is not true or false"<<std::endl;
							QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, key element homekey attribute value is not true or false");
							//don't bother translating
							exit(1);
						}						

						size=att.value(3);
						QStringList sl = QStringList::split(";",size);
						QStringList::Iterator it;
						bool ok=false;
						switch (shapet)
						{
							case MyButton::square:
							width = size.toFloat(&ok);
							if(!ok)
							{
								//std::cout << "Error, key element size attribute value cannot be converted to float"<<std::endl;
								QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, "+size+": Key element size attribute value cannot be converted to float");
								//don't bother translating
								exit(1);
							}
							break;
							case MyButton::rect:
							it = sl.begin();
							width = (*it).toFloat(&ok); //width is the first field
							if(!ok)
							{
								//std::cout << "Error, key element size attribute value cannot be converted to float"<<std::endl;
								QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, "+(*it)+": Key element size attribute value cannot be converted to float");
								//don't bother translating
								exit(1);
							}
							break;
							case MyButton::irregular:
							for(it=sl.begin();it!=sl.end();++it)
							{
								if((*it).at((*it).length()-1)=='E')
								{
									//(*it).truncate((*it).length()-1);
									QString tmp = (*it);
									tmp.truncate(tmp.length()-1);
									width += tmp.toFloat(&ok);
									if(!ok)
									{
										//std::cout << "Error, key element size attribute value cannot be converted to float"<<std::endl;
										QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, "+(*it)+": Key element size attribute value cannot be converted to float");
										exit(1);
									}
								}
							}
							break;
						}
					}
					else
					{
						//std::cout << "Error, key element doesn't have right attributes"<<std::endl;
						QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, key element doesn't have right attributes");
						//don't bother translating
						exit(1);
					}
					bool ok = false;
					if(homekey)
						homeindex = -1;
					else
					{
						if(att.length()>4)
						{
							if(att.localName(4)=="homeindex")
							{
								homeindex = att.value(4).toInt(&ok);
								if(!ok)
								{
									//std::cout << "Error, homeindex attribute of key is non-int"<<std::endl;
									QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, "+att.value(5)+": Homeindex attribute of key isn't an int");
									//don't bother translating
									exit(1);
								}
							}
							else
							{
								//std::cout << "Error, sixth attribute of key isn't homeindex"<<std::endl;
								QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, sixth attribute of key isn't homeindex");
								//don't bother translating
								exit(1);
							}
							
					/*		if(att.length()>6)
							{
								if(att.localName(6) == "botborder")
									if(att.value(6)=="false")
										botborder = false;
							}*/
						}
						else
							homeindex = -1;
					}
					
				}
				else
				{
					//std::cout << "Error, key element doesn't have enough attributes"<<std::endl;
					QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, key element doesn't have enough attributes");
					//don't bother translating
					exit(1);
				}

				lastbutton = new MyButton(shapet,keyt,homekey,size,homeindex,va,vd,m);
				(*v).push_back(lastbutton);
		//		lastrow.push_back(lastbutton);
			//	if(!botborder)
			//		lastbutton->botborder = false;
				
				if(tkwidth==0.0)
					tkwidth = width;
				else
					tkwidth += vertgap + width;
								
				va += vertgap +width;

			}
			else
				if(version1 && name=="value" && withinkey)
				{
					withinvalue=true;
					//when = MyButton::normal;
					when.clear();
					draw = false;
					newline = false;
					say = "";
					if(att.length()>1)
					{
					  if(att.localName(0)=="when" && att.localName(1)=="draw")
					  {
						  QStringList l  = QStringList::split(";",att.value(0));
						  if(l.count() > 0)
						  {
							  for(int i=0;i<l.count();i++)
							  {
								if(l[i]=="normal")
									when.push_back(MyButton::normal);
								else
								if(l[i]=="altgr")
									when.push_back(MyButton::altgr);
								else
								if(l[i]=="leftshift")
									when.push_back(MyButton::leftshift);
								else
								if(l[i]=="rightshift")
									when.push_back(MyButton::rightshift);
								else
								if(l[i]=="forwardaccent")
									when.push_back(MyButton::forwardaccent);
								else
								if(l[i]=="doubledot")
									when.push_back(MyButton::doubledot);
								else
								if(l[i]=="hat")
									when.push_back(MyButton::hat);
								else
								if(l[i]=="backwardaccent")
									when.push_back(MyButton::backwardaccent);
								else
								if(l[i]=="squiggle")
									when.push_back(MyButton::squiggle);
								else
								if(l[i]=="cedilla")
									when.push_back(MyButton::cedilla);
								else
								if(l[i]=="caron")
									when.push_back(MyButton::caron);
								else
								if(l[i]=="breve")
									when.push_back(MyButton::breve);
								else
								if(l[i]=="degreesign")
									when.push_back(MyButton::degreesign);
								else
								if(l[i]=="ogonek")
									when.push_back(MyButton::ogonek);
								else
								if(l[i]=="dotabove")
									when.push_back(MyButton::dotabove);
								else
								if(l[i]=="doubleacuteaccent")
									when.push_back(MyButton::doubleacuteaccent);
								else
								if(l[i]=="capslock")
									when.push_back(MyButton::capslock);
								else
								{
									//std::cout << "Error, value element when attribute value is not known"<<std::endl;
									QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, "+l[i]+" Value element when attribute value is not known");
									//don't bother translating
									exit(1);
								}
							  }
						  }
						  else
						  {
							  when.push_back(MyButton::normal);
						  }

						if(att.value(1)=="true")
							draw=true;
						else
							if(att.value(1)=="false")
								draw=false;
							else
							{
								//std::cout << "Error, value element's draw attribute is not true or false"<<std::endl;
								QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, value element's draw attribute is not true or false");
								//don't bother translating
								exit(1);
							}
					  }
					  else
					  {
						  //std::cout << "Error, value element doesn't have right attributes"<<std::endl;
						QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, value element does not have right attributes,'when' and 'draw'");
						//don't bother translating
						exit(1);
					  }

					  if(att.length()>2)
					  {
						  if(att.localName(2)=="newline")
						  {
							  if(att.value(2)=="true")
								  newline=true;
							  else
								  if(att.value(2)=="false")
									  newline=false;//it is already
								  else
								  {
									  //std::cout << "Error, newline attribute of value element is not boolean"<<std::endl;
									QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, newline attribute of value element is not true or false");
									//don't bother translating
									exit(1);
								  }
						  }
						  else
						  {
							  //std::cout << "Error, 3rd attribute of value element is not newline "<<std::endl;
							  QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, third attribute of value is not newline");
							  //don't bother translating
								exit(1);
						  }
						  if(att.length()>3)
						  {
							  if(att.localName(3)=="say")
							  {
								  say = att.value(3);
							  }
							  else
							  {
									QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, fourth attribute of value is not say");
								//don't bother translating
							    	exit(1);
							  }
						  }
						  //else say=""
					  }
					  else
						  newline=false; //it is already
					}
					else
					{
						//std::cout << "Error, not enough attributes for value element"<<std::endl;
						QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, not enough attributes for value element, needs 'when' and 'draw'");
						//don't bother translating
						exit(1);
					}

				}
				else
				{
					//std::cout << "Error, unknown element or element not within correct structure"<<std::endl;
					QMessageBox::critical(0,"TypeFaster Typing Tutor","Parse fatal error, "+name + ": Unknown element or element not within correct structure");
					//don't bother translating
					exit(1);
				}
	}
    return true;
}
bool MyParser::characters(const QString& c) 
{
	//seems to get a lot of newlines
	if(version1 && withinvalue)
	{
		lastbutton->addvalue(c,when,draw,newline,say);
	}
	return true;
}
bool MyParser::endElement( const QString&, const QString& name, const QString& )
{
	if(version1 && name=="row" && withinrow && !withinkey)
	{
		withinrow=false;
		if(tkwidth > kwidth)
			kwidth = tkwidth;
		tkwidth = 0.0;
	//	lastbutton->rightborder = false;
		vd += lastrowscale + horizgap;
	}
	else
		if(version1 && name=="key" && withinkey)
		{
			withinkey = false;
		}
		else
			if(version1 && name=="value" && withinvalue)
			{
				withinvalue = false;
			}
	/*		else
				if(version1 && name=="layout" && !withinrow)
				{
					for(int i=0;i<lastrow.size();i++)
					{
						lastrow[i]->botborder=false;
					}
				}
				else
				{
					QMessageBox::critical(0,"Parse Error",name+": Unknown end element or end element not within correct structure");
					exit(1);
				} */
    return true;
}

bool MyParser::warning (const QXmlParseException & exception) 
{
	//std::cout << errorString() << std::endl;
	QMessageBox::critical(0,"TypeFaster: Parse Warning",exception.message().latin1());
	//don't bother translating
	exit(1);
}
bool MyParser::error (const QXmlParseException & exception) 
{
	//std::cout << errorString() << std::endl;
	QMessageBox::critical(0,"TypeFaster: Parse Error",errorString()+" "+exception.message().latin1());
	//don't bother translating
	exit(1);
}
bool MyParser::fatalError (const QXmlParseException & exception)
{
	//std::cout << errorString().latin1() << std::endl;
	QMessageBox::critical(0,"TypeFaster: Parse Fatal Error",errorString()+", "+exception.message().latin1());
	//don't bother translating
	exit(1);
}
