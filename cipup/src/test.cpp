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

#include <iostream>
#include <exception>
//#include <cstdlib>
//#include <time.h>
#include "cipup.hpp"
using namespace std;

int main(void) {
	puts("Welcome to CIPUP");
	cout << "Cipup Is Privacy for the Ultra Paranoid!" << endl;

	VersionText();

	// Simple Pause
	cout << "Press Enter to continue . . .\n";
	getchar();

	return EXIT_SUCCESS;
}
