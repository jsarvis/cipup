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
using System.IO.Compression;
using System.Net;
using System.Net.Mail;
using System.Security.Cryptography;

namespace RCT
{
    internal class Utilities
    {
        public class Encapsulation
        {
            public static MemoryStream CompressString(string alpha)
            {
                MemoryStream retval = new MemoryStream();
                GZipStream gstr = new GZipStream(retval, CompressionMode.Compress, true);

                byte[] encodedAlpha = Convert.FromBase64String(alpha);

                gstr.Write(encodedAlpha, 0, encodedAlpha.Length);
               
                gstr.Close();
                retval.Position = 0;

                return retval;
            }

            public static string DecompressString(MemoryStream alpha)
            {
                GZipStream gstr = new GZipStream(alpha, CompressionMode.Decompress, true);

                MemoryStream retval = new MemoryStream();
                int n;
                byte[] buffer = new byte[32768];

                while ((n = gstr.Read(buffer, 0, buffer.Length)) > 0)
                {
                    retval.Write(buffer, 0, n);
                }

                gstr.Close();

                return Convert.ToBase64String(retval.GetBuffer());
            }

            public static MemoryStream CompressStream(MemoryStream alpha)
            {
                MemoryStream retval = new MemoryStream();
                GZipStream gstr = new GZipStream(retval, CompressionMode.Compress, true);

                long alphalen = alpha.Length - alpha.Position;
                if (alphalen > int.MaxValue || alpha.Position > int.MaxValue)
                {
                    byte[] buffer = new byte[32768];
                    int n;
                    while ((n = alpha.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        gstr.Write(buffer, 0, n);
                    }
                }
                else
                {
                    gstr.Write(alpha.GetBuffer(), (int)alpha.Position, (int)alphalen);
                    alpha.Position += alphalen;
                }

                gstr.Close();
                retval.Position = 0;

                return retval;
            }

            public static MemoryStream DecompressStream(MemoryStream alpha)
            {
                GZipStream gstr = new GZipStream(alpha, CompressionMode.Decompress, true);

                MemoryStream retval = new MemoryStream();
                int n;
                byte[] buffer = new byte[32768];

                while ((n = gstr.Read(buffer, 0, buffer.Length)) > 0)
                {
                    retval.Write(buffer, 0, n);
                }

                gstr.Close();

                return retval;
            }

            public static MemoryStream TupleStream(MemoryStream alpha, MemoryStream beta)
            {
                MemoryStream retval = new MemoryStream();

                long alphalen = alpha.Length - alpha.Position;
                retval.Write(BitConverter.GetBytes(alphalen), 0, 8);
                if (alphalen > int.MaxValue || alpha.Position > int.MaxValue)
                {
                    byte[] buffer = new byte[32768];
                    int n;
                    while ((n = alpha.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        retval.Write(buffer, 0, n);
                    }
                }
                else
                {
                    retval.Write(alpha.GetBuffer(), (int)alpha.Position, (int)alphalen);
                    alpha.Position += alphalen;
                }

                long betalen = beta.Length - beta.Position;
                retval.Write(BitConverter.GetBytes(betalen), 0, 8);
                if (betalen > int.MaxValue || beta.Position > int.MaxValue)
                {
                    byte[] buffer = new byte[32768];
                    int n;
                    while ((n = beta.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        retval.Write(buffer, 0, n);
                    }
                }
                else
                {
                    retval.Write(beta.GetBuffer(), (int)beta.Position, (int)betalen);
                    beta.Position += betalen;
                }

                return retval;
            }

            public static void DetupleStream(MemoryStream message, out MemoryStream alpha, out MemoryStream beta)
            {
                alpha = new MemoryStream();
                beta = new MemoryStream();
                byte[] temp = new byte[8];
                message.Read(temp, 0, 8);
                long len = BitConverter.ToInt64(temp, 0);

                if (len > int.MaxValue || alpha.Position > int.MaxValue)
                {
                    byte[] buffer = new byte[32768];
                    while (len > 0)
                    {
                        int n = message.Read(buffer, 0, len > buffer.Length ? buffer.Length : (int)len);
                        alpha.Write(buffer, 0, n);
                        len -= n;
                    }
                }
                else
                {
                    alpha.Write(message.GetBuffer(), (int)message.Position, (int)len);
                    message.Position += len;
                }



                message.Read(temp, 0, 8);
                len = BitConverter.ToInt64(temp, 0);
                if (len > int.MaxValue || alpha.Position > int.MaxValue)
                {
                    byte[] buffer = new byte[32768];
                    while (len > 0)
                    {
                        int n = message.Read(buffer, 0, len > buffer.Length ? buffer.Length : (int)len);
                        beta.Write(buffer, 0, n);
                        len -= n;
                    }
                }
                else
                {
                    beta.Write(message.GetBuffer(), (int)message.Position, (int)len);
                    message.Position += len;
                }
            }
        }

        public class Comm
        {
            public static void SendEmail(MailMessage message)
            {
                SendEmail(message, false);
            }

            public static void SendEmail(MailMessage message, bool retryOnTimeout)
            {
                var fromAddress = new MailAddress(Client.EmailConfig.FromAddress, Client.EmailConfig.FromName);

                var smtp = new SmtpClient
                {
                    Host = Client.EmailConfig.Host,
                    Port = Client.EmailConfig.Port,
                    EnableSsl = Client.EmailConfig.EnableSsl,
                    DeliveryMethod = SmtpDeliveryMethod.Network,
                    UseDefaultCredentials = false,
                    Credentials = new NetworkCredential(fromAddress.Address, Client.EmailConfig.Password),
                    Timeout = 20000
                };

                if (retryOnTimeout)
                {
                    while (retryOnTimeout)
                    {
                        try
                        {
                            smtp.Send(message);
                            retryOnTimeout = false;
                        }
                        catch (SmtpFailedRecipientsException e)
                        {
                            //Remove all recipients
                            message.To.Clear();
                            foreach (SmtpFailedRecipientException t in e.InnerExceptions)
                            {
                                SmtpStatusCode status = t.StatusCode;
                                if (status == SmtpStatusCode.MailboxBusy ||
                                    status == SmtpStatusCode.MailboxUnavailable)
                                {
                                    //Add back failed recipients
                                    message.To.Add(t.FailedRecipient);
                                }
                                else
                                {
                                    throw;
                                }
                            }
                            System.Threading.Thread.Sleep(5000);
                        }
                        catch (SmtpException e)
                        {
                            SmtpStatusCode status = e.StatusCode;
                            if (status == SmtpStatusCode.MailboxBusy ||
                                status == SmtpStatusCode.MailboxUnavailable)
                            {
                                System.Threading.Thread.Sleep(5000);
                            }
                            else
                            {
                                throw;
                            }
                        }
                    }
                }
                else
                {
                    smtp.Send(message);
                }
            }

            public static MailMessage ComposeMessage(string toAddress)
            {
                var sender = new MailAddress(Client.EmailConfig.FromAddress, Client.EmailConfig.FromName);
                var recipient = new MailAddress(toAddress);
                MailMessage retval = new MailMessage(sender, recipient);
                return retval;
            }

            public static MailMessage ComposeMessage(string toAddress, string toName)
            {
                var sender = new MailAddress(Client.EmailConfig.FromAddress, Client.EmailConfig.FromName);
                var recipient = new MailAddress(toAddress, toName);
                MailMessage retval = new MailMessage(sender, recipient);
                return retval;
            }

            public static MailMessage ComposeMessage(string toAddress, string toName, string subject, string body)
            {
                MailMessage retval = ComposeMessage(toAddress, toName);
                retval.Subject = subject;
                retval.Body = body;
                return retval;
            }

            public static MailMessage ComposeMessage(string toAddress, string toName, string subject, string body, Stream attachment, string attachmentName)
            {
                MailMessage retval = ComposeMessage(toAddress, toName, subject, body);
                Attachment temp = new Attachment(attachment, attachmentName, "application/x-gzip");
                retval.Attachments.Add(temp);
                return retval;
            }

            public static void AddAttachment(ref MailMessage msg, Stream attachment, string attachmentName)
            {
                Attachment temp = new Attachment(attachment, attachmentName, "application/x-gzip");
                temp.ContentDisposition.FileName = attachmentName;
                msg.Attachments.Add(temp);
            }

            public static void AddRecipient(ref MailMessage msg, string toAddress)
            {
                msg.To.Add(new MailAddress(toAddress));
            }

            public static void AddRecipient(ref MailMessage msg, string toAddress, string toName)
            {
                msg.To.Add(new MailAddress(toAddress, toName));
            }
        }

        public class Crypto
        {

            public const int AsymKeySizeBytes = 256; //TODO determine key size
            public const int SymKeySizeBytes = 128;
            public const int SymIvSizeBytes = 64;
            public const int DbPasswordSizeChars = 40;
            private const string PasswordChars = "abcdefgijkmnopqrstwxyzABCDEFGHJKLMNPQRSTWXYZ1234567890~`#$%^-_+=|:/";

            public static void GenerateAsymKeys(out string pubPrivKeyStore, out string pubKeyStore)
            {
                RSA rsa = new RSACryptoServiceProvider(AsymKeySizeBytes * 8); // Generate a new 2048 bit RSA key
                pubPrivKeyStore = rsa.ToXmlString(true);
                pubKeyStore = rsa.ToXmlString(false);
            }

            public static byte[] GenerateSymKey()
            {
                byte[] retval = new byte[SymKeySizeBytes];
                cipup.engine.GenerateKey(retval, (uint)SymKeySizeBytes);
                return retval;
            }

            public static byte[] GenerateSymIv()
            {
                byte[] retval = new byte[SymIvSizeBytes];
                cipup.engine.GenerateKey(retval, (uint)SymIvSizeBytes);
                return retval;
            }

            public static string GenerateDbPassword()
            {
                string retval = "";
                byte[] buffer = new byte[DbPasswordSizeChars];
                RNGCryptoServiceProvider rand = new RNGCryptoServiceProvider();
                rand.GetBytes(buffer);

                for (int i = 0; i < DbPasswordSizeChars; i++)
                {
                    retval += PasswordChars[(int)((((double)buffer[i] / ((double)byte.MaxValue + 1.0)) * (double)(PasswordChars.Length)) / 1)];
                }

                return retval;
            }

            public static MemoryStream EncryptAsym(string pubKeyStore, MemoryStream plaintext)
            {
                RSACryptoServiceProvider rsa = new RSACryptoServiceProvider();
                rsa.FromXmlString(pubKeyStore);
                byte[] cryptext = rsa.Encrypt(plaintext.GetBuffer(), true);
                return new MemoryStream(cryptext);
            }

            public static MemoryStream DecryptAsym(string privKeyStore, MemoryStream cryptext)
            {
                RSACryptoServiceProvider rsa = new RSACryptoServiceProvider();
                rsa.FromXmlString(privKeyStore);
                byte[] plaintext = rsa.Decrypt(cryptext.GetBuffer(), true);
                return new MemoryStream(plaintext);
            }

            public static MemoryStream EncryptSym(byte[] key, byte[] iv, MemoryStream plaintext)
            {
                MemoryStream retval = new MemoryStream();
                cipup.engine cryptEng = new cipup.engine();

                cipup.MessageCode status = cryptEng.init(cipup.InitAction.InitEncrypt, retval, key, (byte)(key.Length), iv, (byte)(iv.Length));
                if (status != cipup.MessageCode.InitSuccess)
                {
                    throw new Exception("Error in initialization of cipup: " + cipup.engine.Messages[(int)status]);
                }
                cryptEng.encrypt(plaintext);
                cryptEng.finalize();

                return retval;
            }

            public static MemoryStream DecryptSym(byte[] key, byte[] iv, MemoryStream cryptext)
            {
                MemoryStream retval = new MemoryStream();
                cipup.engine cryptEng = new cipup.engine();

                cipup.MessageCode status = cryptEng.init(cipup.InitAction.InitDecrypt, retval, key, (byte)(key.Length), iv, (byte)(iv.Length));
                if (status != cipup.MessageCode.InitSuccess)
                {
                    throw new Exception("Error in initialization of cipup: " + cipup.engine.Messages[(int)status]);
                }
                cryptEng.decrypt(cryptext);
                cryptEng.finalize();

                return retval;
            }

            public static MemoryStream PasswordEncrypt(string password, MemoryStream plaintext)
            {
                SHA384 shaM = new SHA384Managed();
                byte[] hashvalue = shaM.ComputeHash(Convert.FromBase64String(password)); //384 bit hash of password
                byte[] key = new byte[SymKeySizeBytes];
                byte[] iv = new byte[SymIvSizeBytes];
                //Shrink hash into 192 bits and save in key and iv array
                for (int i = 0; i < 16; i++)
                {
                    key[i] = (byte)(hashvalue[i] ^ hashvalue[i + 24]);
                }
                for (int i = 16; i < 24; i++)
                {
                    iv[i] = (byte)(hashvalue[i] ^ hashvalue[i + 24]);
                }

                return EncryptSym(key, iv, plaintext);
            }

            public static MemoryStream PasswordDecrypt(string password, MemoryStream cryptext)
            {
                SHA384 shaM = new SHA384Managed();
                byte[] hashvalue = shaM.ComputeHash(Convert.FromBase64String(password)); //384 bit hash of password
                byte[] key = new byte[SymKeySizeBytes];
                byte[] iv = new byte[SymIvSizeBytes];
                //Shrink hash into 192 bits and save in key and iv array
                for (int i = 0; i < 16; i++)
                {
                    key[i] = (byte)(hashvalue[i] ^ hashvalue[i + 24]);
                }
                for (int i = 16; i < 24; i++)
                {
                    iv[i] = (byte)(hashvalue[i] ^ hashvalue[i + 24]);
                }

                return EncryptSym(key, iv, cryptext);
            }

        }


    }
}
