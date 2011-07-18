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

// Uses a lottery system to pick indicies. The (lowered) bias towards picking a specific number is psuedorandomly distributed during each pass.
// This is a minor increase in execution time. With this disabled, an alternate method is used wherein the probability of picking a specific number is evenly distributed.
// Warning: This switch will make cyphertext incompatible with Cipup builds that do not use this.
//#define RANDOM_LOTTERY

// Buffer size of used keystream. This determines the maximum number of bytes of the keystream held in memory at any given time.
// The safest value is one, but since Rabbit consumes the keystream in 16 byte blocks, any value not divisible by 16 discards the byte difference inside the PRNG.
// Thus a value of one only uses the first byte of every 16 byte block. The smallest value that uses the entire keystream is 16.
// Warning: This switch will make cyphertext incompatible with Cipup builds that use a different value. One value must be selected.
#define KEYSTREAMBUFFERSIZE 16
//#define KEYSTREAMBUFFERSIZE 1



/* Do not adjust these */

//Required by Rabbit
#define KEYSIZEINBYTES 16
#define IVSIZEINBYTES 8
#define ECRYPT_BLOCKLENGTH 16

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

#include <iostream>
#include <fstream>

#using <mscorlib.dll>
using namespace Costella::Bitstream;
using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;

namespace cipup {
	
	enum Status { NotReady = 0, Ready, SemiReady };
	enum InitType { NotInit = 0, InitRead, InitWrite };
	enum DestType { DestNone = 0, DestStream, DestFile };

	private ref class istreamManagedWrapper : Stream {
	public:
		istreamManagedWrapper(istream* contents) : boxedStream(contents)
		{ if (contents == NULL) { throw exception("Can't wrap null stream."); } }
		void operator>>(uint8& ch) { *boxedStream >> ch; }
		void read(uint8* s, streamsize n) { boxedStream->read((char*)s,n); }
		streamsize gcount() { return boxedStream->gcount(); }
		bool eof() { return boxedStream->eof(); }
		bool good() { return boxedStream->good(); }
		virtual int Read(array<uint8>^ buffer, int offset, int count) override 
		{
			uint8* temp = new uint8[count];
			read(temp, count);
			for ( int index=0; index < count; index++ )
			{ buffer[index+offset]=temp[index]; }			
			delete[] temp;
			return boxedStream->gcount();
		}
		virtual void Write(array<uint8>^ buffer, int offset, int count) override { } // Do nothing
		virtual property bool CanRead { bool get() override { return boxedStream->good(); } }
		virtual property bool CanWrite { bool get() override { return false; } }
		virtual property bool CanSeek { bool get() override { return boxedStream->good(); } }
		virtual property long long Length { long long get() override { return boxedStream->good() ? 1 : 0; } }
		virtual property long long Position { long long get() override { return boxedStream->tellg(); } void set(long long value) override { boxedStream->seekg((streamoff)value); } }	
		virtual void Flush() override { } // Do nothing
		virtual long long Seek(long long offset, SeekOrigin origin) override { boxedStream->seekg((streamoff)offset, (origin==SeekOrigin::Begin) ? ios_base::beg : ((origin==SeekOrigin::End) ? ios_base::end : ios_base::cur)); return boxedStream->tellg(); } 
		virtual void SetLength(long long value) override { } // Do nothing
	private:
		istream* boxedStream;
	};

	private ref class ostreamManagedWrapper : Stream {
	public:
		ostreamManagedWrapper(ostream* contents) : boxedStream(contents)
		{ origFilePtr=NULL; if (contents == NULL) { throw exception("Can't wrap null stream."); } }
		ostreamManagedWrapper(ofstream* contents)
		{ boxedStream=(ostream*)contents; origFilePtr=contents; if (contents == NULL) { throw exception("Can't wrap null stream."); } }
		void operator<<(uint8 c) { *boxedStream << c; }
		void write(const uint8* s, streamsize n) { boxedStream->write((char*)s,n); }
		void str(const string & s) { ((ostringstream*)boxedStream)->str(s); }
		void clear() { return boxedStream->clear(); }
		void close() { if (origFilePtr != NULL) {origFilePtr->close();} }
		void delfileptr() { if (origFilePtr != NULL) { delete origFilePtr; } }
		virtual int Read(array<uint8>^ buffer, int offset, int count) override { return 0; } // Do nothing
		virtual void Write(array<uint8>^ buffer, int offset, int count) override 
		{
			uint8* temp = new uint8[count];
			for ( int index=0; index < count; index++ )
			{ temp[index]=buffer[index+offset]; }
			write(temp, count);
			delete[] temp;
		}
		virtual property bool CanRead { bool get() override { return false; } }
		virtual property bool CanWrite { bool get() override { return boxedStream->good(); } }
		virtual property bool CanSeek { bool get() override { return boxedStream->good(); } }
		virtual property long long Length { long long get() override { return boxedStream->good() ? 1 : 0; } }
		virtual property long long Position { long long get() override { return boxedStream->tellp(); } void set(long long value) override { boxedStream->seekp((streamoff)value); } }			
		virtual void Flush() override { boxedStream->flush(); }
		virtual long long Seek(long long offset, SeekOrigin origin) override { boxedStream->seekp((streamoff)offset, (origin==SeekOrigin::Begin) ? ios_base::beg : ((origin==SeekOrigin::End) ? ios_base::end : ios_base::cur)); return boxedStream->tellp(); } 
		virtual void SetLength(long long value) override { } // Do nothing
	private:
		ostream* boxedStream;
		ofstream* origFilePtr;
	};

	private ref class stringstreamManagedWrapper : Stream {
	public:
		stringstreamManagedWrapper(stringstream* contents) : boxedStream(contents)
		{ if (contents == NULL) { throw exception("Can't wrap null stream."); } }
		void operator>>(uint8& ch) { *boxedStream >> ch; }
		void operator<<(uint8 c) { *boxedStream << c; }
		void read(uint8* s, streamsize n) { boxedStream->read((char*)s,n); }
		void write(const uint8* s, streamsize n) { boxedStream->write((char*)s,n); }
		streamsize gcount() { return boxedStream->gcount(); }
		void str(const string & s) { boxedStream->str(s); }
		void clear() { return boxedStream->clear(); }
		bool eof() { return boxedStream->eof(); }
		bool good() { return boxedStream->good(); }
		void delSSPtr() { if (boxedStream != NULL) { delete boxedStream; } }
		virtual int Read(array<uint8>^ buffer, int offset, int count) override 
		{
			uint8* temp = new uint8[count];
			read(temp, count);
			for ( int index=0; index < count; index++ )
			{ buffer[index+offset]=temp[index]; }			
			delete[] temp;
			return boxedStream->gcount();
		}
		virtual void Write(array<uint8>^ buffer, int offset, int count) override 
		{
			uint8* temp = new uint8[count];
			for ( int index=0; index < count; index++ )
			{ temp[index]=buffer[index+offset]; }
			write(temp, count);
			delete[] temp;
		}
		virtual property bool CanRead { bool get() override { return boxedStream->good(); } }
		virtual property bool CanWrite { bool get() override { return boxedStream->good(); } }
		virtual property bool CanSeek { bool get() override { return false; } }
		virtual property long long Length { long long get() override { return boxedStream->str().length(); } }
		virtual property long long Position { long long get() override { return boxedStream->tellg(); } void set(long long value) override { } }
		virtual void Flush() override { boxedStream->flush(); }
		virtual long long Seek(long long offset, SeekOrigin origin) override { return 0; } // Do nothing
		virtual void SetLength(long long value) override { } // Do nothing
	private:
		stringstream* boxedStream;
	};

	private ref class twoBitKeystreamStack {
	public:
		~twoBitKeystreamStack();

		twoBitKeystreamStack( ECRYPT_ctx* iRabbitPRNGState );

		uint8 pop();

	private:
		List< uint8 >^ twoBitChunks;
		ECRYPT_ctx* RabbitPRNGState;
		u8 * keyStream;
	};

	private ref class oneBitIstreamStack {
	public:
		~oneBitIstreamStack();

		oneBitIstreamStack( istream& input );

		uint1 pop();
		bool buffered();

	private:
		List< uint1 >^ oneBitChunks;
		Stream^ source;
	};

#ifdef RANDOM_LOTTERY //During each pass, two numbers have half probability of being selected, which moves pseudorandomly

	private ref class oneBitKeystreamStack {
	public:
		~oneBitKeystreamStack();

		oneBitKeystreamStack( ECRYPT_ctx* iRabbitPRNGState );

		uint1 pop();

	private:
		List< uint1 >^ oneBitChunks;
		ECRYPT_ctx* RabbitPRNGState;
		u8 * keyStream;
	};

#else

	private ref class oneByteKeystreamStack {
	public:
		~oneByteKeystreamStack();

		oneByteKeystreamStack( ECRYPT_ctx* iRabbitPRNGState );

		uint8 pop();

	private:
		List< uint8 >^ byteChunks;
		ECRYPT_ctx* RabbitPRNGState;
		u8 * keyStream;
	};

#endif

	private ref class randomLottery {
	public:
		~randomLottery();

		randomLottery( uint16 count, ECRYPT_ctx* iRabbitPRNGState );

		uint16 pop();

	private:
		List< uint16 >^ lotteryBalls;
#ifdef RANDOM_LOTTERY
		oneBitKeystreamStack^ decisionSource;
#else
		oneByteKeystreamStack^ decisionSource;
#endif
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
		void buildDownTree( treebranch* cpCurrentBranch, uint16& LeafsRemaining, twoBitKeystreamStack^ cpTwoBitChunks );
		void buildUpTree( treebranch* cpTreeRoot, uint16& LeafsRemaining, twoBitKeystreamStack^ cpTwoBitChunks );
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

		void encode( uint8 datum, Out<>* bsBitBufferIn, uint16% ui16BitsBuffered );
		void decode( istream& input, Stream^ output, bool outputWrapped, uint64% ui64BytesRead );

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

		void generateGears( treenode* cpCurrentNode, randomLottery^ IndexSelector, uint8 Depth, std::vector< uint8 >& Sequence, huffman_gear** acpGearBox );
		void crank();
		void crankCascade();

	};

	private ref class engine_internal {
	public:
		engine_internal();
		~engine_internal();

		Status eStatus;
		InitType eInitType;
		DestType eDestType;

		ECRYPT_ctx* RabbitPRNGState;
		huffman_gearbox* cpHuffmanGearbox;
		stringstreamManagedWrapper^ ssBitBufferOut;
		Out<>* bsBitBufferIn;
		uint16 ui16BitsBuffered;

		Stream^ output;
		bool outputWrapped;

		uint64 ui64BitsWritten;
		uint64 ui64BytesRead;

		void flush();

	};
	


} //namespace cipup
