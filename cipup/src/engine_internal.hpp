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


#include "..\bitstream\Bitstream.imp.h"


#using <mscorlib.dll>
using namespace Costella::Bitstream;
using namespace System;

namespace cipup {
	
	enum Status { NotReady = 0, Ready, SemiReady };
	enum InitType { NotInit = 0, InitRead, InitWrite };
	enum DestType { DestNone = 0, DestStream, DestFile };
	
	private class engine_internal {
	public:
		engine_internal();
		~engine_internal();

		Status eStatus;
		InitType eInitType;
		DestType eDestType;

		//HuffmanEngine
		stringstream ssBitBufferOut;
		Out<>* bsBitBufferIn;
		//RabbitPRNG

		ostream* output;

		uint64 ui64BitsWritten;
		uint64 ui64BytesRead;

	};


} //namespace cipup
