
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

#include <iostream>
#include <fstream>
#include <sstream>
#include "cipupCLT.hpp"

using namespace cipup;
using namespace std;
using namespace System;
#using <mscorlib.dll>

namespace cipupCLT {

	/*
		cipupCLT Action Piping keyFile ivFile [InFile] [OutFile]
		Action = e, d
		Piping = f2f, f2c, c2f, c2c

		cipupCLT e f2f keyfile ivfile InFile OutFile
		cipupCLT d f2c keyfile ivfile InFile
		cipupCLT d c2f keyfile ivfile OutFile
		cipupCLT d c2c keyfile ivfile
	*/
	int main(array<String ^> ^args)
	{
		if ( args->Length == 6 )
		{
			if ( args[0] == "e" )
			{
				if ( args[1] == "f2f" )
				{
					return FileToFile(InitAction::InitEncrypt, args[2], args[3], args[4], args[5]);
				}
				else if ( args[1] == "f2c" || args[1] == "c2f" )
				{
					PrintHelp("Piping does not match arguement count. Expected 5 arguements.");
					return -1;
				}
				else if ( args[1] == "c2c" )
				{
					PrintHelp("Piping does not match arguement count. Expected 4 arguements.");
					return -1;
				}
				else
				{
					PrintHelp("Invalid Piping: "+args[1]+"! Valid choices are (f2f,f2c,c2f,c2c).");
					return -1;
				}
			}
			else if ( args[0] == "d" )
			{
				if ( args[1] == "f2f" )
				{
					return FileToFile(InitAction::InitDecrypt, args[2], args[3], args[4], args[5]);
				}
				else if ( args[1] == "f2c" || args[1] == "c2f" )
				{
					PrintHelp("Piping does not match arguement count. Expected 5 arguements.");
					return -1;
				}
				else if ( args[1] == "c2c" )
				{
					PrintHelp("Piping does not match arguement count. Expected 4 arguements.");
					return -1;
				}
				else
				{
					PrintHelp("Invalid Piping: "+args[1]+"! Valid choices are (f2f,f2c,c2f,c2c).");
					return -1;
				}
				
			}
			else
			{
				PrintHelp("Invalid Action: "+args[0]+"! Valid choices are (e,d).");
				return -1;
			}
		}
		else if ( args->Length == 5 )
		{
			if ( args[0] == "e" )
			{
				if ( args[1] == "f2c" )
				{
					return FileToConsole(InitAction::InitEncrypt, args[2], args[3], args[4]);
				}
				else if ( args[1] == "c2f" )
				{
					return ConsoleToFile(InitAction::InitEncrypt, args[2], args[3], args[4]);
				}
				else if ( args[1] == "f2f" )
				{						
					PrintHelp("Piping does not match arguement count. Expected 6 arguements.");
					return -1;
				}
				else if ( args[1] == "c2c" )
				{
					PrintHelp("Piping does not match arguement count. Expected 4 arguements.");
					return -1;
				}
				else
				{
					PrintHelp("Invalid Piping: "+args[1]+"! Valid choices are (f2f,f2c,c2f,c2c).");
					return -1;
				}
			}
			else if ( args[0] == "d" )
			{
				if ( args[1] == "f2c" )
				{
					return FileToConsole(InitAction::InitDecrypt, args[2], args[3], args[4]);
				}
				else if ( args[1] == "c2f" )
				{
					return ConsoleToFile(InitAction::InitDecrypt, args[2], args[3], args[4]);
				}
				else if ( args[1] == "f2f" )
				{						
					PrintHelp("Piping does not match arguement count. Expected 6 arguements.");
					return -1;
				}
				else if ( args[1] == "c2c" )
				{
					PrintHelp("Piping does not match arguement count. Expected 4 arguements.");
					return -1;
				}
				else
				{
					PrintHelp("Invalid Piping: "+args[1]+"! Valid choices are (f2f,f2c,c2f,c2c).");
					return -1;
				}
			}
			else
			{
				PrintHelp("Invalid Action: "+args[0]+"! Valid choices are (e,d).");
				return -1;
			}
		}
		else if ( args->Length == 4 )
		{
			if ( args[0] == "e" )
			{
				if ( args[1] == "c2c" )
				{
					return ConsoleToConsole(InitAction::InitEncrypt, args[2], args[3]);
				}
				else if ( args[1] == "f2c" || args[1] == "c2f" )
				{
					PrintHelp("Piping does not match arguement count. Expected 5 arguements.");
					return -1;
				}
				else if ( args[1] == "f2f" )
				{
					PrintHelp("Piping does not match arguement count. Expected 6 arguements.");
					return -1;
				}
				else
				{
					PrintHelp("Invalid Piping: "+args[1]+"! Valid choices are (f2f,f2c,c2f,c2c).");
					return -1;
				}
			}
			else if ( args[0] == "d" )
			{
				if ( args[1] == "c2c" )
				{
					return ConsoleToConsole(InitAction::InitDecrypt, args[2], args[3]);
				}
				else if ( args[1] == "f2c" || args[1] == "c2f" )
				{
					PrintHelp("Piping does not match arguement count. Expected 5 arguements.");
					return -1;
				}
				else if ( args[1] == "f2f" )
				{
					PrintHelp("Piping does not match arguement count. Expected 6 arguements.");
					return -1;
				}
				else
				{
					PrintHelp("Invalid Piping: "+args[1]+"! Valid choices are (f2f,f2c,c2f,c2c).");
					return -1;
				}					
			}
			else
			{
				PrintHelp("Invalid Action: "+args[0]+"! Valid choices are (e,d).");
				return -1;
			}
		}
		else if ( args->Length > 6 )
		{
			PrintHelp("Too many arguements specified.");
			return -1;
		}
		else //if ( args->Length < 4 )
		{
			PrintHelp("Too few arguements specified.");
			return -1;
		}
	}

	void PrintHelp(String^ specialMessage)
	{
		Console::WriteLine("Cipup Command Line Tool");
		Console::WriteLine(engine::GetVersionText());
		Console::WriteLine(engine::GetConfiguration());
		Console::WriteLine();
		Console::WriteLine("Arugements: Action Piping KeyFile IVFile [InFile] [OutFile]");
		Console::WriteLine("Action: e = Encrypt");
		Console::WriteLine("        d = Decrypt");
		Console::WriteLine("Piping: f2f = File to File, requires both InFile and OutFile");
		Console::WriteLine("        f2c = File to Console, requires InFile");
		Console::WriteLine("        c2f = Console to File, requires OutFile");
		Console::WriteLine("        c2c = Console to Console");
		Console::WriteLine("KeyFile: Path specifying a file that contains at least "+engine::RequiredKeyByteLength()+" bytes");
		Console::WriteLine("IVFile: Path specifying a file that contains at least "+engine::RequiredIVByteLength()+" bytes");
		Console::WriteLine("InFile: Optional arguement, Path specifing file to read from");
		Console::WriteLine("OutFile: Optional arguement, Path specifing file to write to");
		Console::WriteLine();
		Console::WriteLine("Examples:");
		Console::WriteLine("cipupCLT e f2f \"myKey\" \"myIV\" \"PlainText.txt\" \"Cryptext.out\"");
		Console::WriteLine("cipupCLT d f2c \"myKey\" \"myIV\" \"Cryptext.out\"");
		Console::WriteLine("cipupCLT e c2f \"myKey\" \"myIV\" \"Cryptext.out\"");
		Console::WriteLine("cipupCLT d c2c \"myKey\" \"myIV\"");
		Console::WriteLine();
		Console::WriteLine(specialMessage);
	}

	int ReadFile(String^ file, uint8* buffer, uint8 length)
	{
		IntPtr filename = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(file);
		ifstream fileptr ((char*)(void*)filename);
		Runtime::InteropServices::Marshal::FreeHGlobal(filename);
		if (fileptr.is_open())
		{
			fileptr.read((char*)buffer, length);
		
			fileptr.close();

			if (fileptr.gcount() != length)
			{
				PrintHelp("Could not read "+length+" bytes from file "+file);
				return -1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			PrintHelp("Could not open file "+file);
			return -1;
		}		
	}

	int FileToFile(InitAction action, String^ keyFile, String^ ivFile, String^ inFile, String^ outFile)
	{
		uint8 keylen = engine::RequiredKeyByteLength();
		uint8 ivlen = engine::RequiredIVByteLength();
		uint8* key = new uint8[keylen];
		uint8* iv = new uint8[ivlen];

		int status;

		status = ReadFile(keyFile, key, keylen);
		if ( status != 0 )
		{
			delete[] key;
			delete[] iv;
			return status;
		}

		status = ReadFile(ivFile, iv, ivlen);
		if ( status != 0 )
		{
			delete[] key;
			delete[] iv;
			return status;
		}

		IntPtr infilename = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(inFile);
		ifstream fileptr ((char*)(void*)infilename);
		Runtime::InteropServices::Marshal::FreeHGlobal(infilename);
		if (!fileptr.is_open())
		{
			PrintHelp("Could not open file "+inFile);
			return -1;
		}		

		MessageCode statusCode;

        engine^ alpha = gcnew engine();

		IntPtr outfilename = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(outFile);

        statusCode = alpha->init(action, (char*)(void*)outfilename, false, key, keylen, iv, ivlen);

		Runtime::InteropServices::Marshal::FreeHGlobal(outfilename);

		if ( statusCode != MessageCode::InitSuccess )
		{
			Console::WriteLine("Error! Init returned: "+engine::Messages[(int)statusCode]);
			delete[] key;
			delete[] iv;
			fileptr.close();
			return -1;
		}

		if ( action == InitAction::InitEncrypt )
        {
            alpha->encrypt(*((basic_istream<char,char_traits<char>>*)&fileptr));
            alpha->finalize();
        }
        else if (action == InitAction::InitDecrypt)
        {
            alpha->decrypt(*((istream*)&fileptr));
            alpha->finalize();
        }

		fileptr.close();

		delete[] key;
		delete[] iv;
		return 0;
	}

	int FileToConsole(InitAction action, String^ keyFile, String^ ivFile, String^ inFile)
	{
		uint8 keylen = engine::RequiredKeyByteLength();
		uint8 ivlen = engine::RequiredIVByteLength();
		uint8* key = new uint8[keylen];
		uint8* iv = new uint8[ivlen];

		int status;

		status = ReadFile(keyFile, key, keylen);
		if ( status != 0 )
		{
			delete[] key;
			delete[] iv;
			return status;
		}

		status = ReadFile(ivFile, iv, ivlen);
		if ( status != 0 )
		{
			delete[] key;
			delete[] iv;
			return status;
		}

		IntPtr infilename = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(inFile);
		ifstream fileptr ((char*)(void*)infilename);
		Runtime::InteropServices::Marshal::FreeHGlobal(infilename);
		if (!fileptr.is_open())
		{
			PrintHelp("Could not open file "+inFile);
			return -1;
		}
		
		MessageCode statusCode;

        engine^ alpha = gcnew engine();

		stringstream* outputBuffer = new stringstream();

        statusCode = alpha->init(action, (ostringstream*)outputBuffer, key, keylen, iv, ivlen);

		if ( statusCode != MessageCode::InitSuccess )
		{
			Console::WriteLine("Error! Init returned: "+engine::Messages[(int)statusCode]);
			delete[] key;
			delete[] iv;
			delete outputBuffer;
			fileptr.close();
			return -1;
		}

		if ( action == InitAction::InitEncrypt )
        {
            alpha->encrypt(*((istream*)&fileptr));
            alpha->finalize();

			if ( outputBuffer->good() )
			{
				cout << outputBuffer->rdbuf();
			}

        }
        else if (action == InitAction::InitDecrypt)
        {
            alpha->decrypt(*((istream*)&fileptr));
            alpha->finalize();

			if ( outputBuffer->good() )
			{
				cout << outputBuffer->rdbuf();
			}
        }

		delete outputBuffer;

		fileptr.close();

		delete[] key;
		delete[] iv;
		return 0;
	}

	int ConsoleToFile(InitAction action, String^ keyFile, String^ ivFile, String^ outFile)
	{
		uint8 keylen = engine::RequiredKeyByteLength();
		uint8 ivlen = engine::RequiredIVByteLength();
		uint8* key = new uint8[keylen];
		uint8* iv = new uint8[ivlen];

		int status;

		status = ReadFile(keyFile, key, keylen);
		if ( status != 0 )
		{
			delete[] key;
			delete[] iv;
			return status;
		}

		status = ReadFile(ivFile, iv, ivlen);
		if ( status != 0 )
		{
			delete[] key;
			delete[] iv;
			return status;
		}

		MessageCode statusCode;

        engine^ alpha = gcnew engine();

		IntPtr outfilename = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(outFile);

        statusCode = alpha->init(action, (char*)(void*)outfilename, false, key, keylen, iv, ivlen);

		Runtime::InteropServices::Marshal::FreeHGlobal(outfilename);

		if ( statusCode != MessageCode::InitSuccess )
		{
			Console::WriteLine("Error! Init returned: "+engine::Messages[(int)statusCode]);
			delete[] key;
			delete[] iv;
			return -1;
		}

		if ( action == InitAction::InitEncrypt )
        {
			int input;
			while ( (input=Console::Read()) != -1 )
			{
				alpha->encrypt(Convert::ToByte(input));
			}

            alpha->finalize();
        }
        else if (action == InitAction::InitDecrypt)
        {
			stringstream* inputBuffer = new stringstream();

			int input;
			while ( (input=Console::Read()) != -1 )
			{
				*inputBuffer << Convert::ToByte(input);
			}

            alpha->decrypt(*inputBuffer);
            alpha->finalize();

			delete inputBuffer;
        }

		delete[] key;
		delete[] iv;
		return 0;
	}

	int ConsoleToConsole(InitAction action, String^ keyFile, String^ ivFile)
	{
		uint8 keylen = engine::RequiredKeyByteLength();
		uint8 ivlen = engine::RequiredIVByteLength();
		uint8* key = new uint8[keylen];
		uint8* iv = new uint8[ivlen];

		int status;

		status = ReadFile(keyFile, key, keylen);
		if ( status != 0 )
		{
			delete[] key;
			delete[] iv;
			return status;
		}

		status = ReadFile(ivFile, iv, ivlen);
		if ( status != 0 )
		{
			delete[] key;
			delete[] iv;
			return status;
		}

		MessageCode statusCode;

        engine^ alpha = gcnew engine();

		stringstream* outputBuffer = new stringstream();

        statusCode = alpha->init(action, (ostringstream*)outputBuffer, key, keylen, iv, ivlen);

		if ( statusCode != MessageCode::InitSuccess )
		{
			Console::WriteLine("Error! Init returned: "+engine::Messages[(int)statusCode]);
			delete[] key;
			delete[] iv;
			delete outputBuffer;
			return -1;
		}

		if ( action == InitAction::InitEncrypt )
        {
			int input;
			while ( (input=Console::Read()) != -1 )
			{
				alpha->encrypt(Convert::ToByte(input));
			}

            alpha->finalize();

			if ( outputBuffer->good() )
			{
				cout << outputBuffer->rdbuf();
			}
        }
        else if (action == InitAction::InitDecrypt)
        {
			stringstream* inputBuffer = new stringstream();

			int input;
			while ( (input=Console::Read()) != -1 )
			{
				*inputBuffer << Convert::ToByte(input);
			}

            alpha->decrypt(*inputBuffer);
            alpha->finalize();

			delete inputBuffer;

			if ( outputBuffer->good() )
			{
				cout << outputBuffer->rdbuf();
			}
        }

		delete outputBuffer;

		delete[] key;
		delete[] iv;
		return 0;
	}

} //namespace cipupCLT

