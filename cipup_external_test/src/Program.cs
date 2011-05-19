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
using System.Linq;
using System.Text;
using cipup_internal_test;

namespace cipup_external_test
{
    class Program
    {

        static void Main(string[] args)
        {
            Console.WriteLine("Test ext test");
            test alpha = new test();
            Console.WriteLine(alpha.GetVersionTextTest());
            Console.WriteLine(alpha.PrintVersionTextTest());

        }
    }
} //namespace cipup_external_test
