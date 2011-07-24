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

#include "..\..\cipup\src\cipup.hpp"
#include "..\..\cipup\src\engine_internal.hpp"
//#include "..\..\cipup\bitstream\Bitstream.imp.h"
//#include "..\..\cipup\prng\ecrypt-sync.h"

#include "stdafx.h"

#using <mscorlib.dll>
//using namespace System;
using namespace NUnit::Framework;

// This class is exported from the cipup.internal.test.dll

namespace cipup_internal_test {

	[TestFixture]
	public ref class test {
	public:
		test(void);
		[Test]
		static void BitstreamTest();
		[Test]
		static void TreeTest();
		[Test]
		static void GenGearPresetTest();
		[Test]
		static void GenGearRandTest();

		static void GenGearTest(uint8* key, uint8* iv);
		static void PrintGears(cipup::huffman_gear** acpGears);

		static array<uint8>^ StreamToStreamTest(cipup::InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest);
		static array<uint8>^ StreamFlowOpToStreamTest(cipup::InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest);
		static array<uint8>^ VectorToStreamTest(cipup::InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest);
		static array<uint8>^ VectorFlowOpToStreamTest(cipup::InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest);
		static array<uint8>^ SingleToStreamTest(cipup::InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest);
		static array<uint8>^ SingleFlowOpToStreamTest(cipup::InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest);

		[Test]
		static void StreamToStreamEncryptRandKeyRandIVRandMessageTest();
		[Test]
		static void StreamToStreamEncryptRandKeyRandIVTest();
		[Test]
		static void StreamToStreamEncryptRandIVTest();
		[Test]
		static void StreamToStreamEncryptTest();
		[Test]
		static void StreamToStreamRandKeyRandIVRandMessageFullTest();
		[Test]
		static void StreamToStreamRandKeyRandIVFullTest();
		[Test]
		static void StreamToStreamRandIVFullTest();
		[Test]
		static void StreamToStreamFullTest();

		[Test]
		static void StreamFlowOpToStreamFullTest();
		[Test]
		static void VectorToStreamFullTest();
		[Test]
		static void VectorFlowOpToStreamFullTest();
		[Test]
		static void SingleToStreamFullTest();
		[Test]
		static void SingleFlowOpToStreamFullTest();

	};

} //namespace cipup_internal_test
