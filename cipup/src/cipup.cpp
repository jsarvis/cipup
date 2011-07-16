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

#include <iostream>
#include <fstream>
#include <exception>
#include <cstdlib>
//#include <time.h>
#include "cipup.hpp"
#include "engine_internal.hpp"

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

	twoBitKeystreamStack::~twoBitKeystreamStack()
	{
		delete keyStream;
	}

	twoBitKeystreamStack::twoBitKeystreamStack( ECRYPT_ctx* iRabbitPRNGState )
	{
		RabbitPRNGState = iRabbitPRNGState;
		keyStream = new u8[1];
	}

	uint8 twoBitKeystreamStack::pop()
	{
		uint8 temp;

		if ( twoBitChunks.size() == 0 )
		{			
			ECRYPT_keystream_bytes(RabbitPRNGState, keyStream, 1);
			twoBitChunks.push_back( (keyStream[0]>>6) & 0x3 );
			twoBitChunks.push_back( (keyStream[0]>>4) & 0x3 );
			twoBitChunks.push_back( (keyStream[0]>>2) & 0x3 );
			temp = keyStream[0] & 0x3;
			keyStream[0] = 0;
		}
		else
		{
			temp = twoBitChunks.back();
			twoBitChunks.pop_back();
		}

		return temp;
	}

	oneBitKeystreamStack::~oneBitKeystreamStack()
	{
		delete keyStream;
	}

	oneBitKeystreamStack::oneBitKeystreamStack( ECRYPT_ctx* iRabbitPRNGState )
	{
		RabbitPRNGState = iRabbitPRNGState;
		keyStream = new u8[1];
	}

	uint1 oneBitKeystreamStack::pop()
	{
		uint1 temp;

		if ( oneBitChunks.size() == 0 )
		{			
			ECRYPT_keystream_bytes(RabbitPRNGState, keyStream, 1);
			oneBitChunks.push_back( (keyStream[0]>>7) & 0x1 );
			oneBitChunks.push_back( (keyStream[0]>>6) & 0x1 );
			oneBitChunks.push_back( (keyStream[0]>>5) & 0x1 );
			oneBitChunks.push_back( (keyStream[0]>>4) & 0x1 );
			oneBitChunks.push_back( (keyStream[0]>>3) & 0x1 );
			oneBitChunks.push_back( (keyStream[0]>>2) & 0x1 );
			oneBitChunks.push_back( (keyStream[0]>>1) & 0x1 );
			temp = keyStream[0] & 0x1;
			keyStream[0] = 0;
		}
		else
		{
			temp = oneBitChunks.back();
			oneBitChunks.pop_back();
		}

		return temp;
	}

	oneBitIstreamStack::~oneBitIstreamStack()
	{	}

	oneBitIstreamStack::oneBitIstreamStack( istream& input ) : source(input)
	{	}

	uint1 oneBitIstreamStack::pop()
	{
		uint1 retval;

		if ( oneBitChunks.size() == 0 )
		{
			uint8 temp;
			source >> temp;
			oneBitChunks.push_back( (temp>>7) & 0x1 );
			oneBitChunks.push_back( (temp>>6) & 0x1 );
			oneBitChunks.push_back( (temp>>5) & 0x1 );
			oneBitChunks.push_back( (temp>>4) & 0x1 );
			oneBitChunks.push_back( (temp>>3) & 0x1 );
			oneBitChunks.push_back( (temp>>2) & 0x1 );
			oneBitChunks.push_back( (temp>>1) & 0x1 );
			retval = temp & 0x1;
		}
		else
		{
			retval = oneBitChunks.back();
			oneBitChunks.pop_back();
		}

		return retval;
	}

	bool oneBitIstreamStack::buffered()
	{
		return ( oneBitChunks.size() > 0 );
	}
	
	randomLottery::~randomLottery()
	{
		delete decisionSource;
	}

	randomLottery::randomLottery( uint16 count, ECRYPT_ctx* iRabbitPRNGState )
	{
		decisionSource = new oneBitKeystreamStack(iRabbitPRNGState);

		for ( uint16 itr = 0; itr < count; itr++ )
		{
			lotteryBalls.push_back(itr);
		}
	}

	uint16 randomLottery::pop()
	{
		uint16 curMinIndex = 0, curMaxIndex = lotteryBalls.size()-1, diff, selection;
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
		lotteryBalls.erase(lotteryBalls.begin()+curMinIndex);
		return selection;
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

		void buildDownTree( treebranch* cpCurrentBranch, uint16& LeafsRemaining, twoBitKeystreamStack* cpTwoBitChunks )
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

		void buildUpTree( treebranch* cpTreeRoot, uint16& LeafsRemaining, twoBitKeystreamStack* cpTwoBitChunks )
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
				delete aaui8ReverseTable[ctr];
			}
			delete aaui8ReverseTable;
#else // More secure
			delete aui8ForwardSet;
			delete aui8ReverseSet;
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
				delete aacpGearTable[ctr];
			}
			delete aacpGearTable;
#else // More secure
			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				delete acpGears[index];
			}
			delete acpGears;
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

		twoBitKeystreamStack* cpTwoBitChunks = new twoBitKeystreamStack(RabbitPRNGState);
		uint16 temp = FULL_SET_SIZE-2;
		buildUpTree(cpTreeStart, temp, cpTwoBitChunks);
		delete cpTwoBitChunks;

		cpTreeCrown = cpTreeStart;
		
		ui1AlphaBetaSpin = 0;
		randomLottery IndexSelector ( FULL_SET_SIZE, RabbitPRNGState );

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
				aui8ForwardSet[IndexSelector.pop()] = index;
			}

			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				aaui8ReverseTable[0][aui8ForwardSet[index]] = index;
			}

			//Generate remaining rows
			crankCascade();

			delete aui8ForwardSet; //No longer needed
			
#else // More secure

			aui8ForwardSet = new uint8[FULL_SET_SIZE];

			for ( int index=0; index < FULL_SET_SIZE; index++ )
			{
				aui8ForwardSet[IndexSelector.pop()] = index;
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

	huffman_gear& huffman_gearbox::encode( uint8 datum )
	{
		//TODO SHRINK_CYPHERTEXT
#ifdef FASTER_HUFFMAN // Faster crank
		huffman_gear& temp = *(aacpGearTable[ui16Counter][datum]);
#else // More secure
		huffman_gear& temp = *(acpGears[datum]);
#endif
		crank();		
		return temp;
	}

	void huffman_gearbox::decode( istream& input, ostream* output )
	{
		//consume entire stream until dummy found
		oneBitIstreamStack TreeNavigator ( input );

		treenode* treeCursor;

		while ( input.good() || TreeNavigator.buffered() )
		{

			//TODO SHRINK_CYPHERTEXT


			treeCursor = cpTreeCrown; //Rewind to top of tree

			//Navigate down to a leaf, consuming bits
			while ( treeCursor->IsLeaf == false )
			{
				if ( TreeNavigator.pop() == 0 )
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
				*output << aaui8ReverseTable[ui16Counter][((treeleaf*)treeCursor)->datum];
			}
#else // More secure
			if ( aui8ReverseSet[((treeleaf*)treeCursor)->datum] < CHAR_SET_SIZE )
			{
				*output << aui8ReverseSet[((treeleaf*)treeCursor)->datum];
			}
#endif
			else
			{
				crank();
				return;
			}

			crank();
		}
	}

	void huffman_gearbox::generateGears( treenode* cpCurrentNode, randomLottery& IndexSelector, uint8 Depth, std::vector< uint8 >& Sequence, huffman_gear** acpGearBox )
	{
		if (cpCurrentNode != NULL )
		{
			if (cpCurrentNode->IsLeaf)
			{
				acpGearBox[IndexSelector.pop()] = new huffman_gear( Sequence, Depth );
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
		bsBitBufferIn = new Out<>(ssBitBufferOut);
		ui64BitsWritten = 0;
		ui64BytesRead = 0;
	}
	engine_internal::~engine_internal()
	{
		delete bsBitBufferIn;
		
		//These should not be needed
		if (cpHuffmanGearbox!=NULL)
		{
			delete cpHuffmanGearbox;
		}
		if (RabbitPRNGState!=NULL)
		{
			delete RabbitPRNGState;
		}
		if (output!=NULL)
		{
			delete output;
		}
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

				//TODO Build PRNG & HE

				cpInternal->eInitType = InitRead;
			}
			else if ( action == InitEncrypt )
			{
				
				//TODO Build PRNG & HE

				//Clear bitstream
				cpInternal->bsBitBufferIn->flush();
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
					((ofstream*)(cpInternal->output))->open(filename, ios::out | ios::app);
				}
				else
				{
					((ofstream*)(cpInternal->output))->open(filename);
				}

				if ( cpInternal->output->fail() )
				{
					return InitFailureFileError;
				}

				//TODO Build PRNG & HE

				cpInternal->eInitType = InitRead;
			}
			else if ( action == InitEncrypt )
			{

				cpInternal->output = new ofstream();
				((ofstream*)(cpInternal->output))->open(filename);
				
				if ( cpInternal->output->fail() )
				{
					return InitFailureFileError;
				}

				//TODO Build PRNG & HE

				//Clear bitstream
				cpInternal->bsBitBufferIn->flush();
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
	
	void engine::decrypt( istream& input )
	{
		if ( cpInternal->eInitType == InitRead )
		{
			if ( cpInternal->eStatus == Ready )
			{

				//TODO
				//XOR entire stream
				//feed to huffman gearbox

			}
		}
	}
	void engine::operator>> ( istream& input )
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
				((ostringstream*)(cpInternal->output))->str( string() );
				cpInternal->output->clear();
				//Reset size
				cpInternal->ui64BitsWritten = 0;
				//Set status
				cpInternal->eStatus = Ready;
			}

			if ( cpInternal->eStatus == Ready )
			{
				//TODO
				//encode datum in huffman gearbox
				//feed bit vector into bitstream

				//consume any bytes from bitstream, XORing and feeding to output
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

	void engine::flush()
	{
		if ( cpInternal->eInitType == InitWrite )
		{
			if ( cpInternal->eDestType == DestStream )
			{
				if ( cpInternal->eStatus == Ready )
				{

					//TODO

					//cpInternal->bsBitBufferIn->flush();


					//cpInternal->ssBitBufferOut.str( string() );
					//cpInternal->ssBitBufferOut.clear();

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

} //namespace cipup