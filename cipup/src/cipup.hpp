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

#using <mscorlib.dll>
using namespace System;
//using namespace System::Collections;

namespace cipup {

	char VERSION_NUMBER[] = "0.003";

	// This class is exported from the cipup.dll
	public ref class encrypt_engine {
	public:
		encrypt_engine(void);
		// TODO: add your methods here.
	};

	String^ GetVersionText(void);

	void PrintVersionText(void);

} //namespace cipup
