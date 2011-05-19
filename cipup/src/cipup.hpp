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

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CIPUP_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CIPUP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CIPUP_EXPORTS
#define CIPUP_API __declspec(dllexport)
#else
#define CIPUP_API __declspec(dllimport)
#endif

#include <string>

using namespace std;

namespace cipup {

	const string VERSION_NUMBER = "0.003";

	// This class is exported from the cipup.dll
	class CIPUP_API cipup_encrypt_engine {
	public:
		cipup_encrypt_engine(void);
		// TODO: add your methods here.
	};

	//extern CIPUP_API int ncipup;

	//CIPUP_API int fncipup(void);

	CIPUP_API string GetVersionText(void);

	CIPUP_API void PrintVersionText(void);

} //namespace cipup