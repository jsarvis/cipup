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

namespace RCT
{
    internal class DTO
    {

        /*     FromAddress = "Alice@gmail.com";
               FromName = "Alice";
               Password = "pass";
               Host = "smtp.gmail.com";
               Port = 587;
               EnableSsl = true;
        */
        public class EmailConfig
        {
            protected string _fromAddress;
            protected string _fromName;
            protected string _password;
            protected string _host;
            protected int _port;
            protected bool _enableSsl;

            public string FromAddress{ get { return _fromAddress; } }
            public string FromName{ get { return _fromName; } }
            public string Password{ get { return _password; } }
            public string Host{ get { return _host; } }
            public int Port{ get { return _port; } }
            public bool EnableSsl{ get{ return _enableSsl; } }
        }

        //TODO link with encrypted database
        public class EmailConfigDTO : EmailConfig
        {
            public new string FromAddress{ get { return _fromAddress; } set { _fromAddress = value; } }
            public new string FromName { get { return _fromName; } set { _fromName = value; } }
            public new string Password { get { return _password; } set { _password = value; } }
            public new string Host { get { return _host; } set { _host = value; } }
            public new int Port { get { return _port; } set { _port = value; } }
            public new bool EnableSsl { get { return _enableSsl; } set { _enableSsl = value; } }
        }
    }
}
