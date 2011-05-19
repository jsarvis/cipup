/*=============================================================
   Copyright 2009-2011 Jacob Sarvis

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

#using <mscorlib.dll>
using namespace System;
using namespace NUnit::Framework;

// This class is exported from the cipup.internal.test.dll

namespace cipup_internal_test {

	[TestFixture]
	public ref class test {
	public:
		test(void);
		[Test]
		void GetVersionTextTest();
		[Test]
		void PrintVersionTextTest();
		// TODO: add your methods here.
	};

} //namespace cipup_internal_test
