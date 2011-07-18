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

// cipup.cpp
#include "cipup.hpp"
#include "engine_internal.hpp"

#include <iostream>
#include <fstream>
#include <exception>
#include <cstdlib>
#include <ctime>

using namespace System;
using namespace std;
using namespace Costella::Bitstream;

namespace cipup {

	static char* VERSION_NUMBER = "0.90";

	String^ engine::GetVersionText()
	{
		return String::Concat("CIPUP v", gcnew String(VERSION_NUMBER));
	}

	void engine::PrintVersionText()
	{
		Console::Write( GetVersionText() );
	}

	String^ engine::GetConfiguration()
	{
		stringstream retval;
#ifdef SHRINK_CYPHERTEXT
		retval << "Shrinked Cyphertext. ";
#endif

#ifdef RANDOM_LOTTERY
		retval << "Random Lottery. ";
#endif

#if KEYSTREAMBUFFERSIZE == 16
		retval << "Full buffer. ";
#elif KEYSTREAMBUFFERSIZE == 1
		retval << "Minimal buffer. ";
#endif

#if CHAR_SET_SIZE == 256
		retval << "Full byte character set. ";
#elif CHAR_SET_SIZE == 16
		retval << "Half-byte character set. ";
#endif
		return String::String(retval.str().c_str()).ToString();
	}

	void engine::PrintConfiguration()
	{
		Console::Write( GetConfiguration() );
	}

	uint8 engine::RequiredKeyByteLength(void)
	{
		return KEYSIZEINBYTES;
	}

	uint8 engine::RequiredIVByteLength(void)
	{
		return IVSIZEINBYTES;
	}

	void engine::GenerateKey(array<uint8>^ key, uint8 keybytelen)
	{
		GenerateKey(key, keybytelen, GenerationTechnique::LocalEntropicSecureRand);
	}

	void engine::GenerateKey(uint8*& key, uint8 keybytelen)
	{
		GenerateKey(key, keybytelen, GenerationTechnique::LocalEntropicSecureRand);
	}

	void engine::GenerateKey(array<uint8>^ key, uint8 keybytelen, GenerationTechnique techchoice)
	{
		if ( key == nullptr )
		{
			return;
		}

		//Convert the data type to unmanaged
		uint8* internalKey = new uint8[keybytelen];

		GenerateKey(internalKey, keybytelen, techchoice);

		//Push the data back into the managed type
		for ( uint8 itr = 0; itr < keybytelen; itr++ )
		{
			key[itr] = internalKey[itr];
		}
	}

	void engine::GenerateKey(uint8*& key, uint8 keybytelen, GenerationTechnique techchoice)
	{
		
		if ( key == NULL )
		{
			key = new uint8[keybytelen];
		}

		if ( techchoice == GenerationTechnique::LocalSimpleRand )
		{
			srand ( (uint32) time(NULL) );
			for ( keybytelen-- ; keybytelen >= 0; keybytelen-- )
			{
				key[keybytelen] = (uint8) ( ( ( (double)rand() / ( (double)UINT_MAX + 1.0) ) *  (double)UCHAR_MAX ) / 1 );
			}
		}
		else if ( techchoice == GenerationTechnique::LocalJumpingSimpleRand )
		{
			srand ( (uint32) time(NULL) );
			for ( keybytelen-- ; keybytelen >= 0; keybytelen-- )
			{
				key[keybytelen] = (uint8) ( ( ( (double)rand() / ( (double)UINT_MAX + 1.0) ) *  (double)UCHAR_MAX ) / 1 );
				srand ( rand() );
			}
		}
		else if ( techchoice == GenerationTechnique::LocalSecureRand )
		{
			uint32 number;
			for ( keybytelen-- ; keybytelen >= 0; keybytelen-- )
			{
				rand_s( &number );
				key[keybytelen] = (uint8) ( ( ( (double)number / ( (double)UINT_MAX + 1.0) ) *  (double)UCHAR_MAX ) / 1 );
			}
		}
		else if ( techchoice == GenerationTechnique::LocalEntropicSecureRand )
		{
			HCRYPTPROV hProvider;
			if (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
			{
				CryptGenRandom(hProvider, keybytelen, key);
			}
		}
	}

	engine::engine()
	{
		cpInternal = gcnew engine_internal();

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
	uint8 engine::overflowbits()
	{
		return (uint8)((cpInternal->ui64BitsWritten)%8);
	}

	MessageCode engine::init( InitAction action, IO::Stream^ output, array<uint8>^ key, uint8 keybytelen, array<uint8>^ iv, uint8 ivbytelen )
	{
		if ( cpInternal->eStatus == NotReady )
		{
			if ( keybytelen != KEYSIZEINBYTES )
			{
				return MessageCode::InitFailureInvalidKeyLength;
			}
			if ( ivbytelen != IVSIZEINBYTES )
			{
				return MessageCode::InitFailureInvalidIVLength;
			}

			//Convert the data type to unmanaged
			uint8* internalKey = new uint8[keybytelen];
			uint8* internalIV = new uint8[ivbytelen];
			for ( uint8 itr = 0; itr < keybytelen; itr++ )
			{
				internalKey[itr] = key[itr];
			}
			for ( uint8 itr = 0; itr < ivbytelen; itr++ )
			{
				internalIV[itr] = iv[itr];
			}

			if ( action == InitAction::InitDecrypt )
			{
				//Build PRNG
				cpInternal->RabbitPRNGState = new ECRYPT_ctx;
				ECRYPT_init();
				ECRYPT_keysetup(cpInternal->RabbitPRNGState, internalKey, keybytelen*8, ivbytelen*8); //bit size of key and iv
				ECRYPT_ivsetup(cpInternal->RabbitPRNGState, internalIV);

				//Build HuffmanEngine
				cpInternal->cpHuffmanGearbox = new huffman_gearbox( InitRead, cpInternal->RabbitPRNGState );

				cpInternal->eInitType = InitRead;
			}
			else if ( action == InitAction::InitEncrypt )
			{
				//Build PRNG
				cpInternal->RabbitPRNGState = new ECRYPT_ctx;
				ECRYPT_init();
				ECRYPT_keysetup(cpInternal->RabbitPRNGState, internalKey, keybytelen*8, ivbytelen*8); //bit size of key and iv
				ECRYPT_ivsetup(cpInternal->RabbitPRNGState, internalIV);

				//Build HuffmanEngine
				cpInternal->cpHuffmanGearbox = new huffman_gearbox( InitWrite, cpInternal->RabbitPRNGState );

				//Clear bitstream
				cpInternal->bsBitBufferIn->flush();
				cpInternal->ssBitBufferOut->str( string() );
				cpInternal->ssBitBufferOut->clear();

				cpInternal->eInitType = InitWrite;
			}
			else
			{
				return MessageCode::InitFailureInvalidAction;
			}

			cpInternal->eDestType = DestStream;

			cpInternal->output = output;

			cpInternal->ui64BitsWritten = 0;
			cpInternal->ui64BytesRead = 0;

		}
		else
		{
			return MessageCode::InitFailureNeedFinalize;
		}

		//Set status
		cpInternal->eStatus = Ready;

		return MessageCode::InitSuccess;
	}

	MessageCode engine::init( InitAction action, ostringstream* output, uint8* key, uint8 keybytelen, uint8* iv, uint8 ivbytelen )
	{
		if ( cpInternal->eStatus == NotReady )
		{
			if ( keybytelen != KEYSIZEINBYTES )
			{
				return MessageCode::InitFailureInvalidKeyLength;
			}
			if ( ivbytelen != IVSIZEINBYTES )
			{
				return MessageCode::InitFailureInvalidIVLength;
			}

			if ( action == InitAction::InitDecrypt )
			{
				//Build PRNG
				cpInternal->RabbitPRNGState = new ECRYPT_ctx;
				ECRYPT_init();
				ECRYPT_keysetup(cpInternal->RabbitPRNGState, key, keybytelen*8, ivbytelen*8); //bit size of key and iv
				ECRYPT_ivsetup(cpInternal->RabbitPRNGState, iv);

				//Build HuffmanEngine
				cpInternal->cpHuffmanGearbox = new huffman_gearbox( InitRead, cpInternal->RabbitPRNGState );

				cpInternal->eInitType = InitRead;
			}
			else if ( action == InitAction::InitEncrypt )
			{
				//Build PRNG
				cpInternal->RabbitPRNGState = new ECRYPT_ctx;
				ECRYPT_init();
				ECRYPT_keysetup(cpInternal->RabbitPRNGState, key, keybytelen*8, ivbytelen*8); //bit size of key and iv
				ECRYPT_ivsetup(cpInternal->RabbitPRNGState, iv);

				//Build HuffmanEngine
				cpInternal->cpHuffmanGearbox = new huffman_gearbox( InitWrite, cpInternal->RabbitPRNGState );

				//Clear bitstream
				cpInternal->bsBitBufferIn->flush();
				cpInternal->ssBitBufferOut->str( string() );
				cpInternal->ssBitBufferOut->clear();

				//Prepare output stream for cyphertext
				output->str( string() );
				output->clear();

				cpInternal->eInitType = InitWrite;
			}
			else
			{
				return MessageCode::InitFailureInvalidAction;
			}

			cpInternal->eDestType = DestStream;
			ostreamManagedWrapper^ tempWrapper = gcnew ostreamManagedWrapper(output);
			cpInternal->output = tempWrapper;
			cpInternal->outputWrapped = true;

			cpInternal->ui64BitsWritten = 0;
			cpInternal->ui64BytesRead = 0;

		}
		else
		{
			return MessageCode::InitFailureNeedFinalize;
		}

		//Set status
		cpInternal->eStatus = Ready;

		return MessageCode::InitSuccess;		
	}

	MessageCode engine::init( InitAction action, const char* filename, bool append, array<uint8>^ key, uint8 keybytelen, array<uint8>^ iv, uint8 ivbytelen )
	{
		if ( cpInternal->eStatus == NotReady )
		{
			if ( keybytelen != KEYSIZEINBYTES )
			{
				return MessageCode::InitFailureInvalidKeyLength;
			}
			if ( ivbytelen != IVSIZEINBYTES )
			{
				return MessageCode::InitFailureInvalidIVLength;
			}

			//Convert the data type to unmanaged
			uint8* internalKey = new uint8[keybytelen];
			uint8* internalIV = new uint8[ivbytelen];
			for ( uint8 itr = 0; itr < keybytelen; itr++ )
			{
				internalKey[itr] = key[itr];
			}
			for ( uint8 itr = 0; itr < ivbytelen; itr++ )
			{
				internalIV[itr] = iv[itr];
			}

			return init(action, filename, append, internalKey, keybytelen, internalIV, ivbytelen);
		}
		else
		{
			return MessageCode::InitFailureNeedFinalize;
		}

	}

	MessageCode engine::init( InitAction action, const char* filename, bool append, uint8* key, uint8 keybytelen, uint8* iv, uint8 ivbytelen )
	{
		if ( cpInternal->eStatus == NotReady )
		{
			if ( keybytelen != KEYSIZEINBYTES )
			{
				return MessageCode::InitFailureInvalidKeyLength;
			}
			if ( ivbytelen != IVSIZEINBYTES )
			{
				return MessageCode::InitFailureInvalidIVLength;
			}

			if ( action == InitAction::InitDecrypt )
			{
				ofstream* temp = new ofstream();
				if ( append )
				{
					temp->open(filename, ios::out | ios::app);
				}
				else
				{
					temp->open(filename);
				}

				if ( temp->fail() )
				{
					delete temp;
					return MessageCode::InitFailureFileError;
				}

				ostreamManagedWrapper^ tempWrapper = gcnew ostreamManagedWrapper(temp);
				cpInternal->output = tempWrapper;
				cpInternal->outputWrapped = true;

				//Build PRNG
				cpInternal->RabbitPRNGState = new ECRYPT_ctx;
				ECRYPT_init();
				ECRYPT_keysetup(cpInternal->RabbitPRNGState, key, keybytelen*8, ivbytelen*8); //bit size of key and iv
				ECRYPT_ivsetup(cpInternal->RabbitPRNGState, iv);

				//Build HuffmanEngine
				cpInternal->cpHuffmanGearbox = new huffman_gearbox( InitRead, cpInternal->RabbitPRNGState );

				cpInternal->eInitType = InitRead;
			}
			else if ( action == InitAction::InitEncrypt )
			{
				ofstream* temp = new ofstream();
				if ( append )
				{
					temp->open(filename, ios::out | ios::app);
				}
				else
				{
					temp->open(filename);
				}

				if ( temp->fail() )
				{
					delete temp;
					return MessageCode::InitFailureFileError;
				}

				ostreamManagedWrapper^ tempWrapper = gcnew ostreamManagedWrapper(temp);
				cpInternal->output = tempWrapper;
				cpInternal->outputWrapped = true;

				//Build PRNG
				cpInternal->RabbitPRNGState = new ECRYPT_ctx;
				ECRYPT_init();
				ECRYPT_keysetup(cpInternal->RabbitPRNGState, key, keybytelen*8, ivbytelen*8); //bit size of key and iv
				ECRYPT_ivsetup(cpInternal->RabbitPRNGState, iv);

				//Build HuffmanEngine
				cpInternal->cpHuffmanGearbox = new huffman_gearbox( InitWrite, cpInternal->RabbitPRNGState );

				//Clear bitstream
				cpInternal->bsBitBufferIn->flush();
				cpInternal->ssBitBufferOut->str( string() );
				cpInternal->ssBitBufferOut->clear();

				cpInternal->eInitType = InitWrite;
			}
			else
			{
				return MessageCode::InitFailureInvalidAction;
			}

			cpInternal->eDestType = DestFile;

			cpInternal->ui64BitsWritten = 0;
			cpInternal->ui64BytesRead = 0;

		}
		else
		{
			return MessageCode::InitFailureNeedFinalize;
		}

		//Set status
		cpInternal->eStatus = Ready;

		return MessageCode::InitSuccess;		
	}
	
	void engine::decrypt( IO::Stream^ input )
	{
		if ( cpInternal->eInitType == InitRead )
		{
			if ( cpInternal->eStatus == Ready )
			{
				if ( input->CanRead )
				{					
					array<uint8>^ inputBuf = gcnew array<uint8>(ECRYPT_BLOCKLENGTH);
					//input is cryptext
					uint8 cryptext[ECRYPT_BLOCKLENGTH]; //cryptext input buffer for PRNG
					uint8 middletext[ECRYPT_BLOCKLENGTH]; //middletext output buffer for PRNG
					stringstream MiddleTextBuf; //middletext input buffer for huffman gearbox
					//output is plaintext
					long long numBytesToRead = inputBuf->Length;
					int n, itr;
					//XOR entire stream
					while (numBytesToRead > 0)
					{
						// Read may return anything from 0 to ECRYPT_BLOCKLENGTH.
						n = input->Read(inputBuf, 0, ECRYPT_BLOCKLENGTH);
						// The end of the file is reached.
						if (n == 0)
						{
							break;
						}
						else if ( n == ECRYPT_BLOCKLENGTH )
						{
							//Full block
							for (itr=0; itr<ECRYPT_BLOCKLENGTH; itr--)
							{
								cryptext[itr] = inputBuf[itr];
							}
							ECRYPT_decrypt_blocks(cpInternal->RabbitPRNGState, cryptext, middletext, 1); //XOR
							MiddleTextBuf.write((char*)middletext, ECRYPT_BLOCKLENGTH); //Dump into buffer
						}
						else
						{
							//Remaining bytes
							for (itr=0; itr<n; itr--)
							{
								cryptext[itr] = inputBuf[itr];
							}
							ECRYPT_decrypt_bytes(cpInternal->RabbitPRNGState, cryptext, middletext, n ); //XOR
							MiddleTextBuf.write((char*)middletext, n ); //Dump into buffer, finishing message
							break;
						}
						numBytesToRead -= (long long)n;
					}
					
					//Feed middletext into huffman gearbox
					cpInternal->cpHuffmanGearbox->decode( MiddleTextBuf, cpInternal->output, cpInternal->outputWrapped, cpInternal->ui64BytesRead ); //Plaintext is feed directly into output stream
				}
			}
		}
	}

	void engine::decrypt( istream& input )
	{
		if ( cpInternal->eInitType == InitRead )
		{
			if ( cpInternal->eStatus == Ready )
			{
				//input is cryptext
				uint8 cryptext[ECRYPT_BLOCKLENGTH]; //cryptext input buffer for PRNG
				uint8 middletext[ECRYPT_BLOCKLENGTH]; //middletext output buffer for PRNG
				stringstream MiddleTextBuf; //middletext input buffer for huffman gearbox
				//output is plaintext
				
				//XOR entire stream
				while ( !input.eof() )
				{
					input.read((char*)cryptext, ECRYPT_BLOCKLENGTH); //Try to pull a block into buffer

					if ( input.gcount() == 0 )
					{
						break;
					}
					else if ( input.gcount() == ECRYPT_BLOCKLENGTH )
					{
						//Full block
						ECRYPT_decrypt_blocks(cpInternal->RabbitPRNGState, cryptext, middletext, 1); //XOR
						MiddleTextBuf.write((char*)middletext, ECRYPT_BLOCKLENGTH); //Dump into buffer
					}
					else
					{
						//Remaining bytes
						ECRYPT_decrypt_bytes(cpInternal->RabbitPRNGState, cryptext, middletext, input.gcount() ); //XOR
						MiddleTextBuf.write((char*)middletext, input.gcount() ); //Dump into buffer, finishing message
						break;
					}

				}
								
				//Feed middletext into huffman gearbox
				cpInternal->cpHuffmanGearbox->decode( MiddleTextBuf, cpInternal->output, cpInternal->outputWrapped, cpInternal->ui64BytesRead ); //Plaintext is feed directly into output stream
			}
		}
	}

	void engine::operator>> ( engine% a, IO::Stream^ input )
	{
		a.decrypt(input);
	}

	void engine::operator>> ( engine% a, istream& input )
	{
		a.decrypt(input);
	}

	void engine::encrypt( uint8% datum )
	{
		if ( cpInternal->eInitType == InitWrite )
		{
			if ( cpInternal->eStatus == SemiReady )
			{
				if ( cpInternal->outputWrapped )
				{
					//Clear output stream
					((ostreamManagedWrapper^)(cpInternal->output))->str( string() );
					((ostreamManagedWrapper^)(cpInternal->output))->clear();
				}

				//Reset size
				cpInternal->ui64BitsWritten = 0;
				//Set status
				cpInternal->eStatus = Ready;
			}

			if ( cpInternal->eStatus == Ready )
			{
				//Encode datum byte in huffman gearbox into bit buffer
				cpInternal->cpHuffmanGearbox->encode( datum, cpInternal->bsBitBufferIn, cpInternal->ui16BitsBuffered );
				
				//Consume full blocks from bit buffer if available, XORing and feeding to output
				if ( cpInternal->ui16BitsBuffered >= ECRYPT_BLOCKLENGTH*8 )
				{
					//ssBitBufferOut is middletext
					uint8 middletext[ECRYPT_BLOCKLENGTH]; //middletext input buffer for PRNG
					uint8 cryptext[ECRYPT_BLOCKLENGTH]; //cryptext output buffer for PRNG
					//output is cryptext
					array<uint8>^ outputBuf = gcnew array<uint8>(ECRYPT_BLOCKLENGTH);
					int itr;

					do
					{
						cpInternal->ssBitBufferOut->read(middletext, ECRYPT_BLOCKLENGTH); //Pull a block into buffer
						ECRYPT_encrypt_blocks(cpInternal->RabbitPRNGState, middletext, cryptext, 1); //XOR
						cpInternal->ui64BitsWritten += ECRYPT_BLOCKLENGTH*8;
						cpInternal->ui16BitsBuffered -= ECRYPT_BLOCKLENGTH*8; //Mark bit buffer reduction
						if ( cpInternal->outputWrapped )
						{
							((ostreamManagedWrapper^)(cpInternal->output))->write(cryptext, ECRYPT_BLOCKLENGTH); //Dump into output
						}
						else
						{
							for (itr=0; itr<ECRYPT_BLOCKLENGTH; itr--)
							{
								outputBuf[itr] = cryptext[itr];
							}
							cpInternal->output->Write(outputBuf,0,ECRYPT_BLOCKLENGTH);
						}

					} while ( cpInternal->ui16BitsBuffered >= ECRYPT_BLOCKLENGTH*8 );
				}
			}
		}

	}

	void engine::operator<< ( engine% a, uint8% datum )
	{
		a.encrypt(datum);
	}	

	void engine::encrypt( array<uint8>^ data )
	{
		for (int i=0; i<data->Length; i++)
			encrypt(data[i]);
	}

	void engine::encrypt( std::vector< uint8 >& data )
	{
		for (uint32 i=0; i<data.size(); i++)
			encrypt(data.at(i));
	}

	void engine::operator<< ( engine% a, array<uint8>^ data )
	{
		a.encrypt(data);
	}

	void engine::operator<< ( engine% a, std::vector< uint8 >& data )
	{
		a.encrypt(data);
	}

	void engine::encrypt( IO::Stream^ input )
	{
		if ( cpInternal->eInitType == InitWrite )
		{
			if ( cpInternal->eStatus == SemiReady )
			{
				if ( cpInternal->outputWrapped )
				{
					//Clear output stream
					((ostreamManagedWrapper^)(cpInternal->output))->str( string() );
					((ostreamManagedWrapper^)(cpInternal->output))->clear();
				}

				//Reset size
				cpInternal->ui64BitsWritten = 0;
				//Set status
				cpInternal->eStatus = Ready;
			}

			if ( cpInternal->eStatus == Ready )
			{
				if ( input->CanRead )
				{
					array<uint8>^ inputBuf = gcnew array<uint8>(1);
					long long numBytesToRead = inputBuf->Length;
					int n;
					uint8 temp;
					while (numBytesToRead > 0)
					{
						// Read may return anything from 0 to 1.
						n = input->Read(inputBuf, 0, 1);
						// The end of the file is reached.
						if (n == 0)
						{
							break;
						}
						else
						{
							temp = inputBuf[0];
							encrypt(temp);							
						}
						numBytesToRead -= (long long)n;
					}
					temp = 0;
				}
			}
		}
	}

	void engine::encrypt( istream& input )
	{
		if ( cpInternal->eInitType == InitWrite )
		{
			if ( cpInternal->eStatus == SemiReady )
			{
				if ( cpInternal->outputWrapped )
				{
					//Clear output stream
					((ostreamManagedWrapper^)(cpInternal->output))->str( string() );
					((ostreamManagedWrapper^)(cpInternal->output))->clear();
				}

				//Reset size
				cpInternal->ui64BitsWritten = 0;
				//Set status
				cpInternal->eStatus = Ready;
			}

			if ( cpInternal->eStatus == Ready )
			{
				uint8 curDatum;

				//Consume entire stream
				while ( !input.eof() )
				{
					input >> curDatum;

					encrypt(curDatum);
				}

				curDatum = 0; //Wipe data before it goes out of scope

			}
		}
	}

	void engine::operator<< ( engine% a, IO::Stream^ input )
	{
		a.encrypt(input);
	}

	void engine::operator<< ( engine% a, istream& input )
	{
		a.encrypt(input);
	}

	void engine::flush()
	{
		if ( cpInternal->eInitType == InitWrite )
		{
			if ( cpInternal->eDestType == DestStream )
			{
				if ( cpInternal->eStatus == Ready )
				{
					//Dump bit buffer into output
					cpInternal->flush(); // Internal flush

					//Reset bit buffer
					cpInternal->ssBitBufferOut->str( string() );
					cpInternal->ssBitBufferOut->clear();

					//Set status
					cpInternal->eStatus = SemiReady;
				}
			}
		}

	}

	void engine::finalize()
	{
		if ( cpInternal->eStatus != NotReady )
		{
			cpInternal->outputWrapped = false;

			if ( cpInternal->eInitType == InitWrite )
			{
				if ( cpInternal->eStatus == Ready )
				{
					cpInternal->flush(); //Internal flush
				}
			}
			//else if ( cpInternal->eInitType == InitRead )

			if ( cpInternal->eDestType == DestFile )
			{
				((ostreamManagedWrapper^)(cpInternal->output))->close();
				((ostreamManagedWrapper^)(cpInternal->output))->delfileptr();
			}
			//else if ( cpInternal->eDestType == DestStream ) { }

			cpInternal->eStatus = NotReady;

			//Destroy PRNG & HE
			if (cpInternal->cpHuffmanGearbox!=NULL)
			{
				delete cpInternal->cpHuffmanGearbox;
			}
			if (cpInternal->RabbitPRNGState!=NULL)
			{
				delete cpInternal->RabbitPRNGState;
			}
		}
	}
	

	twoBitKeystreamStack::~twoBitKeystreamStack()
	{
		delete[] keyStream;
	}

	twoBitKeystreamStack::twoBitKeystreamStack( ECRYPT_ctx* iRabbitPRNGState )
	{
		RabbitPRNGState = iRabbitPRNGState;
		keyStream = new u8[KEYSTREAMBUFFERSIZE];
		twoBitChunks = gcnew List<uint8>();
	}

	uint8 twoBitKeystreamStack::pop()
	{
		uint8 temp;

		if ( twoBitChunks->Count == 0 )
		{			
			ECRYPT_keystream_bytes(RabbitPRNGState, keyStream, KEYSTREAMBUFFERSIZE);
#if KEYSTREAMBUFFERSIZE > 1
			for ( uint16 index = KEYSTREAMBUFFERSIZE-1; index > 0; index-- )
			{
				twoBitChunks->Add( (keyStream[index]>>6) & 0x3 );
				twoBitChunks->Add( (keyStream[index]>>4) & 0x3 );
				twoBitChunks->Add( (keyStream[index]>>2) & 0x3 );
				twoBitChunks->Add( keyStream[index] & 0x3 );
				keyStream[index] = 0;
			}
#endif
			twoBitChunks->Add( (keyStream[0]>>6) & 0x3 );
			twoBitChunks->Add( (keyStream[0]>>4) & 0x3 );
			twoBitChunks->Add( (keyStream[0]>>2) & 0x3 );
			temp = keyStream[0] & 0x3;
			keyStream[0] = 0;
		}
		else
		{
			temp = twoBitChunks[twoBitChunks->Count - 1];
			twoBitChunks->RemoveAt(twoBitChunks->Count - 1);
		}

		return temp;
	}

	oneBitIstreamStack::~oneBitIstreamStack()
	{	}

	oneBitIstreamStack::oneBitIstreamStack( istream& input )
	{	
		source = gcnew istreamManagedWrapper(&input);
		oneBitChunks = gcnew List<uint1>();
	}

	uint1 oneBitIstreamStack::pop()
	{
		uint1 retval;

		if ( oneBitChunks->Count == 0 )
		{
			uint8 temp;
			if ( ((istreamManagedWrapper^)source)->good() )
			{
				*((istreamManagedWrapper^)source) >> temp;
			}
			else
			{
				throw exception("Unexpected end of stream!");
			}
			oneBitChunks->Add( (temp>>7) & 0x1 );
			oneBitChunks->Add( (temp>>6) & 0x1 );
			oneBitChunks->Add( (temp>>5) & 0x1 );
			oneBitChunks->Add( (temp>>4) & 0x1 );
			oneBitChunks->Add( (temp>>3) & 0x1 );
			oneBitChunks->Add( (temp>>2) & 0x1 );
			oneBitChunks->Add( (temp>>1) & 0x1 );
			retval = temp & 0x1;
		}
		else
		{
			retval = oneBitChunks[oneBitChunks->Count - 1];
			oneBitChunks->RemoveAt(oneBitChunks->Count - 1);
		}

		return retval;
	}

	bool oneBitIstreamStack::buffered()
	{
		return ( oneBitChunks->Count > 0 );
	}
	
#ifdef RANDOM_LOTTERY

	oneBitKeystreamStack::~oneBitKeystreamStack()
	{
		delete[] keyStream;
	}

	oneBitKeystreamStack::oneBitKeystreamStack( ECRYPT_ctx* iRabbitPRNGState )
	{
		RabbitPRNGState = iRabbitPRNGState;
		keyStream = new u8[KEYSTREAMBUFFERSIZE];
		oneBitChunks = gcnew List<uint1>();
	}

	uint1 oneBitKeystreamStack::pop()
	{
		uint1 temp;

		if ( oneBitChunks->Count == 0 )
		{			
			ECRYPT_keystream_bytes(RabbitPRNGState, keyStream, KEYSTREAMBUFFERSIZE);
#if KEYSTREAMBUFFERSIZE > 1
			for ( uint16 index = KEYSTREAMBUFFERSIZE-1; index > 0; index-- )
			{
				oneBitChunks->Add( (keyStream[index]>>7) & 0x1 );
				oneBitChunks->Add( (keyStream[index]>>6) & 0x1 );
				oneBitChunks->Add( (keyStream[index]>>5) & 0x1 );
				oneBitChunks->Add( (keyStream[index]>>4) & 0x1 );
				oneBitChunks->Add( (keyStream[index]>>3) & 0x1 );
				oneBitChunks->Add( (keyStream[index]>>2) & 0x1 );
				oneBitChunks->Add( (keyStream[index]>>1) & 0x1 );
				oneBitChunks->Add( keyStream[index] & 0x1 );
				keyStream[index] = 0;
			}
#endif
			oneBitChunks->Add( (keyStream[0]>>7) & 0x1 );
			oneBitChunks->Add( (keyStream[0]>>6) & 0x1 );
			oneBitChunks->Add( (keyStream[0]>>5) & 0x1 );
			oneBitChunks->Add( (keyStream[0]>>4) & 0x1 );
			oneBitChunks->Add( (keyStream[0]>>3) & 0x1 );
			oneBitChunks->Add( (keyStream[0]>>2) & 0x1 );
			oneBitChunks->Add( (keyStream[0]>>1) & 0x1 );
			temp = keyStream[0] & 0x1;
			keyStream[0] = 0;
		}
		else
		{
			temp = oneBitChunks[oneBitChunks->Count - 1];
			oneBitChunks->RemoveAt(oneBitChunks->Count - 1);
		}

		return temp;
	}

#else

	oneByteKeystreamStack::~oneByteKeystreamStack()
	{
		delete[] keyStream;
	}

	oneByteKeystreamStack::oneByteKeystreamStack( ECRYPT_ctx* iRabbitPRNGState )
	{
		RabbitPRNGState = iRabbitPRNGState;
		keyStream = new u8[KEYSTREAMBUFFERSIZE];
		byteChunks = gcnew List<uint8>();
	}

	uint8 oneByteKeystreamStack::pop()
	{
		uint8 temp;

		if ( byteChunks->Count == 0 )
		{			
			ECRYPT_keystream_bytes(RabbitPRNGState, keyStream, KEYSTREAMBUFFERSIZE);
#if KEYSTREAMBUFFERSIZE > 1
			for ( uint16 index = KEYSTREAMBUFFERSIZE-1; index > 0; index-- )
			{
				byteChunks->Add( keyStream[index] );
				keyStream[index] = 0;
			}
#endif
			temp = keyStream[0];
			keyStream[0] = 0;
		}
		else
		{
			temp = byteChunks[byteChunks->Count - 1];
			byteChunks->RemoveAt(byteChunks->Count - 1);
		}

		return temp;
	}

#endif

	randomLottery::~randomLottery()
	{
		delete decisionSource;
	}

	randomLottery::randomLottery( uint16 count, ECRYPT_ctx* iRabbitPRNGState )
	{

#ifdef RANDOM_LOTTERY
		decisionSource = gcnew oneBitKeystreamStack(iRabbitPRNGState);
#else
		decisionSource = gcnew oneByteKeystreamStack(iRabbitPRNGState);
#endif
		lotteryBalls = gcnew List<uint16>();

		for ( uint16 itr = 0; itr < count; itr++ )
		{
			lotteryBalls->Add(itr);
		}
	}

	uint16 randomLottery::pop()
	{

#ifdef RANDOM_LOTTERY

		uint16 curMinIndex = 0, curMaxIndex = lotteryBalls->Count-1, diff, selection;
		uint1 decision;

		for ( diff = curMaxIndex-curMinIndex; diff > 0;  diff = curMaxIndex-curMinIndex )
		{
			decision = decisionSource->pop();
			if ( decision == 0 )
			{
				if ( diff % 2 == 0 )
				{
					//Pull another bit to determine where to partition
					decision = decisionSource->pop();
					if ( decision == 0 )
					{
						//Middle item falls to the left and left partition chosen, so include it
						curMaxIndex = curMinIndex+(diff/2);
					}
					else
					{
						curMaxIndex = curMinIndex+(diff/2)-1;
					}
				}
				else
				{
					curMaxIndex = curMinIndex+(diff/2);
				}
			}
			else
			{
				if ( diff % 2 == 0 )
				{
					//Pull another bit to determine where to partition
					decision = decisionSource->pop();
					if ( decision == 0 )
					{
						curMinIndex = curMaxIndex-(diff/2)+1;
					}
					else
					{
						//Middle item falls to the right and right partition chosen, so include it
						curMinIndex = curMaxIndex-(diff/2);
					}
				}
				else
				{
					curMinIndex = curMaxIndex-(diff/2);
				}
			}
		}
		
		selection = lotteryBalls[curMinIndex];
		lotteryBalls->RemoveAt(curMinIndex);
		return selection;

#else
		uint16 curMinIndex = 0, curMaxIndex = lotteryBalls->Count, selection;

		curMinIndex = (uint16) ( ( ( (double)(decisionSource->pop()) / ( (double)UCHAR_MAX + 1.0) ) *  (double)curMaxIndex ) / 1 ); // + curMinIndex;

		selection = lotteryBalls[curMinIndex];
		lotteryBalls->RemoveAt(curMinIndex);
		return selection;
#endif

	}


	namespace {
		void deleteTree( treenode* cpTreeCrown ) {
			if (cpTreeCrown != NULL )
			{
				if (cpTreeCrown->IsLeaf)
				{
					delete cpTreeCrown;
				}
				else
				{
					deleteTree(((treebranch*)cpTreeCrown)->zero);
					deleteTree(((treebranch*)cpTreeCrown)->one);
					delete cpTreeCrown;
				}
			}
		}

		void buildDownTree( treebranch* cpCurrentBranch, uint16& LeafsRemaining, twoBitKeystreamStack^ cpTwoBitChunks )
		{
			if ( LeafsRemaining > 1 )
			{
				uint8 temp = cpTwoBitChunks->pop();

				if ( temp == 0 )
				{
					//Terminate branch
					cpCurrentBranch->zero = new treeleaf(); 
					cpCurrentBranch->zero->IsLeaf = true;
					cpCurrentBranch->one = new treeleaf();  
					cpCurrentBranch->one->IsLeaf = true;
				}
				else if ( temp == 1 )
				{
					LeafsRemaining--;
					//Left branch
					cpCurrentBranch->zero = new treebranch(); 
					cpCurrentBranch->zero->IsLeaf = false;
					
					cpCurrentBranch->one = new treeleaf();  
					cpCurrentBranch->one->IsLeaf = true;
					
					//Branch left
					buildDownTree((treebranch *)(cpCurrentBranch->zero), LeafsRemaining, cpTwoBitChunks);
				}
				else if ( temp == 2 )
				{
					LeafsRemaining--;
					//Right branch
					cpCurrentBranch->zero = new treeleaf(); 
					cpCurrentBranch->zero->IsLeaf = true;
					
					cpCurrentBranch->one = new treebranch();  
					cpCurrentBranch->one->IsLeaf = false;
					
					//Branch right
					buildDownTree((treebranch *)(cpCurrentBranch->one), LeafsRemaining, cpTwoBitChunks);
				}
				else
				{
					LeafsRemaining -= 2;
					//Double branch
					cpCurrentBranch->zero = new treebranch(); 
					cpCurrentBranch->zero->IsLeaf = false;
					
					cpCurrentBranch->one = new treebranch();  
					cpCurrentBranch->one->IsLeaf = false;
					
					//Consume another two bits to determine which path to take first
					temp = cpTwoBitChunks->pop();
					//Only consider the first bit, discard the second
					if ( ( temp & 0x1 ) == 0 ) //Check first bit
					{
						//Branch left
						buildDownTree((treebranch *)(cpCurrentBranch->zero), LeafsRemaining, cpTwoBitChunks);
						//Branch right
						buildDownTree((treebranch *)(cpCurrentBranch->one), LeafsRemaining, cpTwoBitChunks);
					}
					else
					{
						//Branch right
						buildDownTree((treebranch *)(cpCurrentBranch->one), LeafsRemaining, cpTwoBitChunks);
						//Branch left
						buildDownTree((treebranch *)(cpCurrentBranch->zero), LeafsRemaining, cpTwoBitChunks);
					}
				}
			}
			else if ( LeafsRemaining > 0 )
			{
				//Do not allow double branching
				uint8 temp = cpTwoBitChunks->pop();

				//Give temination 50% chance, and each single downward path 25% chance
				if ( ( temp & 0x1 ) == 0 ) //Check first bit
				{
					//Terminate branch
					//One leaf remains, to be handled by building up
					cpCurrentBranch->zero = new treeleaf(); 
					cpCurrentBranch->zero->IsLeaf = true;
					cpCurrentBranch->one = new treeleaf();  
					cpCurrentBranch->one->IsLeaf = true;
				}
				else if ( ( temp & 0x2 ) == 0 ) //Check second bit
				{
					LeafsRemaining--;
					//Left branch
					cpCurrentBranch->zero = new treebranch(); 
					cpCurrentBranch->zero->IsLeaf = false;
					
					cpCurrentBranch->one = new treeleaf();  
					cpCurrentBranch->one->IsLeaf = true;
					
					//Branch left
					buildDownTree((treebranch *)(cpCurrentBranch->zero), LeafsRemaining, cpTwoBitChunks);
				}
				else
				{
					LeafsRemaining--;
					//Right branch
					cpCurrentBranch->zero = new treeleaf(); 
					cpCurrentBranch->zero->IsLeaf = true;
					
					cpCurrentBranch->one = new treebranch();  
					cpCurrentBranch->one->IsLeaf = false;
					
					//Branch right
					buildDownTree((treebranch *)(cpCurrentBranch->one), LeafsRemaining, cpTwoBitChunks);
				}

			}
			else if ( LeafsRemaining == 0 )
			{
				//Last of the leaves, no need to consume any bits
				cpCurrentBranch->zero = new treeleaf(); 
				cpCurrentBranch->zero->IsLeaf = true;
				cpCurrentBranch->one = new treeleaf();  
				cpCurrentBranch->one->IsLeaf = true;
			}
		}

		void buildUpTree( treebranch* cpTreeRoot, uint16& LeafsRemaining, twoBitKeystreamStack^ cpTwoBitChunks )
		{

			if ( LeafsRemaining > 1 )
			{
				uint8 temp = cpTwoBitChunks->pop();

				if ( temp == 0 )
				{
					LeafsRemaining -= 2;
					//Double branch left
					treebranch* cpTreeTop = new treebranch();
					cpTreeTop->IsLeaf = false;

					cpTreeTop->zero = new treebranch(); 
					cpTreeTop->zero->IsLeaf = false;
					cpTreeTop->one = cpTreeRoot;
					
					cpTreeRoot = cpTreeTop;

					//Build down left
					buildDownTree((treebranch *)(cpTreeTop->zero), LeafsRemaining, cpTwoBitChunks);
					//Build up further
					buildUpTree(cpTreeRoot, LeafsRemaining, cpTwoBitChunks);
				}
				else if ( temp == 1 )
				{
					LeafsRemaining--;
					//Leaf on left
					treebranch* cpTreeTop = new treebranch();
					cpTreeTop->IsLeaf = false;

					cpTreeTop->zero = new treeleaf(); 
					cpTreeTop->zero->IsLeaf = true;
					cpTreeTop->one = cpTreeRoot;
					
					cpTreeRoot = cpTreeTop;

					//Build up further
					buildUpTree(cpTreeRoot, LeafsRemaining, cpTwoBitChunks);
				}
				else if ( temp == 2 )
				{
					LeafsRemaining--;
					//Leaf on right
					treebranch* cpTreeTop = new treebranch();
					cpTreeTop->IsLeaf = false;

					cpTreeTop->zero = cpTreeRoot;
					cpTreeTop->one = new treeleaf(); 
					cpTreeTop->one->IsLeaf = true;
					
					cpTreeRoot = cpTreeTop;

					//Build up further
					buildUpTree(cpTreeRoot, LeafsRemaining, cpTwoBitChunks);
				}
				else
				{
					LeafsRemaining -= 2;
					//Double branch right
					treebranch* cpTreeTop = new treebranch();
					cpTreeTop->IsLeaf = false;

					cpTreeTop->zero = cpTreeRoot;
					cpTreeTop->one = new treebranch(); 
					cpTreeTop->one->IsLeaf = false;
					
					cpTreeRoot = cpTreeTop;

					//Build down right
					buildDownTree((treebranch *)(cpTreeTop->one), LeafsRemaining, cpTwoBitChunks);
					//Build up further
					buildUpTree(cpTreeRoot, LeafsRemaining, cpTwoBitChunks);
				}				
			}
			else if ( LeafsRemaining > 0 )
			{
				//Only one leaf remaining
				LeafsRemaining--;
				uint8 temp = cpTwoBitChunks->pop();

				treebranch* cpTreeTop = new treebranch();
				cpTreeTop->IsLeaf = false;

				//Give each upward side 50% chance
				if ( ( temp & 0x1 ) == 0 ) //Check first bit
				{
					//Leaf on left
					cpTreeTop->zero = new treeleaf(); 
					cpTreeTop->zero->IsLeaf = true;
					cpTreeTop->one = cpTreeRoot;
				}
				else
				{
					//Leaf on right
					cpTreeTop->zero = cpTreeRoot;
					cpTreeTop->one = new treeleaf(); 
					cpTreeTop->one->IsLeaf = true;
				}

				cpTreeRoot = cpTreeTop;
			}
			//else
			//Tree finished
			//return;
		}

		void enumerateTree( treenode* cpCurrentNode, uint16& Counter )
		{
			if (cpCurrentNode != NULL )
			{
				if (cpCurrentNode->IsLeaf)
				{
					((treeleaf*)cpCurrentNode)->datum = (uint8)(Counter++);
				}
				else
				{
					enumerateTree(((treebranch*)cpCurrentNode)->zero, Counter);
					enumerateTree(((treebranch*)cpCurrentNode)->one, Counter);
				}
			}
		}

	}
	huffman_gear::~huffman_gear()
	{	}

	huffman_gear::huffman_gear( std::vector< uint8 > idata, uint8 inumBits ) : data(idata), numBits(inumBits)
	{
		//if ( (idata.size*8) < inumBits ) throw Exception(idata.size+" byte Vector can not contain "+inumBits+" bits.");
	}

	huffman_gearbox::~huffman_gearbox()
	{
		if ( eInitType == InitRead ) //decoding
		{
			deleteTree(cpTreeCrown);

#ifdef FASTER_HUFFMAN // Faster crank	
			for ( int ctr=0, ctrmax=2*FULL_SET_SIZE; ctr < ctrmax; ctr++ )
			{
				delete[] aaui8ReverseTable[ctr];
			}
			delete[] aaui8ReverseTable;
#else // More secure
			delete[] aui8ForwardSet;
			delete[] aui8ReverseSet;
#endif
		}
		else //InitWrite, encoding
		{		
#ifdef FASTER_HUFFMAN // Faster crank
			for ( int ctr=0, ctrmax=2*FULL_SET_SIZE; ctr < ctrmax; ctr++ )
			{
				for ( int index=0; index < FULL_SET_SIZE; index++ )
				{
					delete aacpGearTable[ctr][index];
				}
				delete[] aacpGearTable[ctr];
			}
			delete[] aacpGearTable;
#else // More secure
			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				delete acpGears[index];
			}
			delete[] acpGears;
#endif
		}

	}
	huffman_gearbox::huffman_gearbox( InitType itype, ECRYPT_ctx* RabbitPRNGState )
	{
		eInitType = itype;

		// Create tree

		treebranch* cpTreeStart = new treebranch();

		cpTreeStart->IsLeaf = false;
		cpTreeStart->zero = new treeleaf(); 
		cpTreeStart->zero->IsLeaf = true;
		cpTreeStart->one = new treeleaf();  
		cpTreeStart->one->IsLeaf = true;

		twoBitKeystreamStack^ cpTwoBitChunks = gcnew twoBitKeystreamStack(RabbitPRNGState);
		uint16 temp = FULL_SET_SIZE-2;
		buildUpTree(cpTreeStart, temp, cpTwoBitChunks);
		delete cpTwoBitChunks;

		cpTreeCrown = cpTreeStart;
		
		ui1AlphaBetaSpin = 0;
		randomLottery^ IndexSelector = gcnew randomLottery( FULL_SET_SIZE, RabbitPRNGState );

		if ( eInitType == InitRead ) //decoding
		{
			temp = 0;
			enumerateTree(cpTreeCrown, temp);

#ifdef FASTER_HUFFMAN // Faster crank	
			aaui8ReverseTable = new uint8*[2*FULL_SET_SIZE];
			for ( int ctr=0, ctrmax=2*FULL_SET_SIZE; ctr < ctrmax; ctr++ )
			{
				aaui8ReverseTable[ctr] = new uint8[FULL_SET_SIZE];
			}

			//Generate first row
			aui8ForwardSet = new uint8[FULL_SET_SIZE];

			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				aui8ForwardSet[IndexSelector->pop()] = index;
			}

			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				aaui8ReverseTable[0][aui8ForwardSet[index]] = index;
			}

			//Generate remaining rows
			crankCascade();

			delete[] aui8ForwardSet; //No longer needed
			
#else // More secure

			aui8ForwardSet = new uint8[FULL_SET_SIZE];

			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				aui8ForwardSet[IndexSelector->pop()] = index;
			}

			aui8ReverseSet = new uint8[FULL_SET_SIZE];

			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				aui8ReverseSet[aui8ForwardSet[index]] = index;
			}
#endif
		}
		else //InitWrite, encoding
		{		
			std::vector< uint8 > Sequence;

#ifdef FASTER_HUFFMAN // Faster crank
			aacpGearTable = new huffman_gear**[2*FULL_SET_SIZE];

			for ( int ctr=0, ctrmax=2*FULL_SET_SIZE; ctr < ctrmax; ctr++ )
			{
				aacpGearTable[ctr] = new huffman_gear*[FULL_SET_SIZE];
			}

			//Generate first row
			generateGears( cpTreeCrown, IndexSelector, 0, Sequence, aacpGearTable[0] );
			//Generate remaining rows
			crankCascade();
						
#else // More secure
			acpGears = new huffman_gear*[FULL_SET_SIZE];

			generateGears( cpTreeCrown, IndexSelector, 0, Sequence, acpGears );

#endif
			deleteTree(cpTreeCrown); //Tree no longer needed for encoding
		}
	}

	void huffman_gearbox::encode( uint8 datum, Out<>* bsBitBufferIn, uint16% ui16BitsBuffered  )
	{
		huffman_gear* temp;
#ifdef SHRINK_CYPHERTEXT
		uint8 datumUpper = datum >> 4;
		datum = datum & 0xF;
#endif

#ifdef FASTER_HUFFMAN // Faster crank
		temp = aacpGearTable[ui16Counter][datum];
#else // More secure
		temp = acpGears[datum];
#endif
		bsBitBufferIn->bits(temp->data, temp->numBits);
		ui16BitsBuffered += temp->numBits;
		crank();

#ifdef SHRINK_CYPHERTEXT

#ifdef FASTER_HUFFMAN // Faster crank
		temp = aacpGearTable[ui16Counter][datumUpper];
#else // More secure
		temp = acpGears[datumUpper];
#endif
		bsBitBufferIn->bits(temp->data, temp->numBits);
		ui16BitsBuffered += temp->numBits;
		crank();
#endif
	}

	void huffman_gearbox::decode( istream& input, Stream^ output, bool outputWrapped, uint64% ui64BytesRead )
	{
		//consume entire stream until dummy found
		oneBitIstreamStack^ TreeNavigator = gcnew oneBitIstreamStack( input );

		treenode* treeCursor;

#ifdef SHRINK_CYPHERTEXT
		uint8 temp;
#endif

		while ( input.good() || TreeNavigator->buffered() )
		{
			treeCursor = cpTreeCrown; //Rewind to top of tree

			//Navigate down to a leaf, consuming bits
			while ( treeCursor->IsLeaf == false )
			{
				if ( TreeNavigator->pop() == 0 )
				{
					treeCursor = ((treebranch*)treeCursor)->zero;
				}
				else
				{
					treeCursor = ((treebranch*)treeCursor)->one;				
				}
			}

#ifdef FASTER_HUFFMAN // Faster crank
			if ( aaui8ReverseTable[ui16Counter][((treeleaf*)treeCursor)->datum] < CHAR_SET_SIZE )
			{
#ifdef SHRINK_CYPHERTEXT
				temp = aaui8ReverseTable[ui16Counter][((treeleaf*)treeCursor)->datum];
#else
				if (outputWrapped)
				{
					((ostreamManagedWrapper^)(cpInternal->output)) << aaui8ReverseTable[ui16Counter][((treeleaf*)treeCursor)->datum];
				}
				else
				{
					output->WriteByte(aaui8ReverseTable[ui16Counter][((treeleaf*)treeCursor)->datum]);
				}
#endif
			}
#else // More secure
			if ( aui8ReverseSet[((treeleaf*)treeCursor)->datum] < CHAR_SET_SIZE )
			{
#ifdef SHRINK_CYPHERTEXT
				temp = aui8ReverseSet[((treeleaf*)treeCursor)->datum];
#else
				if (outputWrapped)
				{
					((ostreamManagedWrapper^)(output)) << aui8ReverseSet[((treeleaf*)treeCursor)->datum];
				}
				else
				{
					output->WriteByte(aui8ReverseSet[((treeleaf*)treeCursor)->datum]);
				}
#endif
				
			}
#endif
			else
			{
				crank();
				return;
			}

			crank();

#ifdef SHRINK_CYPHERTEXT
			//Get the upper half-byte

			treeCursor = cpTreeCrown; //Rewind to top of tree

			//Navigate down to a leaf, consuming bits
			while ( treeCursor->IsLeaf == false )
			{
				if ( TreeNavigator->pop() == 0 )
				{
					treeCursor = ((treebranch*)treeCursor)->zero;
				}
				else
				{
					treeCursor = ((treebranch*)treeCursor)->one;				
				}
			}

#ifdef FASTER_HUFFMAN // Faster crank
			if ( aaui8ReverseTable[ui16Counter][((treeleaf*)treeCursor)->datum] < CHAR_SET_SIZE )
			{
				if (outputWrapped)
				{
					((ostreamManagedWrapper^)(cpInternal->output)) << ( ((aaui8ReverseTable[ui16Counter][((treeleaf*)treeCursor)->datum])<<4) | temp );
				}
				else
				{
					output->WriteByte( ((aaui8ReverseTable[ui16Counter][((treeleaf*)treeCursor)->datum])<<4) | temp );
				}
			}
#else // More secure
			if ( aui8ReverseSet[((treeleaf*)treeCursor)->datum] < CHAR_SET_SIZE )
			{
				if (outputWrapped)
				{
					((ostreamManagedWrapper^)(cpInternal->output)) << ( ((aui8ReverseSet[((treeleaf*)treeCursor)->datum])<<4) | temp );
				}
				else
				{
					output->WriteByte( ((aui8ReverseSet[((treeleaf*)treeCursor)->datum])<<4) | temp );
				}
			}
#endif
			else
			{
				//ERROR, should not happen
				throw Exception("ERROR! Upper part of Half-Byte encoding is terminal character!");
			}

			crank();
#endif
			ui64BytesRead++;
		}
	}

	void huffman_gearbox::generateGears( treenode* cpCurrentNode, randomLottery^ IndexSelector, uint8 Depth, std::vector< uint8 >& Sequence, huffman_gear** acpGearBox )
	{
		if (cpCurrentNode != NULL )
		{
			if (cpCurrentNode->IsLeaf)
			{
				acpGearBox[IndexSelector->pop()] = new huffman_gear( Sequence, Depth );
			}
			else
			{
				if (Depth % 8 == 0)
				{
					Sequence.push_back(0);
				}
				Sequence.back() = Sequence.back() << 1;
				generateGears( ((treebranch*)cpCurrentNode)->zero, IndexSelector, Depth+1, Sequence, acpGearBox);
				Sequence.back() = Sequence.back() + 1;
				generateGears( ((treebranch*)cpCurrentNode)->one, IndexSelector, Depth+1, Sequence, acpGearBox);

				if (Depth % 8 == 0)
				{
					Sequence.pop_back();
				}
				else
				{
					Sequence.back() = Sequence.back() >> 1;
				}
			}
		}		
	}

	void huffman_gearbox::crank()
	{
#ifdef FASTER_HUFFMAN // Faster crank
		ui16Counter++;
		if (ui16Counter >= 2*FULL_SET_SIZE)
		{
			ui16Counter = 0;
		}
#else // More secure
		if ( eInitType == InitRead ) //decoding
		{
			uint8 formerHead = aui8ForwardSet[0];
			uint8 formerTail = aui8ForwardSet[FULL_SET_SIZE-1];

			for ( int index=0, stop=(FULL_SET_SIZE/2)-1; index < stop; index++ )
			{
				aui8ForwardSet[index] = aui8ForwardSet[index+1];
			}

			for ( int index=FULL_SET_SIZE-1, stop=(FULL_SET_SIZE/2); index > stop; index-- )
			{
				aui8ForwardSet[index] = aui8ForwardSet[index-1];
			}

			if ( ui1AlphaBetaSpin == 0 )
			{
				ui1AlphaBetaSpin = 1;

				aui8ForwardSet[(FULL_SET_SIZE/2)-1] = formerHead;
				aui8ForwardSet[FULL_SET_SIZE/2] = formerTail;
			}
			else
			{
				ui1AlphaBetaSpin = 0;
				
				aui8ForwardSet[(FULL_SET_SIZE/2)-1] = formerTail;
				aui8ForwardSet[FULL_SET_SIZE/2] = formerHead;
			}

			// Recreate aui8ReverseSet
			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				aui8ReverseSet[aui8ForwardSet[index]] = index;
			}
			
		}
		else //InitWrite, encoding
		{	
			huffman_gear* formerHead = acpGears[0];
			huffman_gear* formerTail = acpGears[FULL_SET_SIZE-1];

			for ( int index=0, stop=(FULL_SET_SIZE/2)-1; index < stop; index++ )
			{
				acpGears[index] = acpGears[index+1];
			}

			for ( int index=FULL_SET_SIZE-1, stop=(FULL_SET_SIZE/2); index > stop; index-- )
			{
				acpGears[index] = acpGears[index-1];
			}

			if ( ui1AlphaBetaSpin == 0 )
			{
				ui1AlphaBetaSpin = 1;

				acpGears[(FULL_SET_SIZE/2)-1] = formerHead;
				acpGears[FULL_SET_SIZE/2] = formerTail;
			}
			else
			{
				ui1AlphaBetaSpin = 0;
				
				acpGears[(FULL_SET_SIZE/2)-1] = formerTail;
				acpGears[FULL_SET_SIZE/2] = formerHead;
			}

		}
#endif
	}

	void huffman_gearbox::crankCascade()
	{
#ifdef FASTER_HUFFMAN // Faster crank
		if ( eInitType == InitRead ) //decoding
		{
			for ( uint16 startIndex=1, stopIndex= 2*FULL_SET_SIZE ; itr < stopIndex; itr++ )
			{
				uint8 formerHead = aui8ForwardSet[0];
				uint8 formerTail = aui8ForwardSet[FULL_SET_SIZE-1];

				for ( int index=0, stop=(FULL_SET_SIZE/2)-1; index < stop; index++ )
				{
					aui8ForwardSet[index] = aui8ForwardSet[index+1];
				}

				for ( int index=FULL_SET_SIZE-1, stop=(FULL_SET_SIZE/2); index > stop; index-- )
				{
					aui8ForwardSet[index] = aui8ForwardSet[index-1];
				}

				if ( ui1AlphaBetaSpin == 0 )
				{
					ui1AlphaBetaSpin = 1;

					aui8ForwardSet[(FULL_SET_SIZE/2)-1] = formerHead;
					aui8ForwardSet[FULL_SET_SIZE/2] = formerTail;
				}
				else
				{
					ui1AlphaBetaSpin = 0;
					
					aui8ForwardSet[(FULL_SET_SIZE/2)-1] = formerTail;
					aui8ForwardSet[FULL_SET_SIZE/2] = formerHead;
				}

				// Recreate aui8ReverseSet
				for ( int index=0; index < FULL_SET_SIZE; index++ )
				{
					aaui8ReverseTable[startIndex][aui8ForwardSet[index]] = index;
				}
			}
		}
		else //InitWrite, encoding
		{	
			for ( uint16 startIndex=0, stopIndex= (2*FULL_SET_SIZE)-1 ; itr < stopIndex; itr++ )
			{
				for ( int index=0, stop=(FULL_SET_SIZE/2)-1; index < stop; index++ )
				{
					aacpGearTable[startIndex+1][index] = aacpGearTable[startIndex][index+1];
				}

				for ( int index=FULL_SET_SIZE-1, stop=(FULL_SET_SIZE/2); index > stop; index-- )
				{
					aacpGearTable[startIndex+1][index] = aacpGearTable[startIndex][index-1];
				}

				if ( ui1AlphaBetaSpin == 0 )
				{
					ui1AlphaBetaSpin = 1;

					aacpGearTable[startIndex+1][(FULL_SET_SIZE/2)-1] = aacpGearTable[startIndex][0];
					aacpGearTable[startIndex+1][FULL_SET_SIZE/2] = aacpGearTable[startIndex][FULL_SET_SIZE-1];
				}
				else
				{
					ui1AlphaBetaSpin = 0;
					
					aacpGearTable[startIndex+1][(FULL_SET_SIZE/2)-1] = aacpGearTable[startIndex][FULL_SET_SIZE-1];
					aacpGearTable[startIndex+1][FULL_SET_SIZE/2] = aacpGearTable[startIndex][0];
				}
			}
		}
#endif
	}


	engine_internal::engine_internal()
	{
		eStatus = NotReady;
		eInitType = NotInit;
		eDestType = DestNone;
		stringstream* temp = new stringstream();
		ssBitBufferOut = gcnew stringstreamManagedWrapper(temp);
		bsBitBufferIn = new Out<>(*temp);
		ui64BitsWritten = 0;
		ui64BytesRead = 0;
		outputWrapped = false;
	}
	engine_internal::~engine_internal()
	{
		delete bsBitBufferIn;
		ssBitBufferOut->delSSPtr();
	}

	void engine_internal::flush()
	{
		//Write dummy terminal character through huffman gearbox into bit buffer
		cpHuffmanGearbox->encode( (uint8)CHAR_SET_SIZE, bsBitBufferIn, ui16BitsBuffered );
		
		//Flush any remaining bits in the bit buffer into the output stream, which might create padding
		bsBitBufferIn->flush();

		//ssBitBufferOut is middletext
		uint8 middletext[ECRYPT_BLOCKLENGTH]; //middletext input buffer for PRNG
		uint8 cryptext[ECRYPT_BLOCKLENGTH]; //cryptext output buffer for PRNG
		//output is cryptext
		array<uint8>^ outputBuf = gcnew array<uint8>(ECRYPT_BLOCKLENGTH);
		int itr;

		//Consume full blocks from bit buffer, XORing and feeding to output
		while ( ui16BitsBuffered >= ECRYPT_BLOCKLENGTH*8 )
		{				
			ssBitBufferOut->read(middletext, ECRYPT_BLOCKLENGTH); //Pull a block into buffer
			ECRYPT_encrypt_blocks(RabbitPRNGState, middletext, cryptext, 1); //XOR
			ui64BitsWritten += ECRYPT_BLOCKLENGTH*8;
			ui16BitsBuffered -= ECRYPT_BLOCKLENGTH*8; //Mark bit buffer reduction

			if ( outputWrapped )
			{
				((ostreamManagedWrapper^)(output))->write(cryptext, ECRYPT_BLOCKLENGTH); //Dump into output
			}
			else
			{
				for (itr=0; itr<ECRYPT_BLOCKLENGTH; itr--)
				{
					outputBuf[itr] = cryptext[itr];
				}
				output->Write(outputBuf,0,ECRYPT_BLOCKLENGTH);
			}
		}
		
		if ( ui16BitsBuffered > 0 )
		{
			//Remaining bytes
			uint8 RemainingBytes = (ui16BitsBuffered % 8 > 0 ) ? ((ui16BitsBuffered/8)+1) : (ui16BitsBuffered/8);
			ssBitBufferOut->read(middletext, RemainingBytes ); //Pull a block into buffer
			ECRYPT_encrypt_bytes(RabbitPRNGState, middletext, cryptext, RemainingBytes); //XOR
			ui64BitsWritten += ui16BitsBuffered;
			ui16BitsBuffered = 0; //Mark bit buffer reduction
			if ( outputWrapped )
			{
				((ostreamManagedWrapper^)(output))->write(cryptext, RemainingBytes); //Dump into output
			}
			else
			{
				for (itr=0; itr<RemainingBytes; itr--)
				{
					outputBuf[itr] = cryptext[itr];
				}
				output->Write(outputBuf,0,RemainingBytes);
			}
		}
	}

} //namespace cipup