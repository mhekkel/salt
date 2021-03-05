using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Net.Sockets;
using System.Threading;
using System.Text;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.IO;
using System.Linq;
using Org.Mentalis.Security.Cryptography;
using System.Globalization;

namespace SSH
{
	enum SSH_MSG
	{
		DISCONNECT = 1,
		IGNORE,
		UNIMPLEMENTED,
		DEBUG,
		SERVICE_REQUEST,
		SERVICE_ACCEPT,

		KEXINIT = 20,
		NEWKEYS,

		/*	Numbers 30-49 used for kex packets.
			Different kex methods may reuse message numbers in
			this range. */

		KEXDH_INIT = 30,
		KEXDH_REPLY,

		USERAUTH_REQUEST = 50,
		USERAUTH_FAILURE,
		USERAUTH_SUCCESS,
		USERAUTH_BANNER,

		USERAUTH_INFO_REQUEST = 60,
		USERAUTH_INFO_RESPONSE,

		GLOBAL_REQUEST = 80,
		REQUEST_SUCCESS,
		REQUEST_FAILURE,

		CHANNEL_OPEN = 90,
		CHANNEL_OPEN_CONFIRMATION,
		CHANNEL_OPEN_FAILURE,
		CHANNEL_WINDOW_ADJUST,
		CHANNEL_DATA,
		CHANNEL_EXTENDED_DATA,
		CHANNEL_EOF,
		CHANNEL_CLOSE,
		CHANNEL_REQUEST,
		CHANNEL_SUCCESS,
		CHANNEL_FAILURE,
	};

	enum SSH_DISCONNECT
	{
		HOST_NOT_ALLOWED_TO_CONNECT = 1,
		PROTOCOL_ERROR,
		KEY_EXCHANGE_FAILED,
		RESERVED,
		MAC_ERROR,
		COMPRESSION_ERROR,
		SERVICE_NOT_AVAILABLE,
		PROTOCOL_VERSION_NOT_SUPPORTED,
		HOST_KEY_NOT_VERIFIABLE,
		CONNECTION_LOST,
		BY_APPLICATION,
		TOO_MANY_CONNECTIONS,
		AUTH_CANCELLED_BY_USER,
		NO_MORE_AUTH_METHODS_AVAILABLE,
		ILLEGAL_USER_NAME
	};

	enum SSH_AGENTC
	{

		/*
		 * SSH1 agent messages.
		 */
		SSH_AGENTC_REQUEST_RSA_IDENTITIES = 1,
		SSH_AGENT_RSA_IDENTITIES_ANSWER,
		SSH_AGENTC_RSA_CHALLENGE,
		SSH_AGENT_RSA_RESPONSE,
		/*
		 * Messages common to SSH1 and OpenSSH's SSH2.
		 */
		SSH_AGENT_FAILURE,
		SSH_AGENT_SUCCESS,

		/*
		 * SSH1 agent messages.
		 */

		SSH_AGENTC_ADD_RSA_IDENTITY,
		SSH_AGENTC_REMOVE_RSA_IDENTITY,
		SSH_AGENTC_REMOVE_ALL_RSA_IDENTITIES,	/* openssh private? */

		/*
		 * OpenSSH's SSH2 agent messages.
		 */
		SSH_AGENTC_REQUEST_IDENTITIES = 11,
		SSH_AGENT_IDENTITIES_ANSWER,
		SSH_AGENTC_SIGN_REQUEST,
		SSH_AGENT_SIGN_RESPONSE,
		SSH_AGENTC_ADD_IDENTITY = 17,
		SSH_AGENTC_REMOVE_IDENTITY,
		SSH_AGENTC_REMOVE_ALL_IDENTITIES
	};

	internal class OutPacket
	{
		private MemoryStream data;

		internal byte[] Data
		{
			get
			{
				byte[] data = new byte[this.data.Length];
				Array.Copy(this.data.GetBuffer(), data, (int)this.data.Length);
				return data;
			}
		}

		internal OutPacket()
		{
			this.data = new MemoryStream();
		}

		internal OutPacket(SSH_MSG msg)
			: this()
		{
			Write((byte)msg);
		}

		public OutPacket Write(byte b) { data.WriteByte(b); return this; }
		public OutPacket WriteBytes(byte[] b) { data.Write(b, 0, b.Length); return this; }
		public OutPacket Write(bool b) { data.WriteByte((byte)(b ? 1 : 0)); return this; }

		public OutPacket Write(UInt32 v)
		{
			for (int i = 3; i >= 0; --i)
				data.WriteByte((byte)(v >> i * 8));
			return this;
		}

		public OutPacket Write(byte[] b)
		{
			Write((UInt32)b.Length);
			WriteBytes(b);
			return this;
		}

		public OutPacket Write(string s)
		{
			Write((UInt32)s.Length);
			WriteBytes(Encoding.UTF8.GetBytes(s));
			return this;
		}

		public OutPacket WriteBigInt(byte[] i)
		{
			if ((i[0] & 0x80) != 0)
			{
				Write((UInt32)(i.Length + 1));
				Write((byte)0);
			}
			else
				Write((UInt32)i.Length);
			WriteBytes(i);
			return this;
		}

		public OutPacket Write(InPacket i)
		{
			byte[] b = i.Data;
			Write((UInt32)b.Length);
			WriteBytes(b);
			return this;
		}

		public byte[] Wrap(int blockSize, RNGCryptoServiceProvider rng, int seqNr, HMAC hmac)
		{
			int length = (int)this.data.Length + 5;
			int paddingLength = blockSize - (length % blockSize);
			if (paddingLength < 4)
				paddingLength += blockSize;

			length += paddingLength;

			byte[] padding = new byte[paddingLength];
			rng.GetBytes(padding);
			this.data.Write(padding, 0, paddingLength);

			int hmacLen = 0;
			if (hmac != null)
				hmacLen = hmac.HashSize / 8;

			byte[] res = new byte[length + hmacLen];
			length -= 4;

			for (int i = 0; i < 4; ++i)
				res[i] = ((byte)(length >> (24 - i * 8)));
			res[4] = (byte)paddingLength;

			Array.Copy(this.Data, 0, res, 5, (int)this.data.Length);

			if (hmac != null)
			{
				CryptoStream cs = new CryptoStream(System.IO.Stream.Null, hmac, CryptoStreamMode.Write);

				byte[] sn = new byte[4] {
					(byte)(seqNr >> 24),
					(byte)(seqNr >> 16),
					(byte)(seqNr >>  8),
					(byte)(seqNr      )
				};
				cs.Write(sn, 0, 4);
				cs.Write(res, 0, res.Length - hmacLen);
				cs.Close();

				byte[] h = hmac.Hash;
				Array.Copy(h, 0, res, res.Length - hmacLen, h.Length);
			}

			return res;
		}
	}

	internal class InPacket
	{
		private MemoryStream data;
		private SSH_MSG msg;
		private int offset = 0;

		public SSH_MSG Msg { get { return msg; } }
		public byte[] Data
		{
			get
			{
				byte[] data = new byte[this.data.Length];
				Array.Copy(this.data.GetBuffer(), offset, data, 0, (int)this.data.Length);
				return data;
			}
		}

		private InPacket(MemoryStream data, byte msg, int offset)
		{
			this.data = data;
			this.msg = (SSH_MSG)msg;
			this.offset = offset;
		}

		internal static InPacket Unwrap(MemoryStream packet, int seqNr, HMAC verifier)
		{
			int size = (int)packet.Length;
			byte[] b = packet.GetBuffer();

			if (verifier != null)
			{
				int hmacLen = verifier.HashSize / 8;

				byte[] sn = new byte[4] {
					(byte)(seqNr >> 24),
					(byte)(seqNr >> 16),
					(byte)(seqNr >>  8),
					(byte)(seqNr      )
				};

				verifier.TransformBlock(sn, 0, sn.Length, sn, 0);
				byte[] h = verifier.TransformFinalBlock(b, 0, size - hmacLen);
				
				for (int i = 0; i < hmacLen; ++i)
					if (b[size - hmacLen + i] != h[i])
						throw new Exception("HMAC invalid");
	
				size -= hmacLen;
			}

			int len = (int)(size - 5);
			len -= b[4];
			byte msg = b[5];
			return new InPacket(new MemoryStream(b, 6, len, false, true), msg, 5);
		}

		public InPacket Read(out byte b) { b = (byte)this.data.ReadByte(); return this; }
		public InPacket Read(out bool b) { byte v = (byte)this.data.ReadByte(); b = v != 0; return this; }
		public InPacket Read(byte[] b) { this.data.Read(b, 0, b.Length); return this; }
	
		public InPacket Read(out UInt32 v)
		{
			v = 0;
			for (int i = 0; i < 4; ++i)
				v = (v << 8) | (byte)data.ReadByte();
			return this;
		}

		public InPacket Read(out string v)
		{
			UInt32 len;
			Read(out len);
			byte[] b = new byte[len];
			Read(b);
			v = Encoding.UTF8.GetString(b, 0, (int)len);
			return this;
		}

		public InPacket Read(out InPacket v)
		{
			UInt32 len;
			Read(out len);
			byte[] b = new byte[len];
			Read(b);
			v = new InPacket(new MemoryStream(b, 0, b.Length, false, true), 0, 0);
			return this;
		}

		public InPacket ReadBigInt(out byte[] v)
		{
			UInt32 len;
			Read(out len);
			v = new byte[len];
			Read(v);
			return this;
		}
	}

	public class Connection
	{
		private static string kVersionString = "SSH-2.0-SaltWP-1.0";
		private static string
			kKeyExchangeAlgorithms = "diffie-hellman-group14-sha1,diffie-hellman-group1-sha1",
			kServerHostKeyAlgorithms = "ssh-rsa,ssh-dss",
			//kEncryptionAlgorithms = "aes256-cbc,aes192-cbc,aes128-cbc",
			kEncryptionAlgorithms = "aes128-cbc",
			//kMacAlgorithms = "hmac-sha1,hmac-md5",
			kMacAlgorithms = "hmac-sha1",
			kDontUseCompressionAlgorithms = "none";

		private static byte[]
			p2 = {
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
				0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
				0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
				0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
				0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
				0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
				0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
				0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
			},
			p14 = {
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
				0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
				0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
				0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
				0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
				0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
				0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
				0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
				0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
				0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
				0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
				0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
				0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F,
				0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18,
				0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
				0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
			};

		Socket _socket;
		static ManualResetEvent _clientDone = new ManualResetEvent(false);

		// Define a timeout in milliseconds for each asynchronous call. If a response is not received within this 
		// timeout period, the call is aborted.
		const int TIMEOUT_MILLISECONDS = 5000;

		// The maximum size of the data buffer to use with the asynchronous socket methods
		const int MAX_BUFFER_SIZE = 64000;
		private string hostVersion;
		private byte[] myPayLoad, hostPayLoad;
		private string kexAlg, serverHostKeyAlg, encAlgC2S, encAlgS2C, macAlgC2S, macAlgS2C, compAlgC2S, compAlgS2C, langC2S, langS2C;
		private byte[] A, B, C, D, E, F;
		private ICryptoTransform encryptor, decryptor;
		private AesManaged aesEnc, aesDec;
		private HMAC signer, verifier;
		private DiffieHellman dh;
		private byte[] e, x;
		private RNGCryptoServiceProvider rng;
		private List<byte> buffer;
		private MemoryStream packet;
		private int packetLength, inSequenceNr, outSequenceNr;
		private byte[] sessionID;
		private bool authenticated = false;

		public Connection()
		{
			this.rng = new RNGCryptoServiceProvider();
			this.buffer = new List<byte>();
			this.packet = new MemoryStream();
		}

		public void Connect(string hostName, int portNumber)
		{
			// Create DnsEndPoint. The hostName and port are passed in to this method.
			DnsEndPoint hostEntry = new DnsEndPoint(hostName, portNumber);

			// Create a stream-based, TCP socket using the InterNetwork Address Family. 
			_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

			// Create a SocketAsyncEventArgs object to be used in the connection request
			SocketAsyncEventArgs socketEventArg = new SocketAsyncEventArgs();
			socketEventArg.RemoteEndPoint = hostEntry;

			// Inline event handler for the Completed event.
			// Note: This event handler was implemented inline in order to make this method self-contained.
			socketEventArg.Completed += new EventHandler<SocketAsyncEventArgs>(ConnectCompleted);

			// Make an asynchronous Connect request over the socket
			_socket.ConnectAsync(socketEventArg);
		}

		void ConnectCompleted(object sender, SocketAsyncEventArgs e)
		{
			if (e.SocketError == SocketError.Success)
			{
				SocketAsyncEventArgs socketEventArg = new SocketAsyncEventArgs();
				socketEventArg.RemoteEndPoint = _socket.RemoteEndPoint;
				socketEventArg.UserToken = null;
				socketEventArg.Completed += new EventHandler<SocketAsyncEventArgs>(SendConnectCompleted);
				byte[] payload = Encoding.UTF8.GetBytes(kVersionString + "\r\n");
				socketEventArg.SetBuffer(payload, 0, payload.Length);
				_socket.SendAsync(socketEventArg);
			}
			//else
			//    ; // TODO error handling
		}

		void SendConnectCompleted(object sender, SocketAsyncEventArgs e)
		{
			if (e.SocketError == SocketError.Success)
			{
				SocketAsyncEventArgs socketEventArg = new SocketAsyncEventArgs();
				socketEventArg.RemoteEndPoint = _socket.RemoteEndPoint;
				socketEventArg.SetBuffer(new Byte[MAX_BUFFER_SIZE], 0, MAX_BUFFER_SIZE);
				socketEventArg.Completed += new EventHandler<SocketAsyncEventArgs>(VersionExchangeCompleted);
				_socket.ReceiveAsync(socketEventArg);
			}
		}

		void VersionExchangeCompleted(object sender, SocketAsyncEventArgs e)
		{
			if (e.SocketError == SocketError.Success)
			{
				int n = 0;
				while (n < e.BytesTransferred && e.Buffer[e.Offset + n] != '\n')
					++n;
				if (n == e.BytesTransferred)
					throw new Exception("ouch");
				++n;

				hostVersion = Encoding.UTF8.GetString(e.Buffer, e.Offset, n);
				hostVersion = hostVersion.TrimEnd(new[] { '\r', '\n' });

				if (hostVersion.StartsWith("SSH-2.0") == false)
					throw new Exception("ouch");

				byte[] salt = new byte[16];
				rng.GetBytes(salt);

				OutPacket p = new OutPacket(SSH_MSG.KEXINIT);
				p.WriteBytes(salt)
				 .Write(kKeyExchangeAlgorithms)
				 .Write(kServerHostKeyAlgorithms)
				 .Write(kEncryptionAlgorithms)
				 .Write(kEncryptionAlgorithms)
				 .Write(kMacAlgorithms)
				 .Write(kMacAlgorithms)
				 .Write(kDontUseCompressionAlgorithms)
				 .Write(kDontUseCompressionAlgorithms)
				 .Write("")
				 .Write("")
				 .Write(false)
				 .Write((UInt32)0);

				myPayLoad = p.Data;

				Send(p);
				
				if (e.BytesTransferred > n)
				{
					for (int i = e.Offset + n; i < e.Offset + e.BytesTransferred; ++i)
						buffer.Add(e.Buffer[i]);
					ProcessBuffer();
				}

				// Start receiving remaining data
				SocketAsyncEventArgs socketEventArg = new SocketAsyncEventArgs();
				socketEventArg.RemoteEndPoint = _socket.RemoteEndPoint;
				socketEventArg.SetBuffer(new Byte[MAX_BUFFER_SIZE], 0, MAX_BUFFER_SIZE);
				socketEventArg.Completed += (s, a) => Receive(s, a);
				_socket.ReceiveAsync(socketEventArg);
			}
			else
			{
				string response = e.SocketError.ToString();
			}

			_clientDone.Set();
		}

		private void Error()
		{
			throw new NotImplementedException();
		}

		void Send(OutPacket p)
		{
			SocketAsyncEventArgs socketEventArg = new SocketAsyncEventArgs();

			// Set properties on context object
			socketEventArg.RemoteEndPoint = _socket.RemoteEndPoint;
			socketEventArg.UserToken = null;

			// Inline event handler for the Completed event.
			// Note: This event handler was implemented inline in order to make this method self-contained.
			socketEventArg.Completed += (s, e) =>
			{
				string response = e.SocketError.ToString();

				//// Unblock the UI thread
				//_clientDone.Set();
			};

			int blockSize = 8;
			if (encryptor != null)
				blockSize = encryptor.InputBlockSize;
			byte[] b = p.Wrap(blockSize, this.rng, outSequenceNr, signer);
			if (encryptor != null)
			{
				int l = b.Length - (signer.HashSize / 8);
				byte[] t = new byte[l];
				encryptor.TransformBlock(b, 0, l, t, 0);
				Array.Copy(t, b, l);
			}
			++outSequenceNr;

			socketEventArg.SetBuffer(b, 0, b.Length);
			_socket.SendAsync(socketEventArg);
		}

		void Receive(object s, SocketAsyncEventArgs e)
		{
			if (e.SocketError != SocketError.Success)
				throw new Exception("receive: " + e.SocketError.ToString());

			for (int i = e.Offset; i < e.Offset + e.BytesTransferred; ++i)
				buffer.Add(e.Buffer[i]);
			_socket.ReceiveAsync(e);		// restart the receive

			// now process whatever we received
			ProcessBuffer();
		}

		void ProcessBuffer()
		{
			for (; ; )
			{
				int blockSize = 8;
				if (decryptor != null)
					blockSize = decryptor.InputBlockSize;

				if (buffer.Count < blockSize)
					break;

				byte[] block = new byte[blockSize];
				buffer.CopyTo(0, block, 0, blockSize);
				buffer.RemoveRange(0, blockSize);

				if (decryptor != null)
				{
					using (var m = new MemoryStream())
					{
						using (var cs = new CryptoStream(m, decryptor, CryptoStreamMode.Write))
						{
							cs.Write(block, 0, blockSize);
						}
						block = m.ToArray();
					}

					byte[] t = new byte[blockSize];
					decryptor.TransformBlock(block, 0, blockSize, t, 0);
					block = t;
				}

				packet.Write(block, 0, blockSize);

				if (packet.Position == blockSize)
				{
					packetLength = 0;
					for (int i = 0; i < 4; ++i)
						packetLength = (packetLength << 8) | block[i];
				}

				if (packet.Position == packetLength + sizeof(UInt32))
				{
					ProcessPacket(InPacket.Unwrap(packet, inSequenceNr, verifier));
					++inSequenceNr;

					packet = new MemoryStream();
					packetLength = 0;
				}
			}
		}

		void ProcessPacket(InPacket p)
		{
			switch (p.Msg)
			{
				//case SSH_MSG.DISCONNECT: ProcessDisconnect(p); break;
				//case SSH_MSG.IGNORE: break;
				//case SSH_MSG.UNIMPLEMENTED: Error(SSH_DISCONNECT_PROTOCOL_ERROR, _("Unimplemented SSH feature")); break;
				//case SSH_MSG.DEBUG: ProcessDebug(in); break;
				//case SSH_MSG.SERVICE_ACCEPT:		ProcessServiceAccept(in); break;
				case SSH_MSG.KEXINIT:				ProcessKexInit(p); break;
				case SSH_MSG.NEWKEYS:				ProcessNewKeys(p); break;
				case SSH_MSG.KEXDH_REPLY:			ProcessKexDHReply(p); break;
				case SSH_MSG.USERAUTH_SUCCESS:		ProcessUserAuthSuccess(p); break;
				case SSH_MSG.USERAUTH_FAILURE:		ProcessUserAuthFailed(p); break;
				case SSH_MSG.USERAUTH_BANNER:
				{
				//    string msg, lang;
				//    in >> inMessage >> msg >> lang;
			
				//    if (not mChannels.empty() and mChannels.front()->eChannelBanner)
				//        mChannels.front()->eChannelBanner(msg);
				    break;
				}
				//case SSH_MSG.USERAUTH_INFO_REQUEST: ProcessUserAuthInfoRequest(in); break;
				//case SSH_MSG.CHANNEL_OPEN: ProcessChannelOpen(in); break;
				case SSH_MSG.CHANNEL_OPEN_CONFIRMATION:
				case SSH_MSG.CHANNEL_OPEN_FAILURE:
				case SSH_MSG.CHANNEL_WINDOW_ADJUST:
				case SSH_MSG.CHANNEL_DATA:
				case SSH_MSG.CHANNEL_EXTENDED_DATA:
				case SSH_MSG.CHANNEL_EOF:
				case SSH_MSG.CHANNEL_CLOSE:
				case SSH_MSG.CHANNEL_REQUEST:
				case SSH_MSG.CHANNEL_SUCCESS:
				case SSH_MSG.CHANNEL_FAILURE:
					//if (not mAuthenticated)
					//    Error(SSH_DISCONNECT_PROTOCOL_ERROR, _("invalid message, not authenticated yet"));
					//ProcessChannel(in);
					break;
				case SSH_MSG.REQUEST_SUCCESS: break;
				default:
					Console.WriteLine("This message should not have been received");
//			Error(SSH_DISCONNECT_PROTOCOL_ERROR, "Unknown message received");
					break;
			}
		}

		private string ChooseProtocol(string s, string c)
		{
			string[] se = s.Split(new char[] { ',' });
			string[] ce = c.Split(new char[] { ',' });

			return (from a in se
					from b in ce
					where a == b
					select a).FirstOrDefault();
		}

		void ProcessKexInit(InPacket p)
		{
			hostPayLoad = p.Data;

			byte[] salt = new byte[16];
			bool first_kex_packet_follows;

			p.Read(salt)
			 .Read(out kexAlg)
			 .Read(out serverHostKeyAlg)
			 .Read(out encAlgC2S)
			 .Read(out encAlgS2C)
			 .Read(out macAlgC2S)
			 .Read(out macAlgS2C)
			 .Read(out compAlgC2S)
			 .Read(out compAlgS2C)
			 .Read(out langC2S)
			 .Read(out langS2C)
			 .Read(out first_kex_packet_follows);

			if (ChooseProtocol(kexAlg, kKeyExchangeAlgorithms) == "diffie-hellman-group14-sha1")
			{
				dh = new DiffieHellmanManaged(p14, new byte[] { 2 }, 0);
				this.e = dh.CreateKeyExchange();
			}
			else if (ChooseProtocol(kexAlg, kKeyExchangeAlgorithms) == "diffie-hellman-group1-sha1")
			{
				dh = new DiffieHellmanManaged(p14, new byte[] { 2 }, 0);
				this.e = dh.CreateKeyExchange();
			}
			else
			{
				throw new Exception("Invalid key exchange algo");
			}
			
			OutPacket o = new OutPacket(SSH_MSG.KEXDH_INIT);
			o.WriteBigInt(this.e);
			Send(o);
		}

		void ProcessKexDHReply(InPacket p)
		{
			InPacket hostKey, signature;
			byte[] f;

			p.Read(out hostKey).ReadBigInt(out f).Read(out signature);
	
			//string hostName = mIPAddress;
			//if (mPortNumber != 22)
			//    hostName = hostName + ':' + boost::lexical_cast<string>(mPortNumber);

			byte[] K = dh.DecryptKeyExchange(f);

			OutPacket h_test = new OutPacket();
			h_test.Write(kVersionString)
				  .Write(hostVersion)
				  .Write(myPayLoad)
				  .Write(hostPayLoad)
				  .Write(hostKey)
				  .WriteBigInt(e)
				  .WriteBigInt(f)
				  .WriteBigInt(K);

			SHA1 sha = new SHA1Managed();
			byte[] H = sha.ComputeHash(h_test.Data);

			if (sessionID == null)
				sessionID = H;

			string pk_type;
			InPacket pk_rs;
			signature.Read(out pk_type).Read(out pk_rs);

			RSAParameters rsaParams = new RSAParameters();

			//if (not MKnownHosts::Instance().CheckHost(hostName, pk_type, hostKey))
			//    Error(SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE, _("User cancelled"));

			string h_pk_type;
			hostKey.Read(out h_pk_type);

			if (h_pk_type == "ssh-dss")
			{
			    byte[] h_p, h_q, h_g, h_y;
			    hostKey.ReadBigInt(out h_p).ReadBigInt(out h_q).ReadBigInt(out h_g).ReadBigInt(out h_y);

				//h_key = new RSAPKCS1SignatureDeformatter(RSA);
				//h_key.SetHashAlgorithm("SHA1");

				//h_key.reset(new GDSA<SHA1>::Verifier(h_p, h_q, h_g, h_y));
			}
			else if (h_pk_type == "ssh-rsa")
			{
			    byte[] h_e, h_n;
			    hostKey.ReadBigInt(out h_e).ReadBigInt(out h_n);

				if (h_n[0] == 0)
				{
					byte[] t = h_n;
					h_n = new byte[h_n.Length - 1];
					Array.Copy(t, 1, h_n, 0, h_n.Length);
				}

				rsaParams.Modulus = h_n;
				rsaParams.Exponent = h_e;
			}
			//else
			//    Error(SSH_DISCONNECT_KEY_EXCHANGE_FAILED, _("Unexpected hostkey type"));

			RSACryptoServiceProvider RSA = new RSACryptoServiceProvider();
			RSA.ImportParameters(rsaParams);
			RSAPKCS1SignatureDeformatter RSADeformatter = new RSAPKCS1SignatureDeformatter(RSA);
			RSADeformatter.SetHashAlgorithm("SHA1");

			using (CryptoStream cs = new CryptoStream(System.IO.Stream.Null, sha, CryptoStreamMode.Write))
			{
				cs.Write(H, 0, H.Length);
			}
	
			if (RSADeformatter.VerifySignature(sha, pk_rs.Data) == false)
				throw new Exception("Could not exchange keys");

			//if (pk_type != h_pk_type)
			//    Error(SSH_DISCONNECT_KEY_EXCHANGE_FAILED, _("Unexpected hostkey type"));
	
			//if (not h_key->VerifyMessage(&H[0], dLen, pk_rs.peek(), pk_rs.size()))
			//    Error(SSH_DISCONNECT_KEY_EXCHANGE_FAILED, _("hostkey verification failed"));

			int keyLen = 16, sigLen = 16;

			if (keyLen < 20 && ChooseProtocol(macAlgC2S, kMacAlgorithms) == "hmac-sha1")
			    sigLen = 20;

			if (keyLen < 20 && ChooseProtocol(macAlgS2C, kMacAlgorithms) == "hmac-sha1")
			    sigLen = 20;

			//if (keyLen < 24 && ChooseProtocol(encAlgC2S, kEncryptionAlgorithms) == "3des-cbc")
			//    keyLen = 24;

			//if (keyLen < 24 && ChooseProtocol(encAlgS2C, kEncryptionAlgorithms) == "3des-cbc")
			//    keyLen = 24;

			//if (keyLen < 24 && ChooseProtocol(encAlgC2S, kEncryptionAlgorithms).IndexOf("aes192-") == 0)
			//    keyLen = 24;

			//if (keyLen < 24 && ChooseProtocol(encAlgS2C, kEncryptionAlgorithms).IndexOf("aes192-") == 0)
			//    keyLen = 24;

			//if (keyLen < 32 && ChooseProtocol(encAlgC2S, kEncryptionAlgorithms).IndexOf("aes256-") == 0)
			//    keyLen = 32;

			//if (keyLen < 32 && ChooseProtocol(encAlgS2C, kEncryptionAlgorithms).IndexOf("aes256-") == 0)
			//    keyLen = 32;

		    DeriveKey(K, H, 0, keyLen, out A);
			DeriveKey(K, H, 1, keyLen, out B);
			DeriveKey(K, H, 2, keyLen, out C);
			DeriveKey(K, H, 3, keyLen, out D);
			DeriveKey(K, H, 4, sigLen, out E);
			DeriveKey(K, H, 5, sigLen, out F);

			OutPacket o = new OutPacket(SSH_MSG.NEWKEYS);
			Send(o);
		}

		private void DeriveKey(byte[] K, byte[] H, int i, int keyLen, out byte[] key)
		{
			OutPacket p = new OutPacket();
			p.Write(K)
			 .WriteBytes(H)
			 .Write((byte)('A' + i))
			 .WriteBytes(sessionID);

			SHA1 hash = new SHA1Managed();

			byte[] h = hash.ComputeHash(p.Data);
			key = new byte[keyLen];
			Array.Copy(h, key, keyLen);
		}

		void ProcessNewKeys(InPacket p)
		{
			aesEnc = new AesManaged() { Key = this.C, IV = this.A };
			encryptor = aesEnc.CreateEncryptor();

			aesDec = new AesManaged() { Key = this.D, IV = this.B };
			decryptor = aesDec.CreateDecryptor();

			signer = new HMACSHA1(this.E);
			verifier = new HMACSHA1(this.F);

			if (authenticated == false)
			{
				OutPacket o = new OutPacket(SSH_MSG.SERVICE_REQUEST);
				o.Write("ssh-userauth");
				Send(o);
			}
		}

		void ProcessUserAuthSuccess(InPacket p)
		{
		}

		void ProcessUserAuthFailed(InPacket p)
		{
		}

		/// <summary>
		/// Closes the Socket connection and releases all associated resources
		/// </summary>
		public void Close()
		{
			if (_socket != null)
			{
				_socket.Close();
			}
		}
	}
}


