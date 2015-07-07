/***************************************************************************
 *   Copyright (C) 2014-2015 by Chris Phillips                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*===========================================================================
 * SVN properties (DO NOT CHANGE)
 *
 * $Id$
 * $HeadURL: https://svn.atnf.csiro.au/difx/applications/vex2difx/branches/multidatastream/src/vex2difx.cpp $
 * $LastChangedRevision$
 * $Author$
 * $LastChangedDate$
 *
 *==========================================================================*/

#include <iostream>
#include <sstream>
#include "parserhelp.h"

enum charType whatChar(const char a)
{
    if (a=='+'||a=='-') 
      return SIGN;
    else if (a=='E'||a=='e')
      return (E);
    else if (a>='0'&&a<='9')
      return DIGIT;
    else if (a==' ')
      return SPACE;
    else if (a=='.')
      return DOT;
    else
      return CHARERROR;
}

int getDouble(std::string &value, double &x)
{
    enum stateType {START, STARTINT, INTEGER, DECIMAL, STARTEXP, EXPONENT, END, ERROR};
    enum stateType state = START;
    enum charType what;

    unsigned int i;
    for (i=0 ; i<value.length(); i++) {
      what = whatChar(value[i]);
      
      switch (state) {
      case START:
	switch (what) {
	case CHARERROR:
	  std::cerr << "Error parsing character in \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	  value = "";
	  return 1; 
	  break;
	case SIGN:
	  state = STARTINT;
	  break;
	case DIGIT:
	  state = INTEGER;
	  break;
	case SPACE:
	  break;
	case E:
	  state = ERROR;
	  break;
	case DOT:
	  state=DECIMAL;
	  break;
	}
	break;
	
      case STARTINT:
	switch (what) {
	case CHARERROR:
	  std::cerr << "Error parsing character in \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	  value = "";
	  return 1; 
	  break;
	case SIGN:
	case E:
	  state = ERROR;
	  break;
	case DIGIT:
	  state = INTEGER;
	  break;
	case SPACE:
	  break;
	case DOT:
	  state = DECIMAL;
	}
	break;

      case INTEGER:
	switch (what) {
	case CHARERROR:
	  std::cerr << "Error parsing character in \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	  value = "";
	  return 1; 
	  break;
	case DIGIT:
	  break;
	case SIGN:
	case SPACE:
	  state = END;
	  break;
	case E:
	  state = STARTEXP;
	  break;
	case DOT:
	  state = DECIMAL;
	}
	break;
	
      case DECIMAL:
	switch (what) {
	case CHARERROR:
	  std::cerr << "Error parsing character in \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	  value = "";
	  return 1; 
	  break;
	case DIGIT:
	  break;
	case SIGN:
	case SPACE:
	  state = END;
	  break;
	case E:
	  state = STARTEXP;
	  break;
	case DOT:
	  state = ERROR;
	  break;
	}
	break;
	
      case STARTEXP:
	switch (what) {
	case CHARERROR:
	  std::cerr << "Error parsing character in \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	  value = "";
	  return 1; 
	  break;
	case SIGN:
	case DIGIT:
	  state = EXPONENT;
	  break;
	case SPACE:
	case E:
	case DOT:
	  state = ERROR;
	  break;
	}
	break;
	
      case EXPONENT:
	switch (what) {
	case CHARERROR:
	  std::cerr << "Error parsing character in \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	  value = "";
	  return 1; 
	  break;
	case SPACE:
	case SIGN:
	  state = END;
	  break;
	case DIGIT:
	  break;
	case DOT:
	case E:
	  state = ERROR;
	  break;
	}
	break;

      case ERROR:
      case END:
	break;
	
      }
      
      if (state==ERROR) {
	std::cerr << "Error parsing \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	value = "";
	return 1; 
      }
      if (state==END) break;
    }

    std::stringstream ss;
    if (state==START) {
      value = "";
      return 1;
    } else if (state==END) {
    } else {
      i = value.length();
    }
    ss << value.substr(0,i);
    ss >> x;
    value  = value.substr(i);

    return 0;
}
  
int getOp(std::string &value, int &plus) {
    enum charType what;

    unsigned int i;
    for (i=0 ; i<value.length(); i++) {
      what = whatChar(value[i]);
      
      if (what==CHARERROR) {
	std::cerr << "Error parsing character in \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	value = "";
	return 1; 
      } else if (what==SPACE) {
	continue;
      } else if (what==SIGN) {
	if (value[i]=='+') {
	  plus = 1;
	} else {
	  plus = 0;
	} 
	value = value.substr(i+1);
	return(0);
      } else {
	std::cerr << "Unexpected character in \"" << value << "\" at : '" << value[i] << "':" << i << std::endl;
	value = "";
	return 1; 
      }
    }
    return 1; // Did not match anything
  }

// Read a string consisting of a series of additions and subtrations (only) and return a double
double parseDouble(const std::string &value)
{
  std::string str = value; // Copy as the procedure destroys the string
  
  int status, number=1, sign=-1;
  double thisvalue, result=0;
  while (str.length()) {
    if (number) {
      status = getDouble(str, thisvalue);
      if (status) break;
      if (sign==-1)
	result = thisvalue;
      else if (sign==1) 
	result += thisvalue;
      else
	result -= thisvalue;
      number = 0;
    
    } else  {
      status = getOp(str, sign);
      if (status) break;
      number = 1;
    }
  }

  return result;
}
