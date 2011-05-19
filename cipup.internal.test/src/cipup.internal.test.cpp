
/*=============================================================
   Copyright 2009 Jacob Sarvis

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
================================================================*/
// cipup.internal.test.cpp : Defines the exported functions for the DLL application.
//


#include <iostream>
#include <exception>
//#include <cstdlib>
//#include <time.h>
#include "stdafx.h"
#include "cipup.internal.test.hpp"

#include "..\..\cipup\src\cipup.hpp"

using namespace std;

// This is an example of an exported variable
//CIPUPINTERNALTEST_API int ncipupinternaltest=0;

// This is an example of an exported function.
/*CIPUPINTERNALTEST_API int fncipupinternaltest(void)
{
	return 42;
}*/

// This is the constructor of a class that has been exported.
// see cipup.internal.test.h for the class definition
Ccipupinternaltest::Ccipupinternaltest()
{
	return;
}

bool Ccipupinternaltest::GetVersionTextTest()
{
	cout << "GetVersionTextTest" << endl;
	string temp;
	temp = cipup::GetVersionText();
	cout << temp << endl;
	return true;
}

bool Ccipupinternaltest::PrintVersionTextTest()
{
	cout << "PrintVersionTextTest" << endl;
	cipup::PrintVersionText();
	return true;
}

