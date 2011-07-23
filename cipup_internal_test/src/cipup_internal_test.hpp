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

//#include "..\..\cipup\src\cipup.hpp"
#include "..\..\cipup\src\engine_internal.hpp"
//#include "..\..\cipup\bitstream\Bitstream.imp.h"
//#include "..\..\cipup\prng\ecrypt-sync.h"

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
		void BitstreamTest();
		[Test]
		void TreeTest();
		[Test]
		void GenGearPresetTest();
		[Test]
		void GenGearRandTest();
		void GenGearTest(uint8* key, uint8* iv);
		void PrintGears(cipup::huffman_gear** acpGears);
	};

} //namespace cipup_internal_test
