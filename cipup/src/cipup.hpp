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

/* Exports for CIPUP.dll */

#include "stdafx.h"

#include <string>
#include <vector>

using namespace System;
using namespace std;

namespace cipup {

	/** <summary> Initialization return codes. Signifies success or reason for failure. Consult <c>engine.Messages</c> for a description of each code. </summary> */
	static public enum class MessageCode { InitSuccess = 0, InitFailure, InitFailureInvalidAction, InitFailureNeedFinalize, InitFailureFileError, 
			InitFailureInvalidKeyLength, InitFailureInvalidIVLength };

	/** <summary> Initialization action codes. Use to specify initialization for decryption or encryption. </summary> */
	static public enum class InitAction { InitDecrypt = 0, InitEncrypt };

	/** <summary> Generation technique choices. Use to specify technique used by <c>engine.GenerateKey()</c> to generate random data. <c>LocalEntropicSecureRand</c> is the preferred choice.</summary> */
	static public enum class GenerationTechnique { LocalEntropicSecureRand = 0, LocalSecureRand, LocalJumpingSimpleRand, LocalSimpleRand };

	// Non-exposed private data members and functions
	ref class engine_internal;

	/** <summary> Cipup engine class. Provides static helper methods. Instantiate to use the engine for encryption or decryption.
		<para>CIPUP Is Privacy for the Ultra Paranoid</para>
		<para>Output can be directed to a file or stringstream. Input can be directed from a file or stream.
			Additionally during encryption, input can be provided as a single byte or vector of bytes.</para>
		<para>Use <c>init()</c> to start the engine by setting the Key, IV, de/encryption action, and output destination.
			Use <c>finalize()</c> to stop the engine, purging memory and closing output streams. You can then call <c>init()</c> again.</para>
		<para>Once the engine has been started in encryption mode, you can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
			If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
			If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.
			Note that the stringstream will be purged when calling <c>encrypt()</c> after calling <c>flush()</c>, so consume the stream first.</para>
		<para>To decrypt sequentially linked messages, call <c>decrypt()</c> for each message.
			Note that it will only decrypt one message per call, but it will consume the entire stream or file provided.
			It is therefore recommended for the input stream or file to only contain one message. Set the input marker of the stream to the appropriate beginning of the message.</para>
	</summary> */
	public ref class engine {
	public:

		/** <summary> Descriptions for the <c>MessageCode</c> returned by <c>init()</c>. Use the <c>MessageCode</c> as the index to this array. </summary> */
		static array<String^>^ Messages = { "Init Successful", "Init Failure", "Init Failure: Invalid action", "Init Failure: Already initialized, call finalize", "Init Failure: File access error", 
			"Init Failure: Invalid Key length", "Init Failure: Invalid IV length" };

		//CIPUP build information

		/**
			<summary>
			Returns the version text for CIPUP.
			<para>Different versions may or may not be compatible. Consult the build notes and CIPUP website for more information.</para>
			</summary> 
			<returns>A String containing the version text for this build of CIPIP.</returns>
		*/
		static String^ GetVersionText(void);
		/**
			<summary>
			Prints the version text for CIPUP to the console.
			<para>Different versions may or may not be compatible. Consult the build notes and CIPUP website for more information.</para>
			</summary> 
		*/
		static void PrintVersionText(void);

		/**
			<summary>
			Returns the configuration text for CIPUP.
			<para>The compilation switches that contribute to this text modify CIPUP in such a way that different configurations WILL NOT be compatible.</para>
			<para>Hence, if this text is not identical between two builds of CIPUP, they are incompatible.</para>
			</summary> 
			<returns>A String containing the configuration text for this build of CIPIP.</returns>
		*/
		static String^ GetConfiguration(void);
		/**
			<summary>
			Prints the configuration text for CIPUP to the console.
			<para>The compilation switches that contribute to this text modify CIPUP in such a way that different configurations WILL NOT be compatible.</para>
			<para>Hence, if this text is not identical between two builds of CIPUP, they are incompatible.</para>
			</summary>
		*/
		static void PrintConfiguration(void);

		//Key helper functions

		/**
			<summary>
			Returns the byte length for keys required by the PRNG that CIPUP employs.
			<para>The key provided to <c>init()</c> must be this length.</para>
			</summary> 
			<returns>A byte containing the required byte length of a key.</returns>
		*/
		static uint8 RequiredKeyByteLength(void);
		/**
			<summary>
			Returns the byte length for IVs required by the PRNG that CIPUP employs.
			<para>The IV provided to <c>init()</c> must be this length.</para>
			</summary> 
			<returns>A byte containing the required byte length of an IV.</returns>
		*/
		static uint8 RequiredIVByteLength(void);

		/**
			<summary>
			Generates a new random Key or IV into the key array. Uses the <c>LocalEntropicSecureRand</c> technique by default.
			</summary> 
			<param name="key">Byte array to be filled with generated data.</param>
			<param name="keybytelen">Length of the array in bytes. Recommended values can be found by calling <c>RequiredKeyByteLength()</c> and <c>RequiredIVByteLength()</c>.</param>
		*/
		static void GenerateKey(array<uint8>^ key, uint8 keybytelen); 
		/**
			<summary>
			Generates a new random Key or IV into the key array. Uses the <c>LocalEntropicSecureRand</c> technique by default.
			<para>The key array will be allocated if null.</para>
			</summary> 
			<param name="key">Byte array to be filled with generated data.</param>
			<param name="keybytelen">Length of the array in bytes. Recommended values can be found by calling <c>RequiredKeyByteLength()</c> and <c>RequiredIVByteLength()</c>.</param>
		*/
		static void GenerateKey(uint8*& key, uint8 keybytelen); //key array will be allocated if null.
		/** 
			<summary>
			Generates a new random Key or IV into the key array.
			</summary> 
			<param name="key">Byte array to be filled with generated data.</param>
			<param name="keybytelen">Length of the array in bytes. Recommended values can be found by calling <c>RequiredKeyByteLength()</c> and <c>RequiredIVByteLength()</c>.</param>
			<param name="techchoice">Enum <c>GenerationTechnique</c> specifies the technique used to generate random data. <c>LocalEntropicSecureRand</c> is the preferred choice.</param>
		*/
		static void GenerateKey(array<uint8>^ key, uint8 keybytelen, GenerationTechnique techchoice);
		/** 
			<summary>
			Generates a new random Key or IV into the key array.
			<para>The key array will be allocated if null.</para>
			</summary> 
			<param name="key">Byte array to be filled with generated data.</param>
			<param name="keybytelen">Length of the array in bytes. Recommended values can be found by calling <c>RequiredKeyByteLength()</c> and <c>RequiredIVByteLength()</c>.</param>
			<param name="techchoice">Enum <c>GenerationTechnique</c> specifies the technique used to generate random data. <c>LocalEntropicSecureRand</c> is the preferred choice.</param>
		*/
		static void GenerateKey(uint8*& key, uint8 keybytelen, GenerationTechnique techchoice);

		/**
			<summary>
			Instantiates the engine, allowing access to non-static member methods.
			<para>You must call <c>init()</c> before you can call <c>encrypt()</c> or <c>decrypt()</c>.</para>
			</summary>
		*/
		engine(void);
		/**
			<summary>
			Destroys the engine.
			<para>Automatically calls <c>finalize()</c>.</para>
			</summary>
		*/
		~engine(void);

		/** 
			<summary>
			Initializes the engine for encryption or decryption to a stringstream with a Key and IV.
			<para>There is an overloaded method for writing to a file.</para>
			</summary> 
			<param name="action">Enum <c>InitAction</c> specifing initialization for encryption or decryption.</param>
			<param name="output">Output will be directed to this stringstream.</param>
			<param name="key">Byte array containing the Key.</param>
			<param name="keybytelen">Length of the Key array in bytes. Required values can be found by calling <c>RequiredKeyByteLength()</c>.</param>
			<param name="iv">Byte array containing the IV.</param>
			<param name="ivbytelen">Length of the IV array in bytes. Required values can be found by calling <c>RequiredIVByteLength()</c>.</param>
			<returns>A MessageCode signifying success or reason for failure. Consult <c>engine.Messages</c> for a description of each code.</returns>
		*/
		MessageCode init( InitAction action, IO::Stream^ output, array<uint8>^ key, uint8 keybytelen, array<uint8>^ iv, uint8 ivbytelen );
		/** 
			<summary>
			Initializes the engine for encryption or decryption to a stringstream with a Key and IV.
			<para>There is an overloaded method for writing to a file.</para>
			</summary> 
			<param name="action">Enum <c>InitAction</c> specifing initialization for encryption or decryption.</param>
			<param name="output">Output will be directed to this stringstream.</param>
			<param name="key">Byte array containing the Key.</param>
			<param name="keybytelen">Length of the Key array in bytes. Required values can be found by calling <c>RequiredKeyByteLength()</c>.</param>
			<param name="iv">Byte array containing the IV.</param>
			<param name="ivbytelen">Length of the IV array in bytes. Required values can be found by calling <c>RequiredIVByteLength()</c>.</param>
			<returns>A MessageCode signifying success or reason for failure. Consult <c>engine.Messages</c> for a description of each code.</returns>
		*/
		MessageCode init( InitAction action, ostringstream* output, uint8* key, uint8 keybytelen, uint8* iv, uint8 ivbytelen );
		/** 
			<summary>
			Initializes the engine for encryption or decryption to a file with a Key and IV.
			<para>There is an overloaded method for writing to a stringstream.</para>
			<para>This method will open the file designated, and append to an existing file if so directed.</para>
			</summary> 
			<param name="action">Enum <c>InitAction</c> specifing initialization for encryption or decryption.</param>
			<param name="filename">Output will be directed to the file designated by this path.</param>
			<param name="append">Specifies wheter to append to the file.</param>
			<param name="key">Byte array containing the Key.</param>
			<param name="keybytelen">Length of the Key array in bytes. Required values can be found by calling <c>RequiredKeyByteLength()</c>.</param>
			<param name="iv">Byte array containing the IV.</param>
			<param name="ivbytelen">Length of the IV array in bytes. Required values can be found by calling <c>RequiredIVByteLength()</c>.</param>
			<returns>A MessageCode signifying success or reason for failure. Consult <c>engine.Messages</c> for a description of each code.</returns>
		*/
		MessageCode init( InitAction action, const char* filename, bool append, array<uint8>^ key, uint8 keybytelen, array<uint8>^ iv, uint8 ivbytelen );
		/** 
			<summary>
			Initializes the engine for encryption or decryption to a file with a Key and IV.
			<para>There is an overloaded method for writing to a stringstream.</para>
			<para>This method will open the file designated, and append to an existing file if so directed.</para>
			</summary> 
			<param name="action">Enum <c>InitAction</c> specifing initialization for encryption or decryption.</param>
			<param name="filename">Output will be directed to the file designated by this path.</param>
			<param name="append">Specifies wheter to append to the file.</param>
			<param name="key">Byte array containing the Key.</param>
			<param name="keybytelen">Length of the Key array in bytes. Required values can be found by calling <c>RequiredKeyByteLength()</c>.</param>
			<param name="iv">Byte array containing the IV.</param>
			<param name="ivbytelen">Length of the IV array in bytes. Required values can be found by calling <c>RequiredIVByteLength()</c>.</param>
			<returns>A MessageCode signifying success or reason for failure. Consult <c>engine.Messages</c> for a description of each code.</returns>
		*/
		MessageCode init( InitAction action, const char* filename, bool append, uint8* key, uint8 keybytelen, uint8* iv, uint8 ivbytelen );
		/**
			<summary>
			Returns the readiness of the engine.
			<para>TRUE if the engine has been initialized because <c>init()</c> has been called successfully more recently than <c>finalize</c>.</para>
			<para>FALSE if the engine has not been initialized because <c>init()</c> has never been successfully called or <c>finalize</c> has been called more recently than a successful <c>init()</c>.</para>
			<para>If TRUE, then <c>encrypt()</c> or <c>decrypt()</c> can be called, depending on which has been initialized. Use <c>canread()</c> and <c>canwrite()</c> to determine which action can be performed.</para>
			</summary> 
			<returns>A boolean designating the readiness of the engine.</returns>
		*/
		bool ready();
		
		/**
			<summary>
			Returns the readiness of the engine for decryption.
			<para>TRUE if the engine has been initialized for decryption and it is ready.</para>
			<para>FALSE otherwise.</para>
			<para>If TRUE, then <c>decrypt()</c> can be called.</para>
			</summary> 
			<returns>A boolean designating if the engine is ready for decryption.</returns>
		*/
		bool canread();
		/**
			<summary>
			Decrypts the contents of a stream to the initialized output.
			<para>Consumes the entire stream, but only decrypts one (the first) message.</para>
			<para>If the stream ends before a terminal character in the message is encountered, 
				then either the message will be cut off or an exception will be thrown.</para>
			<para>Use <c>canread()</c> to determine if this action can be performed.</para>
			<para>Use <c>bytesread()</c> for the number of bytes of plaintext decrypted from the cyphertext.</para>
			</summary>
			<param name="input">Stream of cyphertext containing one message.</param>
			<exception cref="exception">Thrown when the stream ends before a terminal character in the message is encountered, 
				if the last bits were part of a character's encoding.</exception>
		*/
		void decrypt( IO::Stream^ input );
		/**
			<summary>
			Decrypts the contents of a stream to the initialized output.
			<para>Consumes the entire stream, but only decrypts one (the first) message.</para>
			<para>If the stream ends before a terminal character in the message is encountered, 
				then either the message will be cut off or an exception will be thrown.</para>
			<para>Use <c>canread()</c> to determine if this action can be performed.</para>
			<para>Use <c>bytesread()</c> for the number of bytes of plaintext decrypted from the cyphertext.</para>
			</summary>
			<param name="input">istream of cyphertext containing one message.</param>
			<exception cref="Exception">Thrown when the stream ends before a terminal character in the message is encountered, 
				if the last bits were part of a character's encoding.</exception>
		*/
		void decrypt( istream& input );
		/**
			<summary>
			Decrypts the contents of a stream to the initialized output.
			<para>Consumes the entire stream, but only decrypts one (the first) message.</para>
			<para>If the stream ends before a terminal character in the message is encountered, 
				then either the message will be cut off or an exception will be thrown.</para>
			<para>Use <c>canread()</c> to determine if this action can be performed.</para>
			<para>Use <c>bytesread()</c> for the number of bytes of plaintext decrypted from the cyphertext.</para>
			</summary>
			<param name="input">Stream of cyphertext containing one message.</param>
			<exception cref="Exception">Thrown when the stream ends before a terminal character in the message is encountered, 
				if the last bits were part of a character's encoding.</exception>
		*/
		static void operator>> ( engine% a, IO::Stream^ input );
		/**
			<summary>
			Decrypts the contents of a stream to the initialized output.
			<para>Consumes the entire stream, but only decrypts one (the first) message.</para>
			<para>If the stream ends before a terminal character in the message is encountered, 
				then either the message will be cut off or an exception will be thrown.</para>
			<para>Use <c>canread()</c> to determine if this action can be performed.</para>
			<para>Use <c>bytesread()</c> for the number of bytes of plaintext decrypted from the cyphertext.</para>
			</summary>
			<param name="input">istream of cyphertext containing one message.</param>
			<exception cref="Exception">Thrown when the stream ends before a terminal character in the message is encountered, 
				if the last bits were part of a character's encoding.</exception>
		*/
		static void operator>> ( engine% a, istream& input );

		/**
			<summary>
			Returns the number of post-decryption bytes written to the output.
			<para>This is the number of bytes of plaintext decrypted from the cyphertext.</para>
			</summary> 
			<returns>A 64-bit unsigned integer containing the number of post-decryption bytes written to the output.</returns>
		*/
		uint64 bytesread();

		/**
			<summary>
			Returns the readiness of the engine for encryption.
			<para>TRUE if the engine has been initialized for encryption and it is ready.</para>
			<para>FALSE otherwise.</para>
			<para>If TRUE, then <c>encrypt()</c> can be called.</para>
			</summary> 
			<returns>A boolean designating if the engine is ready for encryption.</returns>
		*/
		bool canwrite();
		/**
			<summary>
			Encrypts a single byte to the initialized output.
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="datum">Byte of plaintext</param>
		*/
		void encrypt( uint8 datum );
		/**
			<summary>
			Encrypts a single byte to the initialized output.
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="datum">Pointer to byte of plaintext</param>
		*/
		void encrypt( uint8* datum );
		/**
			<summary>
			Encrypts a single byte to the initialized output.
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="datum">Pointer to byte of plaintext</param>
		*/
		static void operator<< ( engine% a, uint8* datum );
		/**
			<summary>
			Encrypts a single byte to the initialized output.
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="datum">Byte of plaintext</param>
		*/
		static void operator<< ( engine% a, uint8& datum );
		/**
			<summary>
			Encrypts the contents of a vector of bytes to the initialized output.
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="data">Byte array of plaintext</param>
		*/
		void encrypt( array<uint8>^ data );
		/**
			<summary>
			Encrypts the contents of a vector of bytes to the initialized output.
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="data">Vector of bytes of plaintext</param>
		*/
		void encrypt( std::vector< uint8 >& data );
		/**
			<summary>
			Encrypts the contents of a vector of bytes to the initialized output.
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="data">Byte array of plaintext</param>
		*/
		static void operator<< ( engine% a, array<uint8>^ data );
		/**
			<summary>
			Encrypts the contents of a vector of bytes to the initialized output.
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="data">Vector of bytes of plaintext</param>
		*/
		static void operator<< ( engine% a, std::vector< uint8 >& data );
		/**
			<summary>
			Encrypts the contents of a stream to the initialized output.
			<para>Consumes the entire stream, building into the current (single) message.</para>
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="input">Stream of plaintext</param>			
		*/
		void encrypt( IO::Stream^ input );
		/**
			<summary>
			Encrypts the contents of a stream to the initialized output.
			<para>Consumes the entire stream, building into the current (single) message.</para>
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="input">istream of plaintext</param>			
		*/
		void encrypt( istream& input );
		/**
			<summary>
			Encrypts the contents of a stream to the initialized output.
			<para>Consumes the entire stream, building into the current (single) message.</para>
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="input">Stream of plaintext</param>
		*/
		static void operator<< ( engine% a, IO::Stream^ input );
		/**
			<summary>
			Encrypts the contents of a stream to the initialized output.
			<para>Consumes the entire stream, building into the current (single) message.</para>
			<para>You can incrementally encrypt messages by feeding <c>encrypt()</c> data chunks.
				If you are writing to a stringstream, you can optionally terminate a message using <c>flush()</c>.
				If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>canwrite()</c> to determine if this action can be performed.</para>
			<para>Use <c>bitswritten()</c> for the number of bits of cyphertext generated from the plaintext (non-buffered bits already written to the output).</para>
			</summary>
			<param name="input">istream of plaintext</param>
		*/
		static void operator<< ( engine% a, istream& input );
		/**
			<summary>
			Terminates and flushes the current message to the output, writing any buffered bits with any required padding and a terminal character.
			<para>Can only be called when encrypting to a stringstream.</para>
			<para>If you want additional messages sequentially linked, you can reuse <c>encrypt()</c> after calling <c>flush()</c>.</para>
			<para>Use <c>bitswritten()</c> for the total number of bits written to the output.</para>
			</summary>
		*/
		void flush();

		/**
			<summary>
			Returns the number of post-encryption bits written to the output.
			<para>Does not include terminal padding.</para>
			</summary> 
			<returns>A 64-bit unsigned integer containing the number of post-encryption bits written to the output. Does not include terminal padding.</returns>
		*/
		uint64 bitswritten();
		/**
			<summary>
			Returns the number of post-encryption bytes written to the output.
			<para>Does not include overflow bits or terminal padding, so the actual number of bytes written after a call to <c>flush()</c> or <c>finalize()</c> may be 1 larger.
				See <c>overflowbits()</c> for the number of overflow bits. If this value is non-zero after a call to <c>flush()</c> or <c>finalize()</c>, then the
				number of bytes written is 1 larger.</para>
			</summary> 
			<returns>A 64-bit unsigned integer containing the number of post-encryption bytes written to the output. Does not include overflow bits or terminal padding.</returns>
		*/
		uint64 byteswritten();
		/**
			<summary>
			Returns the number of post-encryption overflow bits written to the output.
			<para>This value will remain zero until after a call to <c>flush()</c> or <c>finalize()</c>.</para>
			<para>These bits recieved terminal padding to be writen as a full byte. This value will never be greater than 7.</para>
			<para>If this value is non-zero after a call to <c>flush()</c> or <c>finalize()</c>, then the
				number of bytes written is 1 larger than the value returned by <c>byteswritten()</c>.</para>
			</summary> 
			<returns>A byte containing the number of post-encryption overflow bits.</returns>
		*/
		uint8 overflowbits();

		/**
			<summary>
			Terminates and flushes the current message to the output, and destroys the internal state of the engine.
			<para>If encrypting, writes any buffered bits with any required padding and a terminal character.</para>
			<para>If outputting to a file, closes the file.</para>
			<para>If encrypting, use <c>bitswritten()</c> for the total number of bits of cyphertext generated from the plaintext and written to the output.</para>
			<para>If decrypting, use <c>bytesread()</c> for the number of bytes of plaintext decrypted from the cyphertext and written to the output.</para>
			<para>Use <c>init()</c> again to restart the engine by setting the Key, IV, de/encryption action, and output destination.</para>
			</summary>
		*/
		void finalize();
		
	private:
		engine_internal^ cpInternal;
	};


} //namespace cipup
