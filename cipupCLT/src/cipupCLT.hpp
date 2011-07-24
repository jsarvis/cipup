/*=============================================================
   Copyright 2011 Jacob Sarvis

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

#include "stdafx.h"

#using <mscorlib.dll>

namespace cipupCLT {

	int main(array<System::String ^> ^args);

	void PrintHelp(System::String^ specialMessage);

	int ReadFile(System::String^ file, uint8* buffer, uint8 length);

	int FileToFile(cipup::InitAction action, System::String^ keyFile, System::String^ ivFile, System::String^ inFile, System::String^ outFile);
	int FileToConsole(cipup::InitAction action, System::String^ keyFile, System::String^ ivFile, System::String^ inFile);
	int ConsoleToFile(cipup::InitAction action, System::String^ keyFile, System::String^ ivFile, System::String^ outFile);
	int ConsoleToConsole(cipup::InitAction action, System::String^ keyFile, System::String^ ivFile);
	
} //namespace cipupCLT
