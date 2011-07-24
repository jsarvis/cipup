
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
// cipup.internal.test.cpp : Defines the exported functions for the DLL application.
//


#include <iostream>
#include <sstream>
#include <exception>
//#include <cstdlib>
//#include <time.h>
#include "cipup_internal_test.hpp"

using namespace Costella::Bitstream;
using namespace cipup;
using namespace cipup::TreeFunc;
using namespace std;
using namespace System;

namespace cipup_internal_test {

	// This is the constructor of a class that has been exported.
	// see cipup.internal.test.h for the class definition
	test::test()
	{
		return;
	}

	static int TestMessageLen = 10000;

	static void AssertEquals(array<uint8>^ a1, array<uint8>^ a2)
        {
            if (a1 == a2)
            {
                return;
            }
			Assert::True((a1 != nullptr) && (a2 != nullptr), "Mismatched arrays, Null.");

			Assert::AreEqual(a1->Length, a2->Length, "Mismatched arrays, length mismatch.");
            for (int i = 0; i < a1->Length; i++)
            {
				Assert::AreEqual(a1[i], a2[i], "Mismatched arrays, value mismatch.");
            }
    }

	void test::BitstreamTest()
	{
		stringstream ss;
		Out<> testBS( ss );
		uint8 temp;
		vector< uint8 > idata;
		idata.push_back(0xAA);

		testBS.bits(idata, 4);
		testBS.bits(idata, 8);

		idata.push_back(0xFF);

		//1 byte, 4 bits
		ss >> temp;
		Console::WriteLine( "170="+(uint32)temp );

		testBS.bits(idata, 10);
		testBS.bits(idata, 16);

		//4 bytes-1 byte = 3 bytes
		ss >> temp;
		Console::WriteLine( "170="+(uint32)temp );
		ss >> temp;
		Console::WriteLine( "174="+(uint32)temp );
		ss >> temp;
		Console::WriteLine( "171="+(uint32)temp );
		testBS.flush();
		ss >> temp;
		Console::WriteLine( "252="+(uint32)temp );
		//Costella::Bitstream::In
		//Assert::AreEqual( );
		//Assert::True( true, "msg1");
	}

	void test::TreeTest()
	{

		uint8* key = new uint8[KEYSIZEINBYTES];
		uint8* iv = new uint8[IVSIZEINBYTES];

		ECRYPT_ctx* RabbitPRNGState;

		RabbitPRNGState = new ECRYPT_ctx;
		ECRYPT_init();
		ECRYPT_keysetup(RabbitPRNGState, key, KEYSIZEINBYTES*8, IVSIZEINBYTES*8); //bit size of key and iv
		ECRYPT_ivsetup(RabbitPRNGState, iv);

		// Create tree
		treebranch* cpTreeCrown;
		treebranch* cpTreeStart = new treebranch();

		cpTreeStart->IsLeaf = false;
		cpTreeStart->zero = new treeleaf(); 
		cpTreeStart->zero->IsLeaf = true;
		cpTreeStart->one = new treeleaf();  
		cpTreeStart->one->IsLeaf = true;

		twoBitKeystreamStack^ cpTwoBitChunks = gcnew twoBitKeystreamStack(RabbitPRNGState);
		uint16 temp = FULL_SET_SIZE-2;
		Console::WriteLine( "Making "+(temp+2)+" leafs.");
		cpTreeCrown = buildUpTree(cpTreeStart, temp, cpTwoBitChunks);
		Console::WriteLine( "Done: "+temp+" leafs remaining.");
		delete cpTwoBitChunks;


		temp = 0;
		enumerateTree(cpTreeCrown, temp);
		Console::WriteLine( "Enumeration Done: "+temp+" leafs found.");

		deleteTree(cpTreeCrown);
		delete key;
		delete iv;
	}

	static uint8 mask[9] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
	static uint32 minSeqLen = UINT_MAX, maxSeqLen = 0, sumSeqLen = 0;

	static void genGears( treenode* cpCurrentNode, randomLottery^ IndexSelector, uint8 Depth, std::vector< uint8 >& Sequence, huffman_gear** acpGearBox )
	{
		if (cpCurrentNode != NULL )
		{
			if (cpCurrentNode->IsLeaf)
			{
				Console::Write("Leaf "+((treeleaf*)cpCurrentNode)->datum+" found, ");
				uint16 temp = IndexSelector->pop();
				Console::Write("Saving sequence ");
				if (Depth % 8 == 0)
				{
					for (uint32 itr = 0; itr < Sequence.size(); itr++)
					{
						Console::Write( ((Sequence[itr]>>7)&0x1) + "-" +((Sequence[itr]>>6)&0x1) + "-" +((Sequence[itr]>>5)&0x1) + "-" +((Sequence[itr]>>4)&0x1) + "-" +((Sequence[itr]>>3)&0x1) + "-" +((Sequence[itr]>>2)&0x1) + "-" +((Sequence[itr]>>1)&0x1) + "-" +(Sequence[itr]&0x1) +"-" );
					}
				}
				else
				{
					for (uint32 itr = 0; itr < Sequence.size()-1; itr++)
					{
						Console::Write( ((Sequence[itr]>>7)&0x1) + "-" +((Sequence[itr]>>6)&0x1) + "-" +((Sequence[itr]>>5)&0x1) + "-" +((Sequence[itr]>>4)&0x1) + "-" +((Sequence[itr]>>3)&0x1) + "-" +((Sequence[itr]>>2)&0x1) + "-" +((Sequence[itr]>>1)&0x1) + "-" +(Sequence[itr]&0x1) +"-" );
					}
					for (int itr = 0; itr < Depth%8; itr++)
					{
						Console::Write( ((Sequence[Sequence.size()-1]>>(7-itr))&0x1) + "-" );
					}
				}
				Console::WriteLine(" at depth "+Depth+" to index "+temp+".");
				acpGearBox[temp] = new huffman_gear( Sequence, Depth );
				if (Depth < minSeqLen) minSeqLen = Depth;
				if (Depth > maxSeqLen) maxSeqLen = Depth;
				sumSeqLen += Depth;
			}
			else
			{
				if (Depth % 8 == 0)
				{
					Sequence.push_back(0);
				}
				
				genGears( ((treebranch*)cpCurrentNode)->zero, IndexSelector, Depth+1, Sequence, acpGearBox);
				Sequence.back() = Sequence.back() | ( 1 << (7 - (Depth%8)));
				genGears( ((treebranch*)cpCurrentNode)->one, IndexSelector, Depth+1, Sequence, acpGearBox);

				if (Depth % 8 == 0)
				{
					Sequence.pop_back();
				}
				else
				{
					Sequence.back() = Sequence.back() & mask[Depth%8];
				}
			}
		}		
	}

	void test::GenGearPresetTest()
	{

		uint8* key = new uint8[KEYSIZEINBYTES];
		uint8* iv = new uint8[IVSIZEINBYTES];

		for ( uint32 itr = 0; itr < KEYSIZEINBYTES; itr++ )
		{
			key[itr] = 0;
		}
		for ( uint32 itr = 0; itr < IVSIZEINBYTES; itr++ )
		{
			iv[itr] = 0;
		}

		GenGearTest(key, iv);
		
		delete key;
		delete iv;
	}

	void test::GenGearRandTest()
	{

		uint8* key = new uint8[KEYSIZEINBYTES];
		uint8* iv = new uint8[IVSIZEINBYTES];

		engine::GenerateKey(key, KEYSIZEINBYTES);
		engine::GenerateKey(iv, IVSIZEINBYTES);		 

		GenGearTest(key, iv);
		
		delete key;
		delete iv;
	}

	void test::GenGearTest(uint8* key, uint8* iv)
	{
		ECRYPT_ctx* RabbitPRNGState;

		RabbitPRNGState = new ECRYPT_ctx;
		ECRYPT_init();
		ECRYPT_keysetup(RabbitPRNGState, key, KEYSIZEINBYTES*8, IVSIZEINBYTES*8); //bit size of key and iv
		ECRYPT_ivsetup(RabbitPRNGState, iv);

		// Create tree
		treebranch* cpTreeCrown;
		treebranch* cpTreeStart = new treebranch();

		cpTreeStart->IsLeaf = false;
		cpTreeStart->zero = new treeleaf(); 
		cpTreeStart->zero->IsLeaf = true;
		cpTreeStart->one = new treeleaf();  
		cpTreeStart->one->IsLeaf = true;

		twoBitKeystreamStack^ cpTwoBitChunks = gcnew twoBitKeystreamStack(RabbitPRNGState);
		uint16 temp = FULL_SET_SIZE-2;
		Console::WriteLine( "Making "+(temp+2)+" leafs.");
		cpTreeCrown = buildUpTree(cpTreeStart, temp, cpTwoBitChunks);
		Console::WriteLine( "Done: "+temp+" leafs remaining.");
		delete cpTwoBitChunks;

		temp = 0;
		enumerateTree(cpTreeCrown, temp);

		randomLottery^ IndexSelector = gcnew randomLottery( FULL_SET_SIZE, RabbitPRNGState );

		std::vector< uint8 > Sequence;
		huffman_gear** acpGears = new huffman_gear*[FULL_SET_SIZE];

		genGears( cpTreeCrown, IndexSelector, 0, Sequence, acpGears );

		Console::WriteLine( "Sequence length: Min "+minSeqLen+", Max "+maxSeqLen+", Avg "+((double)sumSeqLen/(double)FULL_SET_SIZE)+".");
	
		PrintGears(acpGears);
		Console::WriteLine( "Sequence length: Min "+minSeqLen+", Max "+maxSeqLen+", Avg "+((double)sumSeqLen/(double)FULL_SET_SIZE)+".");

		for ( int index=0; index < FULL_SET_SIZE; index++ )
		{
			delete acpGears[index];
		}
		delete[] acpGears;

		deleteTree(cpTreeCrown);
	}

	void test::PrintGears(huffman_gear** acpGears)
	{
		minSeqLen = UINT_MAX;
		maxSeqLen = 0;
		sumSeqLen = 0;
		Console::WriteLine( );
		Console::WriteLine( );
		Console::WriteLine( );

		for ( int index=0; index < FULL_SET_SIZE; index++ )
		{
			Console::Write("Gear "+index+", numBits "+acpGears[index]->numBits+" held in "+acpGears[index]->data.size()+"=");
			if (acpGears[index]->numBits % 8 == 0)
			{
				Console::Write(((acpGears[index]->numBits)/8)+" bytes, sequence ");
				for (uint32 itr = 0; itr < acpGears[index]->data.size(); itr++)
				{
					Console::Write( ((acpGears[index]->data[itr]>>7)&0x1) + "-" +((acpGears[index]->data[itr]>>6)&0x1) + "-" +((acpGears[index]->data[itr]>>5)&0x1) + "-" +((acpGears[index]->data[itr]>>4)&0x1) + "-" +((acpGears[index]->data[itr]>>3)&0x1) + "-" +((acpGears[index]->data[itr]>>2)&0x1) + "-" +((acpGears[index]->data[itr]>>1)&0x1) + "-" +(acpGears[index]->data[itr]&0x1) +"-" );
				}
			}
			else
			{
				Console::Write((((acpGears[index]->numBits)/8)+1)+" bytes, sequence ");
				for (uint32 itr = 0; itr < acpGears[index]->data.size()-1; itr++)
				{
					Console::Write( ((acpGears[index]->data[itr]>>7)&0x1) + "-" +((acpGears[index]->data[itr]>>6)&0x1) + "-" +((acpGears[index]->data[itr]>>5)&0x1) + "-" +((acpGears[index]->data[itr]>>4)&0x1) + "-" +((acpGears[index]->data[itr]>>3)&0x1) + "-" +((acpGears[index]->data[itr]>>2)&0x1) + "-" +((acpGears[index]->data[itr]>>1)&0x1) + "-" +(acpGears[index]->data[itr]&0x1) +"-" );
				}
				for (int itr = 0; itr < acpGears[index]->numBits%8; itr++)
				{
					Console::Write( ((acpGears[index]->data[acpGears[index]->data.size()-1]>>(7-itr))&0x1) + "-" );
				}
			}
			Console::WriteLine(".");
			if (acpGears[index]->numBits < minSeqLen) minSeqLen = acpGears[index]->numBits;
			if (acpGears[index]->numBits > maxSeqLen) maxSeqLen = acpGears[index]->numBits;
			sumSeqLen += acpGears[index]->numBits;
		}
	}

	//C++ interface testing
	array<uint8>^ test::StreamToStreamTest(InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, istream* src, stringstream* dest)
    {
        MessageCode status;

        engine^ alpha = gcnew engine();

        status = alpha->init(action, (ostringstream*)dest, key, keylen, iv, ivlen);

		if ( action == InitAction::InitEncrypt )
			Console::Write("Encrypt stream Init returned: ");
		else
			Console::Write("Decrypt stream Init returned: ");

		Console::WriteLine(engine::Messages[(int)status]);
        Assert::AreEqual(MessageCode::InitSuccess, status);

        uint64 destlen;

        if ( action == InitAction::InitEncrypt )
        {
            alpha->encrypt(*src);
            Console::WriteLine("Pre-flush bits written: " + alpha->bitswritten());
            alpha->flush();
            Console::WriteLine("Post-flush bits written: " + alpha->bitswritten());
            alpha->finalize();
            Console::WriteLine("Post-finalize bits written: " + alpha->bitswritten());
            Console::WriteLine("Post-finalize bytes written: " + alpha->byteswritten());
            Console::WriteLine("Post-finalize overflow bits written: " + alpha->overflowbits());
            destlen = alpha->byteswritten() + (uint64)((alpha->overflowbits() > 0) ? 1 : 0);
        }
        else if (action == InitAction::InitDecrypt)
        {
            alpha->decrypt(*src);
            Console::WriteLine("Pre-finalize bytes read: " + alpha->bytesread());
            alpha->finalize();
            Console::WriteLine("Post-finalize bytes read: " + alpha->bytesread());
            destlen = alpha->bytesread();
        }
        else
        {
            throw new exception("Invalid action!");
        }

		uint8* buffer =  new uint8[(uint32)destlen];
		dest->read((char*)buffer, (int)destlen);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)buffer);
		array<uint8>^ desttext = gcnew array<uint8>((int)destlen);
		Runtime::InteropServices::Marshal::Copy(ptr, desttext, 0, (int)destlen);

		delete[] buffer;

        Console::WriteLine("Result text:");
        for (int itr = 0, stop = (destlen < 20) ? (int)destlen : 20; itr < stop; itr++)
        {
            Console::Write(desttext[itr] + "-");
        }
        Console::WriteLine("... Total of " + destlen + " bytes.");
        return desttext;
    }

	array<uint8>^ test::StreamFlowOpToStreamTest(InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest)
	{
        MessageCode status;

        engine^ alpha = gcnew engine();

        status = alpha->init(action, (ostringstream*)dest, key, keylen, iv, ivlen);

		if ( action == InitAction::InitEncrypt )
			Console::Write("Encrypt stream Init returned: ");
		else
			Console::Write("Decrypt stream Init returned: ");

		Console::WriteLine(engine::Messages[(int)status]);
        Assert::AreEqual(MessageCode::InitSuccess, status);

        uint64 destlen;

        if ( action == InitAction::InitEncrypt )
        {
            *alpha << *src;
            Console::WriteLine("Pre-flush bits written: " + alpha->bitswritten());
            alpha->flush();
            Console::WriteLine("Post-flush bits written: " + alpha->bitswritten());
            alpha->finalize();
            Console::WriteLine("Post-finalize bits written: " + alpha->bitswritten());
            Console::WriteLine("Post-finalize bytes written: " + alpha->byteswritten());
            Console::WriteLine("Post-finalize overflow bits written: " + alpha->overflowbits());
            destlen = alpha->byteswritten() + (uint64)((alpha->overflowbits() > 0) ? 1 : 0);
        }
        else if (action == InitAction::InitDecrypt)
        {
            *alpha >> *src;
            Console::WriteLine("Pre-finalize bytes read: " + alpha->bytesread());
            alpha->finalize();
            Console::WriteLine("Post-finalize bytes read: " + alpha->bytesread());
            destlen = alpha->bytesread();
        }
        else
        {
            throw new exception("Invalid action!");
        }

		uint8* buffer =  new uint8[(uint32)destlen];
		dest->read((char*)buffer, (int)destlen);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)buffer);
		array<uint8>^ desttext = gcnew array<uint8>((int)destlen);
		Runtime::InteropServices::Marshal::Copy(ptr, desttext, 0, (int)destlen);

		delete[] buffer;

        Console::WriteLine("Result text:");
        for (int itr = 0, stop = (destlen < 20) ? (int)destlen : 20; itr < stop; itr++)
        {
            Console::Write(desttext[itr] + "-");
        }
        Console::WriteLine("... Total of " + destlen + " bytes.");
        return desttext;
    }

	array<uint8>^ test::VectorToStreamTest(InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest)
	{
        MessageCode status;

        engine^ alpha = gcnew engine();

        status = alpha->init(action, (ostringstream*)dest, key, keylen, iv, ivlen);

		if ( action == InitAction::InitEncrypt )
			Console::Write("Encrypt stream Init returned: ");
		else
			Console::Write("Decrypt stream Init returned: ");

		Console::WriteLine(engine::Messages[(int)status]);
        Assert::AreEqual(MessageCode::InitSuccess, status);

        uint64 destlen;

        if ( action == InitAction::InitEncrypt )
        {
			uint8 buffer[5];
			vector< uint8 > data;

			while ( !src->eof() )
			{
				src->read((char*)buffer,5);

				if ( src->gcount() == 0 )
				{
					break;
				}
				else
				{
					for (int itr=0; itr<src->gcount(); itr++)
					{
						data.push_back(buffer[itr]);
					}
				}

				alpha->encrypt(data);

				data.clear();
			}
            
            Console::WriteLine("Pre-flush bits written: " + alpha->bitswritten());
            alpha->flush();
            Console::WriteLine("Post-flush bits written: " + alpha->bitswritten());
            alpha->finalize();
            Console::WriteLine("Post-finalize bits written: " + alpha->bitswritten());
            Console::WriteLine("Post-finalize bytes written: " + alpha->byteswritten());
            Console::WriteLine("Post-finalize overflow bits written: " + alpha->overflowbits());
            destlen = alpha->byteswritten() + (uint64)((alpha->overflowbits() > 0) ? 1 : 0);
        }
        else
        {
            throw new exception("Invalid action!");
        }

		uint8* buffer =  new uint8[(uint32)destlen];
		dest->read((char*)buffer, (int)destlen);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)buffer);
		array<uint8>^ desttext = gcnew array<uint8>((int)destlen);
		Runtime::InteropServices::Marshal::Copy(ptr, desttext, 0, (int)destlen);

		delete[] buffer;

        Console::WriteLine("Result text:");
        for (int itr = 0, stop = (destlen < 20) ? (int)destlen : 20; itr < stop; itr++)
        {
            Console::Write(desttext[itr] + "-");
        }
        Console::WriteLine("... Total of " + destlen + " bytes.");
        return desttext;
    }

	array<uint8>^ test::VectorFlowOpToStreamTest(InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest)
	{
        MessageCode status;

        engine^ alpha = gcnew engine();

        status = alpha->init(action, (ostringstream*)dest, key, keylen, iv, ivlen);

		if ( action == InitAction::InitEncrypt )
			Console::Write("Encrypt stream Init returned: ");
		else
			Console::Write("Decrypt stream Init returned: ");

		Console::WriteLine(engine::Messages[(int)status]);
        Assert::AreEqual(MessageCode::InitSuccess, status);

        uint64 destlen;

        if ( action == InitAction::InitEncrypt )
        {
			uint8 buffer[5];
			vector< uint8 > data;

			while ( !src->eof() )
			{
				src->read((char*)buffer,5);

				if ( src->gcount() == 0 )
				{
					break;
				}
				else
				{
					for (int itr=0; itr<src->gcount(); itr++)
					{
						data.push_back(buffer[itr]);
					}
				}

				*alpha << data;

				data.clear();
			}
            
            Console::WriteLine("Pre-flush bits written: " + alpha->bitswritten());
            alpha->flush();
            Console::WriteLine("Post-flush bits written: " + alpha->bitswritten());
            alpha->finalize();
            Console::WriteLine("Post-finalize bits written: " + alpha->bitswritten());
            Console::WriteLine("Post-finalize bytes written: " + alpha->byteswritten());
            Console::WriteLine("Post-finalize overflow bits written: " + alpha->overflowbits());
            destlen = alpha->byteswritten() + (uint64)((alpha->overflowbits() > 0) ? 1 : 0);
        }
        else
        {
            throw new exception("Invalid action!");
        }

		uint8* buffer =  new uint8[(uint32)destlen];
		dest->read((char*)buffer, (int)destlen);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)buffer);
		array<uint8>^ desttext = gcnew array<uint8>((int)destlen);
		Runtime::InteropServices::Marshal::Copy(ptr, desttext, 0, (int)destlen);

		delete[] buffer;

        Console::WriteLine("Result text:");
        for (int itr = 0, stop = (destlen < 20) ? (int)destlen : 20; itr < stop; itr++)
        {
            Console::Write(desttext[itr] + "-");
        }
        Console::WriteLine("... Total of " + destlen + " bytes.");
        return desttext;
    }

	array<uint8>^ test::SingleToStreamTest(InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest)
	{
        MessageCode status;

        engine^ alpha = gcnew engine();

        status = alpha->init(action, (ostringstream*)dest, key, keylen, iv, ivlen);

		if ( action == InitAction::InitEncrypt )
			Console::Write("Encrypt stream Init returned: ");
		else
			Console::Write("Decrypt stream Init returned: ");

		Console::WriteLine(engine::Messages[(int)status]);
        Assert::AreEqual(MessageCode::InitSuccess, status);

        uint64 destlen;

        if ( action == InitAction::InitEncrypt )
        {
			uint8 buffer[1];
			bool toggle = false;

			while ( !src->eof() )
			{
				src->read((char*)buffer,1);

				if ( src->gcount() == 0 )
				{
					break;
				}
				
				//Test both by value and by ref
				if (toggle)
				{
					toggle = false;
					alpha->encrypt(buffer);
				}
				else
				{
					toggle=true;
					alpha->encrypt(buffer[0]);
				}

			}
            
            Console::WriteLine("Pre-flush bits written: " + alpha->bitswritten());
            alpha->flush();
            Console::WriteLine("Post-flush bits written: " + alpha->bitswritten());
            alpha->finalize();
            Console::WriteLine("Post-finalize bits written: " + alpha->bitswritten());
            Console::WriteLine("Post-finalize bytes written: " + alpha->byteswritten());
            Console::WriteLine("Post-finalize overflow bits written: " + alpha->overflowbits());
            destlen = alpha->byteswritten() + (uint64)((alpha->overflowbits() > 0) ? 1 : 0);
        }
        else
        {
            throw new exception("Invalid action!");
        }

		uint8* buffer =  new uint8[(uint32)destlen];
		dest->read((char*)buffer, (int)destlen);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)buffer);
		array<uint8>^ desttext = gcnew array<uint8>((int)destlen);
		Runtime::InteropServices::Marshal::Copy(ptr, desttext, 0, (int)destlen);

		delete[] buffer;

        Console::WriteLine("Result text:");
        for (int itr = 0, stop = (destlen < 20) ? (int)destlen : 20; itr < stop; itr++)
        {
            Console::Write(desttext[itr] + "-");
        }
        Console::WriteLine("... Total of " + destlen + " bytes.");
        return desttext;
    }

	array<uint8>^ test::SingleFlowOpToStreamTest(InitAction action, uint8* key, uint8 keylen, uint8* iv, uint8 ivlen, std::istream* src, std::stringstream* dest)
	{
        MessageCode status;

        engine^ alpha = gcnew engine();

        status = alpha->init(action, (ostringstream*)dest, key, keylen, iv, ivlen);

		if ( action == InitAction::InitEncrypt )
			Console::Write("Encrypt stream Init returned: ");
		else
			Console::Write("Decrypt stream Init returned: ");

		Console::WriteLine(engine::Messages[(int)status]);
        Assert::AreEqual(MessageCode::InitSuccess, status);

        uint64 destlen;

        if ( action == InitAction::InitEncrypt )
        {
			uint8 buffer[1];
			bool toggle = false;

			while ( !src->eof() )
			{
				src->read((char*)buffer,1);

				if ( src->gcount() == 0 )
				{
					break;
				}
				
				//Test both by value and by ref
				if (toggle)
				{
					toggle = false;
					*alpha << buffer;
				}
				else
				{
					toggle=true;
					*alpha << buffer[0];
				}

			}
            
            Console::WriteLine("Pre-flush bits written: " + alpha->bitswritten());
            alpha->flush();
            Console::WriteLine("Post-flush bits written: " + alpha->bitswritten());
            alpha->finalize();
            Console::WriteLine("Post-finalize bits written: " + alpha->bitswritten());
            Console::WriteLine("Post-finalize bytes written: " + alpha->byteswritten());
            Console::WriteLine("Post-finalize overflow bits written: " + alpha->overflowbits());
            destlen = alpha->byteswritten() + (uint64)((alpha->overflowbits() > 0) ? 1 : 0);
        }
        else
        {
            throw new exception("Invalid action!");
        }

		uint8* buffer =  new uint8[(uint32)destlen];
		dest->read((char*)buffer, (int)destlen);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)buffer);
		array<uint8>^ desttext = gcnew array<uint8>((int)destlen);
		Runtime::InteropServices::Marshal::Copy(ptr, desttext, 0, (int)destlen);

		delete[] buffer;

        Console::WriteLine("Result text:");
        for (int itr = 0, stop = (destlen < 20) ? (int)destlen : 20; itr < stop; itr++)
        {
            Console::Write(desttext[itr] + "-");
        }
        Console::WriteLine("... Total of " + destlen + " bytes.");
        return desttext;
    }

    void test::StreamToStreamEncryptRandKeyRandIVRandMessageTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
		uint8 keylen = engine::RequiredKeyByteLength();
		uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        engine::GenerateKey(key, keylen);
        engine::GenerateKey(iv, ivlen);
        engine::GenerateKey(plaintext, (uint32)messagelen);

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");

		src->write((char*)plaintext, messagelen);

		StreamToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);

		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

    void test::StreamToStreamEncryptRandKeyRandIVTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        engine::GenerateKey(key, keylen);
        engine::GenerateKey(iv, ivlen);
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");

		src->write((char*)plaintext, messagelen);

        StreamToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);

		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

    void test::StreamToStreamEncryptRandIVTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        engine::GenerateKey(iv, ivlen);
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        StreamToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

    void test::StreamToStreamEncryptTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        for (int itr = 0; itr < ivlen; itr++)
        {
            iv[itr] = 0;
        }
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        StreamToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

    void test::StreamToStreamRandKeyRandIVRandMessageFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        engine::GenerateKey(key, keylen);
        engine::GenerateKey(iv, ivlen);
        engine::GenerateKey(plaintext, (uint32)messagelen);

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");

		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = StreamToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);

		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

    void test::StreamToStreamRandKeyRandIVFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        engine::GenerateKey(key, keylen);
        engine::GenerateKey(iv, ivlen);
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

        Console::Write("uint8* key = { ");
        for (int itr = 0; itr < keylen-1; itr++)
        {
            Console::Write("0x{0:X}, ", key[itr]);
        }
        Console::WriteLine("0x{0:X} }};", key[keylen - 1]);

        Console::Write("uint8* iv = { ");
        for (int itr = 0; itr < ivlen - 1; itr++)
        {
            Console::Write("0x{0:X}, ", iv[itr]);

        }
        Console::WriteLine("0x{0:X} }};", iv[ivlen - 1]);

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = StreamToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);

		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

    void test::StreamToStreamRandIVFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        engine::GenerateKey(iv, ivlen);
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

        Console::Write("uint8* iv = { ");
        for (int itr = 0; itr < ivlen - 1; itr++)
        {
            Console::Write("0x{0:X}, ", iv[itr]);

        }
        Console::WriteLine("0x{0:X} }};", iv[ivlen - 1]);

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");

		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = StreamToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);

		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);

		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

    void test::StreamToStreamFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        for (int itr = 0; itr < ivlen; itr++)
        {
            iv[itr] = 0;
        }
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = StreamToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);
        
		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

	void test::StreamFlowOpToStreamFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        for (int itr = 0; itr < ivlen; itr++)
        {
            iv[itr] = 0;
        }
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = StreamFlowOpToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);
        
		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamFlowOpToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

	void test::VectorToStreamFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        for (int itr = 0; itr < ivlen; itr++)
        {
            iv[itr] = 0;
        }
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = VectorToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);
        
		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

	void test::VectorFlowOpToStreamFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        for (int itr = 0; itr < ivlen; itr++)
        {
            iv[itr] = 0;
        }
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = VectorFlowOpToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);
        
		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

	void test::SingleToStreamFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        for (int itr = 0; itr < ivlen; itr++)
        {
            iv[itr] = 0;
        }
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = SingleToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);
        
		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

	void test::SingleFlowOpToStreamFullTest()
    {
		stringstream* src = new stringstream();
		stringstream* dest = new stringstream();
        uint8 keylen = engine::RequiredKeyByteLength();
        uint8 ivlen = engine::RequiredIVByteLength();
        int messagelen = TestMessageLen;
        uint8* key = new uint8[keylen];
        uint8* iv = new uint8[ivlen];
        uint8* plaintext = new uint8[messagelen];
        for (int itr = 0; itr < keylen; itr++)
        {
            key[itr] = 0;
        }
        for (int itr = 0; itr < ivlen; itr++)
        {
            iv[itr] = 0;
        }
        for (int itr = 0; itr < messagelen; itr++)
        {
            plaintext[itr] = (uint8)(itr % 256);
        }

		Console::WriteLine("Source text:");
        for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
        {
            Console::Write(plaintext[itr] + "-");
        }
        Console::WriteLine("... Total of " + messagelen + " bytes.");


		src->write((char*)plaintext, messagelen);

        array<uint8>^ cryptext = SingleFlowOpToStreamTest(InitAction::InitEncrypt, key, keylen, iv, ivlen, src, dest);
        
		src->str( string() );
		src->clear();
		dest->str( string() );
		dest->clear();
		//Managed to unmanaged conversion
		pin_ptr<uint8> pptr = &cryptext[0];
		src->write((const char*)pptr, cryptext->Length);

        array<uint8>^ resulttext = StreamToStreamTest(InitAction::InitDecrypt, key, keylen, iv, ivlen, src, dest);

		//Unmanaged to managed conversion
		IntPtr ptr((void*)plaintext);
		array<uint8>^ plaintextarr = gcnew array<uint8>(messagelen);
		Runtime::InteropServices::Marshal::Copy(ptr, plaintextarr, 0, messagelen);

        AssertEquals(plaintextarr, resulttext);
		
		delete[] key;
		delete[] iv;
		delete[] plaintext;
		delete src;
		delete dest;
    }

} //namespace cipup_internal_test

