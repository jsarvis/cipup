
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
#include "cipup_internal_test.hpp"

using namespace std;

namespace cipup_internal_test {

	// This is the constructor of a class that has been exported.
	// see cipup.internal.test.h for the class definition
	test::test()
	{
		return;
	}

	void test::GetVersionTextTest()
	{
		Console::WriteLine( cipup::GetVersionText() );
		//Assert::AreEqual( );
	}

	void test::PrintVersionTextTest()
	{
		cipup::PrintVersionText();
	}

} //namespace cipup_internal_test

