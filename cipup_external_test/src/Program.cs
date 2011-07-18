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
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using NUnit.Framework;


namespace cipup_external_test
{
    [TestFixture]
    class Program
    {

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
            byte keylen = cipup.engine.RequiredKeyByteLength();
            byte[] temp = null; // = new byte[keylen];
            cipup.engine.GenerateKey(temp,keylen);
         }

        



        [Test]
        public void InitializationTest()
        {
            cipup.engine beta = new cipup.engine();

        }
    }
} //namespace cipup_external_test
