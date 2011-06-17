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

// cipup.cpp : Defines the exported functions for the DLL application.
//

#include <iostream>
#include <exception>
#include <cstdlib>
//#include <time.h>
#include "cipup.hpp"
#include "engine_internal.hpp"
#include "..\prng\ecrypt-sync.h"

using namespace std;
using namespace Costella::Bitstream;

namespace cipup {

	static char* VERSION_NUMBER = "0.003";

	String^ GetVersionText()
	{
		return String::Concat("CIPUP v", gcnew String(VERSION_NUMBER));
	}

	void PrintVersionText()
	{
		Console::Write( String::Concat("CIPUP v", gcnew String(VERSION_NUMBER)) );
	}


	engine_internal::engine_internal()
	{
		eStatus = NotReady;
		eInitType = NotInit;
		eDestType = DestNone;
		bsBitBufferIn = new Out<>(ssBitBufferOut);
		ui64BitsWritten = 0;
		ui64BytesRead = 0;
	}
	engine_internal::~engine_internal()
	{
		delete bsBitBufferIn;
	}

	// This is the constructor of a class that has been exported.
	// see cipup.h for the class definition
	engine::engine()
	{
		cpInternal = new engine_internal();

		return;
	}

	engine::~engine()
	{
		finalize();

		delete cpInternal;
		return;
	}

	bool engine::canread()
	{
		return ( ( cpInternal->eStatus == Ready ) && ( cpInternal->eInitType == InitRead ) );
	}
	bool engine::canwrite()
	{
		return ( ( cpInternal->eStatus != NotReady ) && ( cpInternal->eInitType == InitWrite ) );
	}
	bool engine::ready()
	{
		return ( ( cpInternal->eStatus != NotReady ) && ( cpInternal->eInitType != NotInit ) );
	}

	uint64 engine::bytesread()
	{
		return cpInternal->ui64BytesRead;
	}

	uint64 engine::bitswritten()
	{
		return cpInternal->ui64BitsWritten;
	}
	uint64 engine::byteswritten()
	{
		return (cpInternal->ui64BitsWritten)/8;
	}
	uint8 engine::underflowbits()
	{
		return (uint8)((cpInternal->ui64BitsWritten)%8);
	}

	MessageCode engine::init( InitAction action, ostringstream* output, uint8* key, uint8 keybytelen, uint8* iv, uint8 ivbytelen )
	{
		if ( cpInternal->eStatus == NotReady )
		{
			if ( action == InitDecrypt )
			{

				//Build PRNG & HE

				cpInternal->eInitType = InitRead;
			}
			else if ( action == InitEncrypt )
			{
				
				//Build PRNG & HE

				//Clear bitstream
				cpInternal->bsBitBufferIn.flush();
				cpInternal->ssBitBufferOut.str( string() );
				cpInternal->ssBitBufferOut.clear();

				//Prepare output stream for cyphertext
				output->str( string() );
				output->clear();

				cpInternal->eInitType = InitWrite;
			}
			else
			{
				return InitFailureInvalidAction;
			}

			cpInternal->eDestType = DestStream;
			cpInternal->output = output;

			cpInternal->ui64BitsWritten = 0;
			cpInternal->ui64BytesRead = 0;

		}
		else
		{
			return InitFailureNeedFinalize;
		}

		//Set status
		cpInternal->eStatus = Ready;

		return InitSuccess;		
	}

	MessageCode engine::init( InitAction action, const char* filename, bool append, uint8* key, uint8 keybytelen, uint8* iv, uint8 ivbytelen )
	{
		if ( cpInternal->eStatus == NotReady )
		{
			if ( action == InitDecrypt )
			{
				cpInternal->output = new ofstream();
				if ( append )
				{
					cpInternal->output->open(filename, ios::out | ios::app);
				}
				else
				{
					cpInternal->output->open(filename);
				}

				if ( cpInternal->output->fail() )
				{
					return InitFailureFileError;
				}

				//Build PRNG & HE

				cpInternal->eInitType = InitRead;
			}
			else if ( action == InitEncrypt )
			{

				cpInternal->output = new ofstream();
				cpInternal->output->open(filename);
				
				if ( cpInternal->output->fail() )
				{
					return InitFailureFileError;
				}

				//Build PRNG & HE

				//Clear bitstream
				cpInternal->bsBitBufferIn.flush();
				cpInternal->ssBitBufferOut.str( string() );
				cpInternal->ssBitBufferOut.clear();

				cpInternal->eInitType = InitWrite;
			}
			else
			{
				return InitFailureInvalidAction;
			}

			cpInternal->eDestType = DestFile;

			cpInternal->ui64BitsWritten = 0;
			cpInternal->ui64BytesRead = 0;

		}
		else
		{
			return InitFailureNeedFinalize;
		}

		//Set status
		cpInternal->eStatus = Ready;

		return InitSuccess;		
	}
	
	void engine::decrypt( const istream& input )
	{
		if ( cpInternal->eInitType == InitRead )
		{
			if ( cpInternal->eStatus == Ready )
			{

			}
		}
	}
	void engine::operator>> ( const istream& input )
	{
		decrypt(input);
	}	


	void engine::encrypt( const uint8& datum )
	{
		if ( cpInternal->eInitType == InitWrite )
		{
			if ( cpInternal->eStatus == SemiReady )
			{
				//Clear output stream
				cpInternal->output->str( string() );
				cpInternal->output->clear();
				//Reset size
				cpInternal->ui64BitsWritten = 0;
				//Set status
				cpInternal->eStatus = Ready;
			}

			if ( cpInternal->eStatus == Ready )
			{

			}
		}

	}
	void engine::operator<< ( const uint8& datum )
	{
		encrypt(datum);
	}	

	void engine::encrypt( const std::vector< uint8 >& data )
	{
		for (uint32 i=0; i<data.size(); i++)
			encrypt(data.at(i));
	}
	void engine::operator<< ( const std::vector< uint8 >& data )
	{
		encrypt(data);
	}	

	void flush()
	{
		if ( cpInternal->eInitType == InitWrite )
		{
			if ( cpInternal->eDestType == DestStream )
			{
				if ( cpInternal->eStatus == Ready )
				{

					//cpInternal->bsBitBufferIn.flush();


					//cpInternal->ssBitBufferOut.str( string() );
					//cpInternal->ssBitBufferOut.clear();

					//Set status
					cpInternal->eStatus = SemiReady;
				}
			}
		}

	}

	void finalize()
	{
		if ( cpInternal->eStatus != NotReady )
		{
			if ( cpInternal->eInitType == InitRead )
			{
				if ( cpInternal->eDestType == DestFile )
				{
					((ofstream*)(cpInternal->output))->close();
					delete ((ofstream*)(cpInternal->output));
				}
			}
			else if ( cpInternal->eInitType == InitWrite )
			{
				if ( cpInternal->eStatus == Ready )
				{
					flush();
				}

				if ( cpInternal->eDestType == DestFile )
				{
					//Rewind and Write size
				
					((ofstream*)(cpInternal->output))->close();
					delete ((ofstream*)(cpInternal->output));
				}
				//else if ( cpInternal->eDestType == DestStream ) { }

			}

			cpInternal->eStatus = NotReady;

			//Destroy PRNG & HE
		}
	}

} //namespace cipup