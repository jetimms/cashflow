//  Copyright 2014 Jason Eric Timms
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
//  cashflow.hpp
//   Main header file for the cashflow program.

#ifndef _CASHFLOW_HPP_
  #define _CASHFLOW_HPP_
  #include <QString>
  
  const QString VERSION = "0.24.0";
	const QString ABOUT_NAME_AND_VERSION =
		"<h1>cashflow v" + VERSION + "</h1>";
  const QString ABOUT_COPYRIGHT_TEXT =
		"<p>Copyright &copy; 2014 Jason Eric Timms</p>";
  const QString ABOUT_APP_DESCRIPTION =
    "<p>cashflow is a zero-sum budget program.</p>";
  
  #ifndef ATLINE
    #define ATLINE QString(__FILE__) + "(" + QString("%1").arg(__LINE__) + ")"
  #endif // ATLINE

#endif // _CASHFLOW_HPP_
