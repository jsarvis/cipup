
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

namespace cipup_internal_test {

	// This is the constructor of a class that has been exported.
	// see cipup.internal.test.h for the class definition
	test::test()
	{
		return;
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

	void genGears( treenode* cpCurrentNode, randomLottery^ IndexSelector, uint8 Depth, std::vector< uint8 >& Sequence, huffman_gear** acpGearBox )
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

} //namespace cipup_internal_test

