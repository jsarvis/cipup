/*=============================================================
   Code Copyright 2011 Jacob Sarvis
 
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
using System;
using System.IO;
using NUnit.Framework;


namespace cipup_external_test
{
    [TestFixture]
    class Program
    {
        private static int TestMessageLen = 10000;

        private static void AssertEquals(byte[] a1, byte[] a2)
        {
            if (a1 == a2)
            {
                return;
            }
            Assert.True((a1 != null) && (a2 != null), "Mismatched arrays, Null.");

            Assert.AreEqual(a1.Length, a2.Length, "Mismatched arrays, length mismatch.");
            for (int i = 0; i < a1.Length; i++)
            {
                Assert.AreEqual(a1[i], a2[i], "Mismatched arrays, value mismatch.");
            }
        }

        [Test]
        public void GetVersionTextTest()
        {
            Console.WriteLine(cipup.engine.GetVersionText());
        }

        [Test]
        public void PrintVersionTextTest()
        {
            cipup.engine.PrintVersionText();
        }

        [Test]
        public void GetConfigurationTextTest()
        {
            Console.WriteLine(cipup.engine.GetConfiguration());
        }

        [Test]
        public void PrintConfigurationTextTest()
        {
            cipup.engine.PrintConfiguration();
        }

        [Test]
        public void RequiredKeyByteLengthTest()
        {
            Console.WriteLine(cipup.engine.RequiredKeyByteLength());
        }

        [Test]
        public void RequiredIVByteLengthTest()
        {
            Console.WriteLine(cipup.engine.RequiredIVByteLength());
        }

        [Test]
        public void GenerateKeyTest()
        {
            byte len = cipup.engine.RequiredIVByteLength();
            Console.Write("Default IV=");
            GenAndPrint(len);
            len = cipup.engine.RequiredKeyByteLength();
            Console.Write("Defauly Key=");
            GenAndPrint(len);
            Console.Write("LocalSimpleRand Key=");
            GenAndPrint(len, cipup.GenerationTechnique.LocalSimpleRand);
            Console.Write("LocalJumpingSimpleRand Key=");
            GenAndPrint(len, cipup.GenerationTechnique.LocalJumpingSimpleRand);
            Console.Write("LocalSecureRand Key=");
            GenAndPrint(len, cipup.GenerationTechnique.LocalSecureRand);
            Console.Write("LocalEntropicSecureRand Key=");
            GenAndPrint(len, cipup.GenerationTechnique.LocalEntropicSecureRand);
            byte[] temp = null;
            cipup.engine.GenerateKey(temp, 2);
         }

        private static void GenAndPrint(byte len)
        {
            byte[] key = new byte[len];
            cipup.engine.GenerateKey(key, len);
            Console.Write(len + ":");
            foreach (byte b in key)
            {
                Console.Write(b + "-");
            }
            Console.WriteLine();
        }

        private static void GenAndPrint(byte len, cipup.GenerationTechnique tech)
        {
            byte[] key = new byte[len];
            cipup.engine.GenerateKey(key, len, tech);
            Console.Write(len + ":");
            foreach (byte b in key)
            {
                Console.Write(b + "-");
            }
            Console.WriteLine();
        }

        [Test]
        public void InstantiationTest()
        {
            cipup.engine alpha = new cipup.engine();
            alpha.Dispose(); //Manualy destruct
            cipup.engine beta = new cipup.engine(); //GC
        }

        [Test]
        public void UnInitializedTest()
        {
            cipup.engine alpha = new cipup.engine();
            Assert.AreEqual(0,alpha.bitswritten());
            Assert.AreEqual(0,alpha.overflowbits());
            Assert.AreEqual(0,alpha.byteswritten());
            Assert.AreEqual(0,alpha.bytesread());
            Assert.False(alpha.canread());
            Assert.False(alpha.canwrite());
            Assert.False(alpha.ready());

            alpha.encrypt(0);
            byte[] temp = new byte[1];
            alpha.encrypt(temp);
            Stream temp2 = null;
            alpha.encrypt(temp2);

            alpha.decrypt(temp2);
        }
        
        [Test]
        public void InitializationMemoryStreamEncryptSuccessTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            MemoryStream dest = new MemoryStream();

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitEncrypt, dest, key, keylen, iv, ivlen);

            Console.WriteLine("Encrypt Memorystream Init returned: " + status + " = " +
                              cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);
        }


        [Test]
        public void InitializationMemoryStreamDecryptSuccessTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            MemoryStream dest = new MemoryStream();

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitDecrypt, dest, key, keylen, iv, ivlen);

            Console.WriteLine("Decrypt Memorystream Init returned: " + status + " = " +
                              cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);
        }

        [Test]
        public void InitializationFileStreamEncryptSuccessTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            FileStream dest2 = new FileStream("testDoc-InitializationFileStreamEncryptSuccessTest.txt", FileMode.Create);

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();
            status = alpha.init(cipup.InitAction.InitEncrypt, dest2, key, keylen, iv, ivlen);

            Console.WriteLine("Encrypt Filestream Init returned: " + status + " = " +
                              cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            alpha.finalize();
            alpha.Dispose();
            dest2.Close();
            File.Delete(dest2.Name);
        }

        [Test]
        public void InitializationFileStreamDecryptSuccessTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            FileStream dest2 = new FileStream("testDoc-InitializationFileStreamDecryptSuccessTest.txt", FileMode.Create);

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();
            status = alpha.init(cipup.InitAction.InitDecrypt, dest2, key, keylen, iv, ivlen);

            Console.WriteLine("Decrypt Filestream Init returned: " + status + " = " + cipup.engine.Messages[(int)status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            alpha.finalize();
            alpha.Dispose();

            dest2.Close();
            File.Delete(dest2.Name);
        }

        [Test]
        public void InitializationFileEncryptSuccessTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitEncrypt, "testDoc-InitializationFileEncryptSuccessTest.txt", false, key, keylen, iv, ivlen);

            Console.WriteLine("Encrypt file Init returned: " + status + " = " + cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            alpha.finalize();
            alpha.Dispose();

            File.Delete("testDoc-InitializationFileEncryptSuccessTest.txt");
        }

        [Test]
        public void InitializationFileDecryptSuccessTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitDecrypt, "testDoc-InitializationFileDecryptSuccessTest.txt", false, key, keylen, iv, ivlen);

            Console.WriteLine("Decrypt file Init returned: " + status + " = " + cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            alpha.finalize();
            alpha.Dispose();

            File.Delete("testDoc-InitializationFileDecryptSuccessTest.txt");
        }

        [Test]
        public void InitializationFileAppendEncryptSuccessTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitEncrypt, "testDoc-InitializationFileAppendEncryptSuccessTest.txt", true, key, keylen, iv, ivlen);

            Console.WriteLine("Encrypt file append Init returned: " + status + " = " +
                              cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            alpha.finalize();
            alpha.Dispose();

            File.Delete("testDoc-InitializationFileAppendEncryptSuccessTest.txt");
        }

        [Test]
        public void InitializationFileAppendDecryptSuccessTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitDecrypt, "testDoc-InitializationFileAppendDecryptSuccessTest.txt", true, key, keylen, iv, ivlen);

            Console.WriteLine("Decrypt file append Init returned: " + status + " = " + cipup.engine.Messages[(int)status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            alpha.finalize();
            alpha.Dispose();

            File.Delete("testDoc-InitializationFileAppendDecryptSuccessTest.txt");
        }

        [Test]
        public void InitializationStreamFailure1Test()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];

            MemoryStream dest = new MemoryStream();
            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitEncrypt, dest, key, keylen, iv, ivlen);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            status = alpha.init(cipup.InitAction.InitDecrypt, dest, key, keylen, iv, ivlen);

            Console.WriteLine("Init returned: " + status + " = " + cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitFailureNeedFinalize, status);

        }

        [Test]
        public void InitializationStreamFailure2Test()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];

            MemoryStream dest = new MemoryStream();
            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();
            status = alpha.init(cipup.InitAction.InitDecrypt, dest, key, 1, iv, ivlen);

            Console.WriteLine("Init returned: " + status + " = " + cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitFailureInvalidKeyLength, status);
        }

        [Test]
        public void InitializationStreamFailure3Test()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];

            MemoryStream dest = new MemoryStream();
            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitDecrypt, dest, key, keylen, iv, 1);

            Console.WriteLine("Init returned: " + status + " = " + cipup.engine.Messages[(int) status]);
            Assert.AreEqual(cipup.MessageCode.InitFailureInvalidIVLength, status);
        }

        [Test]
        public void InitializationStreamFailure4Test()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];

            MemoryStream dest = new MemoryStream();
            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init((cipup.InitAction)2, dest, key, keylen, iv, ivlen);

            Console.WriteLine("Init returned: " + status + " = " + cipup.engine.Messages[(int)status]);
            Assert.AreEqual(cipup.MessageCode.InitFailureInvalidAction, status);
        }

        [Test]
        public void InitializationFileFailureTest()
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            byte[] key = new byte[keylen];
            byte[] iv = new byte[keylen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(cipup.InitAction.InitEncrypt, "", false, key, keylen, iv, ivlen);

            Console.WriteLine("Init returned: " + status + " = " + cipup.engine.Messages[(int)status]);
            Assert.AreEqual(cipup.MessageCode.InitFailureFileError, status);

            status = alpha.init(cipup.InitAction.InitEncrypt, "?", false, key, keylen, iv, ivlen);

            Console.WriteLine("Init returned: " + status + " = " + cipup.engine.Messages[(int)status]);
            Assert.AreEqual(cipup.MessageCode.InitFailureFileError, status);
        }

        private static byte[] QuietStreamToStreamTest(cipup.InitAction action, byte[] key, byte keylen, byte[] iv, byte ivlen, byte[] srctext, int messagelen, Stream src, Stream dest)
        {
            long srcPos = src.Position;
            src.Write(srctext, 0, messagelen);
            src.Position = srcPos;

            long destPos = dest.Position;

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(action, dest, key, keylen, iv, ivlen);

            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            ulong destlen;

            if (action == cipup.InitAction.InitEncrypt)
            {
                alpha.encrypt(src);
                alpha.flush();
                alpha.finalize();
                destlen = alpha.byteswritten() + (ulong)((alpha.overflowbits() > 0) ? 1 : 0);
            }
            else if (action == cipup.InitAction.InitDecrypt)
            {
                alpha.decrypt(src);
                alpha.finalize();
                destlen = alpha.bytesread();
            }
            else
            {
                throw new Exception("Invalid action!");
            }

            byte[] desttext = new byte[destlen];
            dest.Position = destPos;
            dest.Read(desttext, 0, (int)destlen);

            return desttext;
        }

        private static byte[] StreamToStreamTest(cipup.InitAction action, byte[] key, byte keylen, byte[] iv, byte ivlen, byte[] srctext, int messagelen, Stream src, Stream dest)
        {
            Console.WriteLine("Source text:");
            for (int itr = 0, stop = (messagelen<20)?messagelen : 20; itr < stop; itr++)
            {
                Console.Write(srctext[itr] + "-");
            }
            Console.WriteLine("... Total of " + messagelen + " bytes.");

            long srcPos = src.Position;
            src.Write(srctext, 0, messagelen);
            src.Position = srcPos;

            long destPos = dest.Position;

            cipup.MessageCode status;

            cipup.engine alpha = new cipup.engine();

            status = alpha.init(action, dest, key, keylen, iv, ivlen);

            Console.WriteLine(action+" stream Init returned: " + status + " = " +
                              cipup.engine.Messages[(int)status]);
            Assert.AreEqual(cipup.MessageCode.InitSuccess, status);

            ulong destlen;

            if ( action == cipup.InitAction.InitEncrypt )
            {
                alpha.encrypt(src);
                Console.WriteLine("Pre-flush bits written: " + alpha.bitswritten());
                alpha.flush();
                Console.WriteLine("Post-flush bits written: " + alpha.bitswritten());
                alpha.finalize();
                Console.WriteLine("Post-finalize bits written: " + alpha.bitswritten());
                Console.WriteLine("Post-finalize bytes written: " + alpha.byteswritten());
                Console.WriteLine("Post-finalize overflow bits written: " + alpha.overflowbits());
                destlen = alpha.byteswritten() + (ulong)((alpha.overflowbits() > 0) ? 1 : 0);
            }
            else if (action == cipup.InitAction.InitDecrypt)
            {
                alpha.decrypt(src);
                Console.WriteLine("Pre-finalize bytes read: " + alpha.bytesread());
                alpha.finalize();
                Console.WriteLine("Post-finalize bytes read: " + alpha.bytesread());
                destlen = alpha.bytesread();
            }
            else
            {
                throw new Exception("Invalid action!");
            }

            byte[] desttext = new byte[destlen];
            dest.Position = destPos;
            dest.Read(desttext, 0, (int)destlen);

            Console.WriteLine("Result text:");
            for (int itr = 0, stop = (destlen < 20) ? (int)destlen : 20; itr < stop; itr++)
            {
                Console.Write(desttext[itr] + "-");
            }
            Console.WriteLine("... Total of " + destlen + " bytes.");
            return desttext;
        }

        private static void StreamToStreamEncryptRandKeyRandIVRandMessageTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);
            cipup.engine.GenerateKey(plaintext, (uint)messagelen);

            StreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);
        }

        private static void StreamToStreamEncryptRandKeyRandIVTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);
            for (int itr = 0; itr < messagelen; itr++)
            {
                plaintext[itr] = (byte)(itr % 256);
            }

            StreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);
        }

        private static void StreamToStreamEncryptRandIVTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
            for (int itr = 0; itr < keylen; itr++)
            {
                key[itr] = 0;
            }
            cipup.engine.GenerateKey(iv, ivlen);
            for (int itr = 0; itr < messagelen; itr++)
            {
                plaintext[itr] = (byte)(itr % 256);
            }

            StreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);
        }

        private static void StreamToStreamEncryptTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
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
                plaintext[itr] = (byte)(itr % 256);
            }

            StreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);
        }


        private static ulong StatisticalStreamToStreamRandKeyRandIVRandMessageFullTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);
            cipup.engine.GenerateKey(plaintext, (uint)messagelen);

            byte[] cryptext = QuietStreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);

            src.SetLength(0);
            dest.SetLength(0);

            byte[] resulttext = QuietStreamToStreamTest(cipup.InitAction.InitDecrypt, key, keylen, iv, ivlen, cryptext, cryptext.Length, src, dest);
            AssertEquals(plaintext, resulttext);

            return (ulong)(cryptext.LongLength);
        }

        private static void StreamToStreamRandKeyRandIVRandMessageFullTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);
            cipup.engine.GenerateKey(plaintext, (uint)messagelen);

            byte[] cryptext = StreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);

            src.SetLength(0);
            dest.SetLength(0);

            byte[] resulttext = StreamToStreamTest(cipup.InitAction.InitDecrypt, key, keylen, iv, ivlen, cryptext, cryptext.Length, src, dest);
            AssertEquals(plaintext, resulttext);
        }

        private static void StreamToStreamRandKeyRandIVFullTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
            cipup.engine.GenerateKey(key, keylen);
            cipup.engine.GenerateKey(iv, ivlen);
            for (int itr = 0; itr < messagelen; itr++)
            {
                plaintext[itr] = (byte)(itr % 256);
            }

            Console.Write("byte[] key = { ");
            for (int itr = 0; itr < keylen-1; itr++)
            {
                Console.Write("0x{0:X}, ", key[itr]);
            }
            Console.WriteLine("0x{0:X} }};", key[keylen - 1]);

            Console.Write("byte[] iv = { ");
            for (int itr = 0; itr < ivlen - 1; itr++)
            {
                Console.Write("0x{0:X}, ", iv[itr]);

            }
            Console.WriteLine("0x{0:X} }};", iv[ivlen - 1]);

            byte[] cryptext = StreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);

            src.SetLength(0);
            dest.SetLength(0);

            byte[] resulttext = StreamToStreamTest(cipup.InitAction.InitDecrypt, key, keylen, iv, ivlen, cryptext, cryptext.Length, src, dest);
            AssertEquals(plaintext, resulttext);
        }

        private static void StreamToStreamRandIVFullTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
            for (int itr = 0; itr < keylen; itr++)
            {
                key[itr] = 0;
            }
            cipup.engine.GenerateKey(iv, ivlen);
            for (int itr = 0; itr < messagelen; itr++)
            {
                plaintext[itr] = (byte)(itr % 256);
            }

            Console.Write("byte[] iv = { ");
            for (int itr = 0; itr < ivlen - 1; itr++)
            {
                Console.Write("0x{0:X}, ", iv[itr]);

            }
            Console.WriteLine("0x{0:X} }};", iv[ivlen - 1]);

            byte[] cryptext = StreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);

            src.SetLength(0);
            dest.SetLength(0);

            byte[] resulttext = StreamToStreamTest(cipup.InitAction.InitDecrypt, key, keylen, iv, ivlen, cryptext, cryptext.Length, src, dest);
            AssertEquals(plaintext, resulttext);
        }

        private static void StreamToStreamFullTest(Stream src, Stream dest)
        {
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte ivlen = cipup.engine.RequiredIVByteLength();
            int messagelen = TestMessageLen;
            byte[] key = new byte[keylen];
            byte[] iv = new byte[ivlen];
            byte[] plaintext = new byte[messagelen];
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
                plaintext[itr] = (byte)(itr % 256);
            }

            byte[] cryptext = StreamToStreamTest(cipup.InitAction.InitEncrypt, key, keylen, iv, ivlen, plaintext, messagelen, src, dest);
            
            src.SetLength(0);
            dest.SetLength(0);

            byte[] resulttext = StreamToStreamTest(cipup.InitAction.InitDecrypt, key, keylen, iv, ivlen, cryptext, cryptext.Length, src, dest);
            AssertEquals(plaintext, resulttext);
        }

        [Test]
        public void MemoryStreamToMemoryStreamEncryptRandKeyRandIVRandMessageTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();
            StreamToStreamEncryptRandKeyRandIVRandMessageTest(src, dest);
        }

        [Test]
        public void MemoryStreamToMemoryStreamEncryptRandKeyRandIVTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();
            StreamToStreamEncryptRandKeyRandIVTest(src, dest);
        }

        [Test]
        public void MemoryStreamToMemoryStreamEncryptRandIVTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();
            StreamToStreamEncryptRandIVTest(src, dest);
        }

        [Test]
        public void MemoryStreamToMemoryStreamEncryptTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();
            StreamToStreamEncryptTest(src, dest);
        }

        [Test]
        public void MemoryStreamToMemoryStreamRandKeyRandIVRandMessageFullTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();
            StreamToStreamRandKeyRandIVRandMessageFullTest(src, dest);
        }

        [Test]
        public void MemoryStreamToMemoryStreamRandKeyRandIVFullTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();
            StreamToStreamRandKeyRandIVFullTest(src, dest);
        }

        [Test]
        public void MemoryStreamToMemoryStreamRandIVFullTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();
            StreamToStreamRandIVFullTest(src, dest);
        }

        [Test]
        public void MemoryStreamToMemoryStreamFullTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();
            StreamToStreamFullTest(src, dest);
        }
/*
        [Test]
        public void FileStreamToFileStreamEncryptRandKeyRandIVRandMessageTest()
        {
            FileStream src = new FileStream("testDoc-I-FileStreamToFileStreamEncryptRandKeyRandIVRandMessageTest.txt", FileMode.Create);
            FileStream dest = new FileStream("testDoc-O-FileStreamToFileStreamEncryptRandKeyRandIVRandMessageTest.txt", FileMode.Create);
            StreamToStreamEncryptRandKeyRandIVRandMessageTest(src, dest);
            src.Close();
            File.Delete(src.Name);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void FileStreamToFileStreamEncryptRandKeyRandIVTest()
        {
            FileStream src = new FileStream("testDoc-I-FileStreamToFileStreamEncryptRandKeyRandIVTest.txt", FileMode.Create);
            FileStream dest = new FileStream("testDoc-O-FileStreamToFileStreamEncryptRandKeyRandIVTest.txt", FileMode.Create);
            StreamToStreamEncryptRandKeyRandIVTest(src, dest);
            src.Close();
            File.Delete(src.Name);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void FileStreamToFileStreamEncryptRandIVTest()
        {
            FileStream src = new FileStream("testDoc-I-FileStreamToFileStreamEncryptRandIVTest.txt", FileMode.Create);
            FileStream dest = new FileStream("testDoc-O-FileStreamToFileStreamEncryptRandIVTest.txt", FileMode.Create);
            StreamToStreamEncryptRandIVTest(src, dest);
            src.Close();
            File.Delete(src.Name);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void FileStreamToFileStreamEncryptTest()
        {
            FileStream src = new FileStream("testDoc-I-FileStreamToFileStreamEncryptTest.txt", FileMode.Create);
            FileStream dest = new FileStream("testDoc-O-FileStreamToFileStreamEncryptTest.txt", FileMode.Create);
            StreamToStreamEncryptTest(src, dest);
            src.Close();
            File.Delete(src.Name);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void FileStreamToFileStreamRandKeyRandIVRandMessageFullTest()
        {
            FileStream src = new FileStream("testDoc-I-FileStreamToFileStreamEncryptRandKeyRandIVRandMessageFullTest.txt", FileMode.Create);
            FileStream dest = new FileStream("testDoc-O-FileStreamToFileStreamEncryptRandKeyRandIVRandMessageFullTest.txt", FileMode.Create);
            StreamToStreamRandKeyRandIVRandMessageFullTest(src, dest);
            src.Close();
            File.Delete(src.Name);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void FileStreamToFileStreamRandKeyRandIVFullTest()
        {
            FileStream src = new FileStream("testDoc-I-FileStreamToFileStreamEncryptRandKeyRandIVFullTest.txt", FileMode.Create);
            FileStream dest = new FileStream("testDoc-O-FileStreamToFileStreamEncryptRandKeyRandIVFullTest.txt", FileMode.Create);
            StreamToStreamRandKeyRandIVFullTest(src, dest);
            src.Close();
            File.Delete(src.Name);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void FileStreamToFileStreamRandIVFullTest()
        {
            FileStream src = new FileStream("testDoc-I-FileStreamToFileStreamEncryptRandIVFullTest.txt", FileMode.Create);
            FileStream dest = new FileStream("testDoc-O-FileStreamToFileStreamEncryptRandIVFullTest.txt", FileMode.Create);
            StreamToStreamRandIVFullTest(src, dest);
            src.Close();
            File.Delete(src.Name);
            dest.Close();
            File.Delete(dest.Name);
        }
*/
        [Test]
        public void FileStreamToFileStreamFullTest()
        {
            FileStream src = new FileStream("testDoc-I-FileStreamToFileStreamFullTest.txt", FileMode.Create);
            FileStream dest = new FileStream("testDoc-O-FileStreamToFileStreamFullTest.txt", FileMode.Create);
            StreamToStreamFullTest(src, dest);
            src.Close();
            File.Delete(src.Name);
            dest.Close();
            File.Delete(dest.Name);
        }

/*
        [Test]
        public void FileStreamToMemoryStreamEncryptRandKeyRandIVRandMessageTest()
        {
            FileStream src = new FileStream("testDoc-FileStreamToMemoryStreamEncryptRandKeyRandIVRandMessageTest.txt", FileMode.Create);
            MemoryStream dest = new MemoryStream();
            StreamToStreamEncryptRandKeyRandIVRandMessageTest(src, dest);
            src.Close();
            File.Delete(src.Name);
        }

        [Test]
        public void FileStreamToMemoryStreamEncryptRandKeyRandIVTest()
        {
            FileStream src = new FileStream("testDoc-FileStreamToMemoryStreamEncryptRandKeyRandIVTest.txt", FileMode.Create);
            MemoryStream dest = new MemoryStream();
            StreamToStreamEncryptRandKeyRandIVTest(src, dest);
            src.Close();
            File.Delete(src.Name);
        }

        [Test]
        public void FileStreamToMemoryStreamEncryptRandIVTest()
        {
            FileStream src = new FileStream("testDoc-FileStreamToMemoryStreamEncryptRandIVTest.txt", FileMode.Create);
            MemoryStream dest = new MemoryStream();
            StreamToStreamEncryptRandIVTest(src, dest);
            src.Close();
            File.Delete(src.Name);
        }

        [Test]
        public void FileStreamToMemoryStreamEncryptTest()
        {
            FileStream src = new FileStream("testDoc-FileStreamToMemoryStreamEncryptTest.txt", FileMode.Create);
            MemoryStream dest = new MemoryStream();
            StreamToStreamEncryptTest(src, dest);
            src.Close();
            File.Delete(src.Name);
        }

        [Test]
        public void FileStreamToMemoryStreamRandKeyRandIVRandMessageFullTest()
        {
            FileStream src = new FileStream("testDoc-FileStreamToMemoryStreamEncryptRandKeyRandIVRandMessageFullTest.txt", FileMode.Create);
            MemoryStream dest = new MemoryStream();
            StreamToStreamRandKeyRandIVRandMessageFullTest(src, dest);
            src.Close();
            File.Delete(src.Name);
        }

        [Test]
        public void FileStreamToMemoryStreamRandKeyRandIVFullTest()
        {
            FileStream src = new FileStream("testDoc-FileStreamToMemoryStreamEncryptRandKeyRandIVFullTest.txt", FileMode.Create);
            MemoryStream dest = new MemoryStream();
            StreamToStreamRandKeyRandIVFullTest(src, dest);
            src.Close();
            File.Delete(src.Name);
        }

        [Test]
        public void FileStreamToMemoryStreamRandIVFullTest()
        {
            FileStream src = new FileStream("testDoc-FileStreamToMemoryStreamEncryptRandIVFullTest.txt", FileMode.Create);
            MemoryStream dest = new MemoryStream();
            StreamToStreamRandIVFullTest(src, dest);
            src.Close();
            File.Delete(src.Name);
        }
*/
        [Test]
        public void FileStreamToMemoryStreamFullTest()
        {
            FileStream src = new FileStream("testDoc-FileStreamToMemoryStreamFullTest.txt", FileMode.Create);
            MemoryStream dest = new MemoryStream();
            StreamToStreamFullTest(src, dest);
            src.Close();
            File.Delete(src.Name);
        }

/*
        [Test]
        public void MemoryStreamToFileStreamEncryptRandKeyRandIVRandMessageTest()
        {
            MemoryStream src = new MemoryStream();
            FileStream dest = new FileStream("testDoc-MemoryStreamToFileStreamEncryptRandKeyRandIVRandMessageTest.txt", FileMode.Create);
            StreamToStreamEncryptRandKeyRandIVRandMessageTest(src, dest);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void MemoryStreamToFileStreamEncryptRandKeyRandIVTest()
        {
            MemoryStream src = new MemoryStream();
            FileStream dest = new FileStream("testDoc-MemoryStreamToFileStreamEncryptRandKeyRandIVTest.txt", FileMode.Create);
            StreamToStreamEncryptRandKeyRandIVTest(src, dest);
            dest.Close();
            File.Delete(dest.Name);
        }
               
        [Test]
        public void MemoryStreamToFileStreamEncryptRandIVTest()
        {
            MemoryStream src = new MemoryStream();
            FileStream dest = new FileStream("testDoc-MemoryStreamToFileStreamEncryptRandIVTest.txt", FileMode.Create);
            StreamToStreamEncryptRandIVTest(src, dest);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void MemoryStreamToFileStreamEncryptTest()
        {
            MemoryStream src = new MemoryStream();
            FileStream dest = new FileStream("testDoc-MemoryStreamToFileStreamEncryptTest.txt", FileMode.Create);
            StreamToStreamEncryptTest(src, dest);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void MemoryStreamToFileStreamRandKeyRandIVRandMessageFullTest()
        {
            MemoryStream src = new MemoryStream();
            FileStream dest = new FileStream("testDoc-MemoryStreamToFileStreamEncryptRandKeyRandIVRandMessageFullTest.txt", FileMode.Create);
            StreamToStreamRandKeyRandIVRandMessageFullTest(src, dest);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void MemoryStreamToFileStreamRandKeyRandIVFullTest()
        {
            MemoryStream src = new MemoryStream();
            FileStream dest = new FileStream("testDoc-MemoryStreamToFileStreamEncryptRandKeyRandIVFullTest.txt", FileMode.Create);
            StreamToStreamRandKeyRandIVFullTest(src, dest);
            dest.Close();
            File.Delete(dest.Name);
        }

        [Test]
        public void MemoryStreamToFileStreamRandIVFullTest()
        {
            MemoryStream src = new MemoryStream();
            FileStream dest = new FileStream("testDoc-MemoryStreamToFileStreamEncryptRandIVFullTest.txt", FileMode.Create);
            StreamToStreamRandIVFullTest(src, dest);
            dest.Close();
            File.Delete(dest.Name);
        }
*/
        [Test]
        public void MemoryStreamToFileStreamFullTest()
        {
            MemoryStream src = new MemoryStream();
            FileStream dest = new FileStream("testDoc-MemoryStreamToFileStreamFullTest.txt", FileMode.Create);
            StreamToStreamFullTest(src, dest);
            dest.Close();
            File.Delete(dest.Name);
        }

        //Statisical analysis

        [Test]
        public void StatisticalAverageExpansionTest()
        {
            MemoryStream src = new MemoryStream();
            MemoryStream dest = new MemoryStream();

            ulong numTests = 100;
            ulong totalBytes = 0;

            for (ulong i=0; i < numTests; i++ )
            {
                totalBytes += StatisticalStreamToStreamRandKeyRandIVRandMessageFullTest(src, dest);
                src.SetLength(0);
                dest.SetLength(0);
            }

            Console.WriteLine("Average expansion is {0}x the message length.", (decimal)( (decimal)((decimal)totalBytes / (decimal)numTests) / (decimal) TestMessageLen ) );

        }


    }
} //namespace cipup_external_test
