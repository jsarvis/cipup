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

/*	Cipup switches	
 *
 *	Any time an algorithm was designed to increase security at the cost of speed or size,
 *	a switch was defined that could allow engine customization. Since Cipup prefers security
 *	over speed and size, the default is more security, and thus it is recommended to leave
 *	these switches undefined.
 */

// Reduces Huffman Engine crank operation from O(N) to O(1) -- 256 ops to 1 op -- by building a full table of permutations.
// This is a minor reduction in local security, as the full table would make it trival to find the initial Huffman engine configuration in memory.
//#define FASTER_HUFFMAN

// Reduces output size of cyphertext by using a half-byte character set, which reduces average bit length of encoded characters.
// This is a severe reduction in security, as it would reduce the potential seeds of the initial Huffman engine configuration to less than an exhaustive key search of the PRNG that generates the seed.
// Warning: This switch will make cyphertext incompatible with Cipup builds that do not use this.
//#define SHRINK_CYPHERTEXT


#define KEYSIZEINBYTES 16
#define IVSIZEINBYTES 8

// Character set size, must be divisible by 2
// Full set size with dummies, if CHAR_SET_SIZE % 4 == 0, then add 2 dummies to ensure a huffman gearbox period of 2N
//		Otherwise add 4 dummies, to ensure dummies can be used as terminal signals
#ifdef SHRINK_CYPHERTEXT //Double Half-byte

#define CHAR_SET_SIZE 16
#define FULL_SET_SIZE 18

#else //Full byte

#define CHAR_SET_SIZE 256
#define FULL_SET_SIZE 258

#endif





#include "..\bitstream\Bitstream.imp.h"
#include "..\prng\ecrypt-sync.h"


#using <mscorlib.dll>
using namespace Costella::Bitstream;
using namespace System;

namespace cipup {
	
	enum Status { NotReady = 0, Ready, SemiReady };
	enum InitType { NotInit = 0, InitRead, InitWrite };
	enum DestType { DestNone = 0, DestStream, DestFile };

	private class twoBitKeystreamStack {
	public:
		~twoBitKeystreamStack();

		twoBitKeystreamStack( ECRYPT_ctx* iRabbitPRNGState );

		uint8 pop();

	private:
		vector< uint8 > twoBitChunks;
		ECRYPT_ctx* RabbitPRNGState;
		u8 * keyStream;
	};

	private class oneBitKeystreamStack {
	public:
		~oneBitKeystreamStack();

		oneBitKeystreamStack( ECRYPT_ctx* iRabbitPRNGState );

		uint1 pop();

	private:
		vector< uint1 > oneBitChunks;
		ECRYPT_ctx* RabbitPRNGState;
		u8 * keyStream;
	};

	private class oneBitIstreamStack {
	public:
		~oneBitIstreamStack();

		oneBitIstreamStack( istream& input );

		uint1 pop();
		bool buffered();

	private:
		vector< uint1 > oneBitChunks;
		istream& source;
	};

	//Spreads probability of selecting remaining numbers as evenly as possible
	private class randomLottery {
	public:
		~randomLottery();

		randomLottery( uint16 count, ECRYPT_ctx* iRabbitPRNGState );

		uint16 pop();

	private:
		vector< uint16 > lotteryBalls;
		oneBitKeystreamStack* decisionSource;
	};

	private class treenode {
	public:
		bool IsLeaf;
	};
	private class treebranch : public treenode {
	public:
		treenode* zero;
		treenode* one;
	};
	private class treeleaf : public treenode {
	public:
		uint8 datum;
	};

	namespace {
		void deleteTree( treenode* cpTreeCrown );
		void buildDownTree( treebranch* cpCurrentBranch, uint16& LeafsRemaining, twoBitKeystreamStack* cpTwoBitChunks );
		void buildUpTree( treebranch* cpTreeRoot, uint16& LeafsRemaining, twoBitKeystreamStack* cpTwoBitChunks );
		void enumerateTree( treenode* cpCurrentNode, uint16& Counter );
	}

	private class huffman_gear {
	public:
		~huffman_gear();

		huffman_gear( std::vector< uint8 > idata, uint8 inumBits );

		std::vector< uint8 >& data;
		uint8& numBits;

	};

	private class huffman_gearbox {
	public:
		~huffman_gearbox();

		huffman_gearbox( InitType itype, ECRYPT_ctx* RabbitPRNGState );

		huffman_gear& encode( uint8 datum );
		void decode( istream& input, ostream* output ); //TODO change input type to compatible post-XOR data structure

	private:
		
		InitType eInitType;

		//tree pointer, kept for decoding
		treenode* cpTreeCrown;
		uint8* aui8ForwardSet;
		uint1 ui1AlphaBetaSpin;

#ifdef FASTER_HUFFMAN // Faster crank
		uint16 ui16Counter;
		huffman_gear*** aacpGearTable;
		uint8** aaui8ReverseTable;
#else // More secure		
		huffman_gear** acpGears;
		uint8* aui8ReverseSet;
#endif

		void generateGears( treenode* cpCurrentNode, randomLottery& IndexSelector, uint8 Depth, std::vector< uint8 >& Sequence, huffman_gear** acpGearBox );
		void crank();
		void crankCascade();

	};

	private class engine_internal {
	public:
		engine_internal();
		~engine_internal();

		Status eStatus;
		InitType eInitType;
		DestType eDestType;

		huffman_gearbox* cpHuffmanGearbox;
		stringstream ssBitBufferOut;
		Out<>* bsBitBufferIn;
		ECRYPT_ctx* RabbitPRNGState;

		ostream* output;

		uint64 ui64BitsWritten;
		uint64 ui64BytesRead;

	};
	


} //namespace cipup
