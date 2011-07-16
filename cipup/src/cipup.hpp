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

#include <string>
#include <vector>
#include "stdafx.h"

#using <mscorlib.dll>
using namespace System;
using namespace std;
//using namespace System::Collections;

namespace cipup {

	enum MessageCode { InitSuccess = 0, InitFailure, InitFailureInvalidAction, InitFailureNeedFinalize, InitFailureFileError };

	static const string Messages[5] = { "Init Successful", "Init Failure", "Init Failure: Invalid Action", "Init Failure: Already initialized, call finalize", "Init Failure: File access error" };


	enum InitAction { InitDecrypt = 0, InitEncrypt };


	//Static functions
	String^ GetVersionText(void);

	void PrintVersionText(void);

	//Key helper functions
	//genKey(int len)


	// Non-exposed private data members and functions
	class engine_internal;


	// This class is exported from the cipup.dll
	public ref class engine {
	public:
		engine(void);
		~engine(void);

		//Output designates destination
		MessageCode init( InitAction action, ostringstream* output, uint8* key, uint8 keybytelen, uint8* iv, uint8 ivbytelen );
		//Overwrites file on Encrypt, append only applys to decrypting destination
		MessageCode init( InitAction action, const char* filename, bool append, uint8* key, uint8 keybytelen, uint8* iv, uint8 ivbytelen );
		bool ready();
		
		bool canread();
		//Consumes entire stream
		void decrypt( istream& input );
		void operator>> ( istream& input );

		uint64 bytesread();

		bool canwrite();
		void encrypt( const uint8& datum );
		void operator<< ( const uint8& datum );
		void encrypt( const std::vector< uint8 >& data );
		void operator<< ( const std::vector< uint8 >& data );
		void flush();

		uint64 bitswritten();
		uint64 byteswritten();
		uint8 underflowbits();

		void finalize();
		
	private:
		engine_internal* cpInternal;
	};


} //namespace cipup
