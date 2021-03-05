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
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Resources;

namespace Salt
{
	public struct Style
	{
		public bool bold, underline, blink, inverse, unerasable;
	}

	public class Char
	{
		private byte ch;
		private Style st;

		public byte Ch { get { return ch; } set { ch = value; Dirty = true; } }
		public Style Style { get { return st; } set { st = value; Dirty = true; } }
		public bool Dirty { get; set; }

		public bool Bold { set { st.bold = value; Dirty = true; } get { return st.bold; } }
		public bool Underline { set { st.underline = value; Dirty = true; } get { return st.underline; } }
		public bool Blink { set { st.blink = value; Dirty = true; } get { return st.blink; } }
		public bool Inverse { set { st.inverse = value; Dirty = true; } get { return st.inverse; } }
		public bool Unerasable { set { st.unerasable = value; Dirty = true; } get { return st.unerasable; } }

		public Char()
		{
			this.Clear();
		}

		public Char(Char p)
		{
			this.ch = p.ch;
			this.st = p.st;
			this.Dirty = p.Dirty;
		}

		public void Clear()
		{
			this.ch = 0x20;
			this.st = new Style();
			this.Dirty = true;
		}
	}

	public class Line : IEnumerable<Char>
	{
		private Char[] chars;

		public Line(int width)
		{
			chars = new Char[width];
			for (int c = 0; c < width; ++c)
				chars[c] = new Char();
		}

		public Char this[int column]
		{
			get { return chars[column]; }
			set { chars[column] = value; }
		}

		public int Length { get { return chars.Length; } }

		public bool SoftWrapped { get; set; }
		public bool DoubleWidth { get; set; }

		public void Resize(int width)
		{
			if (chars.Length < width)
			{
				Char[] tmp = new Char[width];
				for (int i = 0; i < chars.Length; ++i)
					tmp[i] = chars[i];
				chars = tmp;
			}
		}

		public IEnumerator<Char> GetEnumerator()
		{
			return chars.Cast<Char>().GetEnumerator();
		}

		System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
		{
			return chars.Cast<Char>().GetEnumerator();
		}

		internal void Insert(int inColumn)
		{
			throw new NotImplementedException();
		}
	}

	public class Buffer
	{
		private Line[] lines;
		public bool Dirty { get; set; }

		public Buffer(int width, int height)
		{
			lines = new Line[height];
			for (int i = 0; i < height; ++i)
				lines[i] = new Line(width);
		}

		public Char this[int column, int row]
		{
			get { return lines[row][column]; }
			set { lines[row][column] = value; }
		}

		public enum EraseMode { eFrom, eTo, eAny };

		public void Clear()
		{
			EraseDisplay(0, 0, EraseMode.eAny, false);
		}

		public void EraseDisplay(int x, int y, EraseMode mode, bool selective)
		{
			for (int l = 0; l < lines.Length; ++l)
			{
				Line line = lines[l];

				line.SoftWrapped = false;
				line.DoubleWidth = false;

				for (int c = 0; c < line.Length; ++c)
				{
					Char ch = line[c];

					if (selective && ch.Unerasable == false)
						continue;

					switch (mode)
					{
						case EraseMode.eFrom:
							if (l > y || (l == y && c >= x))
								line[c].Clear();
							break;
						case EraseMode.eTo:
							if (l < y || (l == y && c <= x))
								line[c].Clear();
							break;
						case EraseMode.eAny:
							line[c].Clear();
							break;
					}
				}
			}
		}

		void EraseLine(int inX, int inY, EraseMode inMode, bool inSelective)
		{
			if (inY >= lines.Length)
				return;

			Line line = lines[inY];
			line.SoftWrapped = false;
			//line.SetSingleWidth();

			int cf = 0, ct = lines.Length;
			switch (inMode)
			{
				case EraseMode.eFrom:	cf = inX; break;
				case EraseMode.eTo:		ct = inX + 1; break;
			}
	
			for (int c = cf; c < ct; ++c)
			{
				if (inSelective && line[c].Unerasable)
					continue;
				line[c] = new Char();
			}

			Dirty = true;
		}


		internal void ScrollBackward(int scrollTop, int scrollBottom)
		{
			if (scrollTop >= scrollBottom || scrollBottom >= lines.Length)
				return;

			Line tmp = lines[scrollBottom];
			for (int line = scrollBottom; line > scrollTop; --line)
				lines[line] = lines[line - 1];
			EraseLine(0, scrollTop, EraseMode.eAny, false);

			Dirty = true;
		}

		internal void ScrollForward(int scrollTop, int scrollBottom)
		{
			if (scrollTop >= scrollBottom || scrollBottom >= lines.Length)
				return;

			Line tmp = lines[scrollTop];
			for (int line = scrollTop; line < scrollBottom; ++line)
				lines[line] = lines[line + 1];
			lines[scrollBottom] = tmp;
			EraseLine(0, scrollBottom, EraseMode.eAny, false);

			Dirty = true;
		}

		internal void WrapLine(int inLine)
		{
			if (inLine < lines.Length)
				lines[inLine].SoftWrapped = true;
		}

		internal void InsertCharacter(int inLine, int inColumn)
		{
			if (inLine >= lines.Length)
				return;

			lines[inLine].Insert(inColumn);

			Dirty = true;
		}

		internal void SetCharacter(int inLine, int inColumn, byte inChar, Style inStyle)
		{
			if (inLine >= lines.Length)
				return;

			lines[inLine][inColumn].Ch = inChar;
			lines[inLine][inColumn].Style = inStyle;
			
			Dirty = true;
		}
	}

	public class Terminal
	{
		// This is what we report as terminal type:
		// 62	VT200 family
		// 1	132 columns
		// 2	printer port
		// 6	selective erase
		// 8	user-defined keys
		// 9	national replacement character-sets
		private static string kVT220Attributes = "\033[?62;1;2;6;8;9c";
		private static long kSmoothScrollDelay = 1666667;
		private static int
			kBackColor = (0x222f44 | ~0x00ffffff),
			kForeColor = (0xd0d7e0 | ~0x00ffffff),
			kBoldColor = (0xffffff | ~0x00ffffff),

			kTerminalPixelWidth = 640,
			kTerminalPixelHeight = 432,

			kTerminalCharWidth = 80,
			kTerminalCharHeight = 24,

			kCharWidth = kTerminalPixelWidth / kTerminalCharWidth,
			kCharHeight = kTerminalPixelHeight / kTerminalCharHeight;

		private int debugInt = 0;

		private Image image;
		private WriteableBitmap bm;
		private DispatcherTimer timer;

		private WriteableBitmap G0BM, G1BM;
		private Queue<byte>		mReceiveBuffer = new Queue<byte>();

		private Buffer			mBuffer;

		private int				mCursorX, mCursorY, mScrollTop, mScrollBottom;
		private Style			mCurStyle;
		private int				mCSGL, mCSGR;
		private int				mSS;
		private bool[]			mTabStops;
		private bool			mSavedCursor;
		private int				mSaveCursorX;
		private int				mSaveCursorY;
		//private const wchar_t*	mSaveCharSetG[4];
		private int				mSaveCSGL, mSaveCSGR;
		private Style			mSaveCurStyle;
		private bool			mSaveOrigin;
		private bool			mSaveSelectiveErase;
	
	// VT52 mode
		private bool			mVT52KeyPadMode;
					
	// DEC modes
		private bool			mDECCKM;
		private bool			mDECANM;
		private bool			mDECSCLM;
		private bool			mDECSCNM;
		private bool			mDECOM;
		private bool			mDECAWM;
		private bool			mDECARM;
		private bool			mDECPFF;
		private bool			mDECPEX;
		private bool			mDECKPAM;
	// VT220
		private int				mDECSCL;
		private bool			mDECTCEM;
		private bool			mDECNRCM;

	// ANSI modes
		private bool			mIRM;
		private bool			mKAM;
		private bool			mSRM;
		private bool			mLNM;
	
	// VT220 support
		private bool			mS8C1T;
		//private struct MDCS*	mDCS;		// device control strings
		//private struct MDCS*	mNewDCS;
	
	// handling of escape sequences
		internal delegate void Escape(byte ch);
		private Escape			mEscape;
		private int				mState;
		private List<int>		mArgs;
		private string			mArgString;
		private char			mCSIOption;
		private uint			mVT52Line;

	// some other state information
		private double			mLastBeep;
		private bool			mBlockCaret, mBlinkOn;
		private long			mLastBlink, mNextSmoothScroll;

		enum CursorMovement { left, up, right, down, tab, cr, lf, crlf, ind, ri };

#if DEBUG
		private bool			mDebugUpdate;
#endif

		public Terminal(Image img)
		{
			this.mBuffer = new Buffer(kTerminalPixelWidth, kTerminalPixelHeight);
			this.mBuffer.Clear();

			this.image = img;
			this.bm = new WriteableBitmap(kTerminalPixelWidth, kTerminalPixelHeight);
			this.image.Source = this.bm;
			Erase();

			this.timer = new DispatcherTimer();
			this.timer.Interval = TimeSpan.FromMilliseconds(500);
			this.timer.Tick += new EventHandler(Idle);
			this.timer.Start();

			this.mBlinkOn = false;
			this.mCursorX = this.mCursorY = 0;

			BitmapImage bitmapImage0 = new BitmapImage();
			bitmapImage0.CreateOptions = BitmapCreateOptions.None;
			bitmapImage0.ImageOpened += (s, e) =>
			{
				BitmapImage bm = (BitmapImage)s;
				this.G0BM = new WriteableBitmap(bm);
			};
			bitmapImage0.UriSource = new Uri("Images/us-ascii.png", UriKind.Relative);

			BitmapImage bitmapImage1 = new BitmapImage();
			bitmapImage1.CreateOptions = BitmapCreateOptions.None;
			bitmapImage1.ImageOpened += (s, e) =>
			{
				BitmapImage bm = (BitmapImage)s;
				this.G1BM = new WriteableBitmap(bm);
			};
			bitmapImage1.UriSource = new Uri("Images/graph.png", UriKind.Relative);

			Reset();

			StreamResourceInfo si = Application.GetResourceStream(new Uri("test-text.txt", UriKind.Relative));
			if (si != null)
			{
				var s = si.Stream;

				byte[] data = new byte[s.Length];
				s.Read(data, 0, (int)s.Length);

				ReceiveData(data);
			}
		}

		void Reset()
		{
		    mS8C1T = false;

		    mTabStops = new bool[kTerminalPixelWidth];
		    for (int i = 8; i < kTerminalPixelWidth; i += 8)
		        mTabStops[i] = true;

			//mEscape.clear();
		    mCursorX = 0;
		    mCursorY = 0;
		    mScrollTop = 0;
		    mScrollBottom = kTerminalPixelHeight - 1;
		    mCurStyle = new Style();

			//mCharSetG[0] = kUSCharSet;
			//mCharSetG[1] = kLineCharSet;
			//mCharSetG[2] = kUSCharSet;
			//mCharSetG[3] = kLineCharSet;
		    mCSGL = 0;
		    mCSGR = 2;
		    mSS = 0;

		    mIRM = false;
		    mKAM = false;
		    mLNM = false;
		    mSRM = true;

		    mDECCKM = false;
		    mDECANM = true;
		    mDECSCLM = false;
		    mDECSCNM = false;
		    mDECOM = false;
		    mDECAWM = true;
		    mDECARM = true;
		    mDECPFF = false;
		    mDECPEX = false;
		    mDECKPAM = false;

		    mDECSCL = 2;
		    mDECTCEM = true;
		    mDECNRCM = false;

		    mSavedCursor = false;
		    mNextSmoothScroll = 0;
		}

		void SoftReset()
		{
		    mDECTCEM = true;
		    mIRM = false;
		    mDECOM = false;
		    mDECAWM = true;	// should be false
		    mDECKPAM = false;
		    mDECCKM = false;
		    mScrollTop = 0;
		    mScrollBottom = kTerminalPixelHeight - 1;
		    mDECNRCM = false;

		    mSaveCurStyle = mCurStyle = new Style();
			//mSaveCharSetG[0] = mCharSetG[0] = kUSCharSet;
			//mSaveCharSetG[1] = mCharSetG[1] = kLineCharSet;
			//mSaveCharSetG[2] = mCharSetG[2] = kUSCharSet;
			//mSaveCharSetG[3] = mCharSetG[3] = kLineCharSet;

		    mSaveCSGL = mCSGL = 0;
		    mSaveCSGR = mCSGR = 2;
		}

		void Idle(object sender, EventArgs e)
		{
			bool update = false;
			int savedCursorX = mCursorX, savedCursorY = mCursorY;

			if (mReceiveBuffer.Count > 0 &&
				(mNextSmoothScroll == 0 || (Math.Abs(mNextSmoothScroll) <= DateTime.Now.Ticks)))
			{
				if (mNextSmoothScroll < 0)
					mBuffer.ScrollBackward(mScrollTop, mScrollBottom);
				else if (mNextSmoothScroll > 0)
					mBuffer.ScrollForward(mScrollTop, mScrollBottom);
		
				mNextSmoothScroll = 0;
		
				Emulate();
			}
	
			if (mDECTCEM /* && mChannel.Open */)
			{
				if (savedCursorX != mCursorX || savedCursorY != mCursorY)
				{
					mBlinkOn = true;
					mLastBlink = DateTime.Now.Ticks;
					update = true;
				}
				else if (Math.Abs(DateTime.Now.Ticks - mLastBlink) >= 0.66)
				{
					mBlinkOn = ! mBlinkOn;
					mLastBlink = DateTime.Now.Ticks;
					update = true;
				}
			}
	
			if (update || mBuffer.Dirty)
				Draw();
		}

		private void Erase()
		{
			int[] pixels = this.bm.Pixels;
			for (int y = 0; y < kTerminalPixelHeight; ++y)
			{
				for (int x = 0; x < kTerminalPixelWidth; ++x)
				{
					pixels[y * 640 + x] = kBackColor;
				}
			}

			this.bm.Invalidate();
		}

		private void Draw()
		{
			// see if we're inited yet
			if (G0BM == null || G1BM == null)
				return;

			// TODO: double width char support

			int[] p = this.bm.Pixels;
			int rx = this.bm.PixelWidth;

			for (int y = 0; y < kTerminalCharHeight; ++y)
			{
				int py = y * rx * kCharHeight;

				for (int x = 0; x < kTerminalCharWidth; ++x)
				{
					Char ch = new Char(mBuffer[x, y]);

					if (x == mCursorX && y == mCursorY && mBlinkOn)
						ch.Inverse = !ch.Inverse;

					if (mDECSCNM)
						ch.Inverse = !ch.Inverse;

					int[] s;
					int sx, ry;

					GetChar(ch, out s, out sx, out ry);

					int px = py + x * kCharWidth;
					for (int cy = 0; cy < kCharHeight; ++cy)
					{
						for (int cx = 0; cx < kCharWidth; ++cx)
						{
							p[px + cx] = s[sx + cx];
						}
						px += rx;
						sx += ry;
					}
				}
			}

			mBuffer.Dirty = false;
			this.bm.Invalidate();
		}

		private void GetChar(Char ch, out int[] s, out int sx, out int ry)
		{
			s = this.G0BM.Pixels;
			sx = 0;
			ry = this.G0BM.PixelWidth;

			if (ch.Ch >= 0x20 && ch.Ch <= 0x7f)
			{
				s = this.G0BM.Pixels;
				sx = kCharWidth * ((byte)ch.Ch - 0x20);

				if (ch.Bold)
					sx += kCharHeight * 4 * ry;
				if (ch.Underline)
					sx += kCharHeight * 2 * ry;
				if (ch.Inverse)
					sx += kCharHeight * 1 * ry;
			}
		}

		void ReceiveData(byte[] data)
		{
			foreach (byte b in data)
				mReceiveBuffer.Enqueue(b);
		    Idle(null, null);
		}


		void Emulate()
		{
			while (mReceiveBuffer.Count > 0)
			{
			    // break on a smooth scroll event
			    if (mNextSmoothScroll != 0)
			        break;

		//#if DEBUG
		//        if (mDebugUpdate && mBuffer->IsDirty())
		//        {
		//            Invalidate();
		//            GetWindow()->UpdateNow();
		//        }
		//#endif
			    // process bytes. We try to keep this code UTF-8 savvy
			    byte ch = mReceiveBuffer.Dequeue();

			    // start by testing if this byte is a control code
			    // The C1 control codes are never leading bytes in a UTF-8
			    // sequence and so we can easily check for them here.

			    if (ch < 0x20 || ch == 0x7f	||					// a C0 control code
			                                                    // or a C1 control code
			        (mDECSCL == 2 && mS8C1T && (ch & 0xe0) == 0x80))
			    {
			        switch (ch)
			        {
		/* ENQ */		case 0x05:
							//mChannel->SendData(Preferences::GetString("answer-back", "salt\r"));
			                                                                            break;
		/* BEL */		case 0x07: if (mEscape != null) mEscape(ch); else Beep();		break;
		/* BS  */		case 0x08: MoveCursor(CursorMovement.left);						break;
		/* HT  */		case 0x09: MoveCursor(CursorMovement.tab);						break;
		/* LF  */		case 0x0a:
		/* VT  */		case 0x0b:
		/* FF  */		case 0x0c: MoveCursor(mLNM ? CursorMovement.crlf : CursorMovement.lf);
																						break;
		/* CR  */		case 0x0d: MoveCursor(CursorMovement.cr);						break;
		/* SO  */		case 0x0e: mCSGL = 1;											break;
		/* SI  */		case 0x0f: mCSGL = 0;											break;
		/* CAN */		case 0x18: 
		/* SUB */		case 0x1a: WriteChar(0x00bf); mEscape = null;					break;
		/* ESC */		case 0x1b: 
			                if (mDECANM)
			                    mEscape = (b) => EscapeStart(b);
			                else
			                    mEscape = (b) => EscapeVT52(b);
			                break;
		/* IND */		case 0x84: MoveCursor(CursorMovement.ind);						break;
		/* NEL */		case 0x85: MoveCursor(CursorMovement.crlf);						break;
		/* HTS */		case 0x88: SetTabstop();										break;
		/* RI  */		case 0x8d: MoveCursor(CursorMovement.ri);						break;
		/* SS2 */		case 0x8e: mSS = 2;												break;
		/* SS3 */		case 0x8f: mSS = 3;												break;
		/* DCS */		case 0x90: mState = 0; mEscape = (b) => EscapeDCS(b, 0);		break;
		/* CSI */		case 0x9b: mState = 0; mEscape = (b) => EscapeCSI(b);			break;
		/* ST  */		case 0x9c: if (mEscape != null) mEscape(ch);					break;
		/* OSC */		case 0x9d: mState = 0; mEscape = (b) => EscapeOSC(b);			break;
			            default:			/* ignore */								break;
			        }
			        continue;
			    }

			    // So it wasn't a control code, now maybe we're processing an escape
			    // sequence, if so, feed the byte to the processor.

			    if (mEscape != null)
			    {
			        mEscape(ch);
			        continue;
			    }

			    // No control character and no escape sequence, so we have to write
			    // this character to the screen.
			    
			    // if it is an ascii character, map it using GL
			    if (ch >= 32 && ch < 127)		// GL
			    {
			        int set = mCSGL;
			        if (mSS != 0)
			            set = mSS;
					//uc = mCharSetG[set][ch - 32];
			    }
			    else							// GR
			    {	// no ascii, see if NRC is in use
			        if (mDECNRCM)
			        {
			            int set = mCSGR;
			            if (mSS != 0)
			                set = mSS;
						//uc = mCharSetG[set][ch - 32 - 128];
			        }
					//else	// no NRC, maybe the data should be interpreted as UTF-8
					//{
					//    if (mEncoding == kEncodingUTF8)
					//    {
					//        uint32 len = 1;
					//        if ((ch & 0x0E0) == 0x0C0)		len = 2;
					//        else if ((ch & 0x0F0) == 0x0E0)	len = 3;
					//        else if ((ch & 0x0F8) == 0x0F0)	len = 4;

					//        // be safe
					//        mReceiveBuffer.push_front(ch);
					//        static MEncodingTraits<kEncodingUTF8> traits;
					//        traits.ReadUnicode(mReceiveBuffer.begin(), len, uc);

					//        while (len-- > 0)
					//            mReceiveBuffer.pop_front();
					//    }
					//    else
					//        uc = MUnicodeMapping::GetUnicode(kEncodingISO88591, ch);
					//}
			    }

			    // reset the single shift code
			    mSS = 0;

			    // and write the final unicode to the screen
			    WriteChar(ch);
			}
		}

		private void SetTabstop()
		{
			if (mCursorX < kTerminalCharWidth)
				mTabStops[mCursorX] = true;
		}

		int GetParam(int inParamNr, int inDefault)
		{
			int result = inDefault;
			if (inParamNr < mArgs.Count)
				result = mArgs[inParamNr];
			return result;
		}

		private void EscapeOSC(byte b)
		{
			throw new NotImplementedException();
		}

		private void EscapeCSI(byte b)
		{
			throw new NotImplementedException();
		}

		private void EscapeDCS(byte b, int p)
		{
			throw new NotImplementedException();
		}

		private void EscapeVT52(byte b)
		{
			throw new NotImplementedException();
		}

		private void EscapeStart(byte inChar)
		{
			mEscape = null;
			mArgs.Clear();
			mState = 0;

			switch ((char)inChar)
			{
			    // VT100 escape codes
			    case '<':	mDECANM = true;									break;
			    case '=':	mDECKPAM = true;								break;
			    case '>':	mDECKPAM = false;								break;
			    case 'D':	MoveCursor(CursorMovement.ind);					break;
			    case 'M':	MoveCursor(CursorMovement.ri);					break;
			    case 'E':	MoveCursor(CursorMovement.crlf);				break;
			    case '7':	SaveCursor();									break;
			    case '8':	RestoreCursor();								break;
			    case 'H':	SetTabstop();									break;
			    case '(':	mEscape = (b) => SelectCharSet(b, 0);			break;
			    case ')':	mEscape = (b) => SelectCharSet(b, 1);			break;
			    case 'N':	mSS = 2;										break;
			    case 'O':	mSS = 3;										break;
			    case '#':	mEscape = (b) => EscapeDouble(b);				break;
			    case '[':	mEscape = (b) => EscapeCSI(b);					break;
			    case 'Z':	SendCommand(kVT220Attributes);					break;
			    case 'c':	Reset();										break;

			    // VT220 escape codes
			    case 'P':	mEscape = (b) => EscapeDCS(b, 0);				break;
			    case '*':	mEscape = (b) => SelectCharSet(b, 2);			break;
			    case '+':	mEscape = (b) => SelectCharSet(b, 3);			break;
			    case '~':	mCSGR = 1;										break;
			    case 'n':	mCSGL = 2;										break;
			    case '}':	mCSGR = 2;										break;
			    case 'o':	mCSGL = 3;										break;
			    case '|':	mCSGR = 3;										break;
			    case ' ':	mEscape = (b) => SelectControlTransmission(b);  break;
				//case '\\':	if (mNewDCS) CommitDCS();					break;

			    // xterm support
			    case ']':	mEscape = (b) => EscapeOSC(b);					break;
			    default:	/* ignore */									break;
			}
		}

		private void SaveCursor()
		{
			throw new NotImplementedException();
		}

		private void RestoreCursor()
		{
			throw new NotImplementedException();
		}

		private void SendCommand(string kVT220Attributes)
		{
			throw new NotImplementedException();
		}

		private void EscapeDouble(byte b)
		{
			throw new NotImplementedException();
		}

		private void SelectControlTransmission(byte b)
		{
			throw new NotImplementedException();
		}

		private void SelectCharSet(byte b, int p)
		{
			throw new NotImplementedException();
		}

		private void MoveCursor(CursorMovement inDirection)
		{
			switch (inDirection)
			{
				case CursorMovement.up:
					if (mCursorY > mScrollTop)
						--mCursorY;
					else if (mCursorY > 0)
						ScrollBackward();
					break;

				case CursorMovement.down:
					if (mCursorY < mScrollBottom)
						++mCursorY;
					break;

				case CursorMovement.right:
					if (mCursorX < kTerminalCharWidth - 1)
						++mCursorX;
					break;

				case CursorMovement.left:
					if (mCursorX >= kTerminalCharWidth - 1)
						mCursorX = kTerminalCharWidth - 2;
					else if (mCursorX > 0)
						--mCursorX;
					break;

				case CursorMovement.ind:
					if (mCursorY < mScrollBottom)
						++mCursorY;
					else
						ScrollForward();
					break;

				// same as MoveUp
				case CursorMovement.ri:
					if (mCursorY > mScrollTop)
						--mCursorY;
					else
						ScrollBackward();
					break;

				case CursorMovement.lf:
					if (mCursorY < mScrollBottom)
						++mCursorY;
					else
						ScrollForward();
					break;

				case CursorMovement.cr:
					mCursorX = 0;
					break;

				case CursorMovement.crlf:
					mCursorX = 0;
					if (mCursorY < mScrollBottom)
						++mCursorY;
					else
						ScrollForward();
					break;

				case CursorMovement.tab:
					while (mCursorX < kTerminalCharWidth - 1)
					{
						++mCursorX;
						if (mTabStops[mCursorX])
							break;
					}
					break;
			}
		}

		private void Beep()
		{
			throw new NotImplementedException();
		}

		void WriteChar(byte inChar)
		{
			if (mCursorX >= kTerminalCharWidth)
			{
				if (mDECAWM)
				{
					mBuffer.WrapLine(mCursorY);
					if (mCursorY < mScrollBottom)
						++mCursorY;
					else
						ScrollForward();
					mCursorX = 0;
				}
				else
					mCursorX = kTerminalCharWidth - 1;
			}

			if (mIRM)
				mBuffer.InsertCharacter(mCursorY, mCursorX);

			mBuffer.SetCharacter(mCursorY, mCursorX, inChar, mCurStyle);

			++mCursorX;
		}

		private void ScrollForward()
		{
			if (mDECSCLM)
				mNextSmoothScroll = DateTime.Now.Ticks + kSmoothScrollDelay;
			else
				mBuffer.ScrollForward(mScrollTop, mScrollBottom);
		}

		private void ScrollBackward()
		{
			if (mDECSCLM)
				mNextSmoothScroll = DateTime.Now.Ticks + kSmoothScrollDelay;
			else
				mBuffer.ScrollBackward(mScrollTop, mScrollBottom);
		}
	
	}

//bool MTerminalView::HandleKeydown(
//    uint32				inKeyCode,
//    uint32				inModifiers,
//    const string&		inText,
//    bool				inRepeat)
//{
//    // shortcut
//    if (inRepeat and mDECARM == false)
//        return true;

//    bool handled = false;
	
//    if (not mChannel->IsOpen())
//    {
//        if (inKeyCode == kSpaceKeyCode or inKeyCode == kReturnKeyCode)
//        {
//            mChannel->Open();
//            handled = true;
//        }
//    }
//    else
//    {
//        string text(inText);
		
//        if (inModifiers & kNumPad)
//        {
//            if (mDECANM)
//                text = "\033O";
//            else
//                text = "\033?";

//            if (inKeyCode == kNumlockKeyCode)
//                text += 'P';
//            else if (inKeyCode == kDivideKeyCode)
//                text += 'Q';
//            else if (inKeyCode == kMultiplyKeyCode)
//                text += 'R';
//            else if (inKeyCode == kSubtractKeyCode)
//                text += 'S';
//            else if ((mDECANM and not mDECKPAM) or (not mDECANM and not mVT52KeyPadMode))
//            {
//                if ((inKeyCode >= '0' and inKeyCode <= '9') or inKeyCode == '-' or inKeyCode == '.')
//                    text = char(inKeyCode);
//                else
//                    text.clear();
//            }
//            else
//            {
//                switch (inKeyCode)
//                {
//                    case '+': text += 'l'; break; // remap + to , of the DEC keyboard
//                    case '-': text += 'm'; break;
//                    case '.': text += 'n'; break;
//                    case kEnterKeyCode:
//                    case kReturnKeyCode: text += 'M'; break;
//                    default:
//                        if (inKeyCode >= '0' and inKeyCode <= '9')
//                            text += char('p' + inKeyCode - '0');
//                        else
//                            text.clear();
//                        break;
//                }
//            }

//            handled = true;
//        }

//        // VT220, device control strings
//        if (not handled and
//            inModifiers == kShiftKey and
//            inKeyCode >= kF6KeyCode and inKeyCode <= kF20KeyCode and
//            mDCS != nullptr)
//        {
//            switch (inKeyCode)
//            {
//                case kF6KeyCode: text = mDCS->key[eF6]; break;
//                case kF7KeyCode: text = mDCS->key[eF7]; break;
//                case kF8KeyCode: text = mDCS->key[eF8]; break;
//                case kF9KeyCode: text = mDCS->key[eF9]; break;
//                case kF10KeyCode: text = mDCS->key[eF10]; break;
//                case kF11KeyCode: text = mDCS->key[eF11]; break;
//                case kF12KeyCode: text = mDCS->key[eF12]; break;
//                case kF13KeyCode: text = mDCS->key[eF13]; break;
//                case kF14KeyCode: text = mDCS->key[eF14]; break;
//                case kF15KeyCode: text = mDCS->key[eHelp]; break;
//                case kF16KeyCode: text = mDCS->key[eDo]; break;
//                case kF17KeyCode: text = mDCS->key[eF17]; break;
//                case kF18KeyCode: text = mDCS->key[eF18]; break;
//                case kF19KeyCode: text = mDCS->key[eF19]; break;
//                case kF20KeyCode: text = mDCS->key[eF20]; break;
//            }
//            handled = true;
//        }
		
//        if (not handled and inModifiers == 0)
//        {
//            const string kCSI("\033["), kSS3("\033O");
//            string prefix;
//            if (mDECANM == false)
//                prefix = "\033";
//            else if (mDECCKM)
//                prefix = kSS3;
//            else
//                prefix = kCSI;

//            switch (inKeyCode)
//            {
//                case kEscapeKeyCode:		text = "\033"; break;
////				case kSpaceKeyCode:			if (mDECCKM) text = kSS3 + ' '; else text = " "; break;
////				case kTabKeyCode:			if (mDECCKM) text = kSS3 + 'I'; else text = "\t";break;
//                case kEnterKeyCode:
//                    if (mVT52KeyPadMode)	{ text = "\033?M"; break; }
//                    if (mDECKPAM)			{ text = "\033OM"; break; }
//                case kReturnKeyCode:
//                    if (not mBuffer->IsSelectionEmpty())
//                    {
//                        MClipboard::Instance().SetData(mBuffer->GetSelectedText(),
//                            mBuffer->IsSelectionBlock());
//                        mBuffer->ClearSelection();
//                        text.clear();
//                        handled = true;
//                    }
//                    else if (mLNM)
//                        text = "\r\n";
//                    else
//                        text = "\r";
//                    break;
//                case kBackspaceKeyCode:		text = '\177'; break;
//                case kHomeKeyCode:			text = kCSI + "1~"; break;
//                case kInsertKeyCode:		text = kCSI + "2~"; break;
//                case kDeleteKeyCode:		text = kCSI + "3~"; break;
//                case kEndKeyCode:			text = kCSI + "4~"; break;
//                case kPageUpKeyCode:		text = kCSI + "5~"; break;
//                case kPageDownKeyCode:		text = kCSI + "6~"; break;
//                case kUpArrowKeyCode:		text = prefix + 'A'; break;
//                case kDownArrowKeyCode:		text = prefix + 'B'; break;
//                case kRightArrowKeyCode:	text = prefix + 'C'; break;
//                case kLeftArrowKeyCode:		text = prefix + 'D'; break;
//                //// use F1..F4 as PF1..PF4
//                //case kF1KeyCode:			if (mDECANM) text = "\033OP"; else text = "\033P"; break;
//                //case kF2KeyCode:			if (mDECANM) text = "\033OQ"; else text = "\033Q"; break;
//                //case kF3KeyCode:			if (mDECANM) text = "\033OR"; else text = "\033R"; break;
//                //case kF4KeyCode:			if (mDECANM) text = "\033OS"; else text = "\033S"; break;
				
//                // xterm compatibility
////				case kF1KeyCode:			text = kSS3 + 'P'; break;
////				case kF2KeyCode:			text = kSS3 + 'Q'; break;
////				case kF3KeyCode:			text = kSS3 + 'R'; break;
////				case kF4KeyCode:			text = kSS3 + 'S'; break;
//                case kF1KeyCode:			text = kCSI + "11~";  break;
//                case kF2KeyCode:			text = kCSI + "12~";  break;
//                case kF3KeyCode:			text = kCSI + "13~";  break;
//                case kF4KeyCode:			text = kCSI + "14~";  break;
//                case kF5KeyCode:			text = kCSI + "15~";  break;

//                // VT220
//                case kF6KeyCode:			if (mDECSCL == 2) text = kCSI + "17~"; break;
//                case kF7KeyCode:			if (mDECSCL == 2) text = kCSI + "18~"; break;
//                case kF8KeyCode:			if (mDECSCL == 2) text = kCSI + "19~"; break;
//                case kF9KeyCode:			if (mDECSCL == 2) text = kCSI + "20~"; break;
//                case kF10KeyCode:			if (mDECSCL == 2) text = kCSI + "21~"; break;
//                case kF11KeyCode:			if (mDECSCL == 2) text = kCSI + "23~"; else text = "\033"; break;
//                case kF12KeyCode:			if (mDECSCL == 2) text = kCSI + "24~"; else text = "\010"; break;
//                case kF13KeyCode:			if (mDECSCL == 2) text = kCSI + "25~"; else text = "\012"; break;
//                case kF14KeyCode:			if (mDECSCL == 2) text = kCSI + "26~"; break;
//                case kF15KeyCode:			if (mDECSCL == 2) text = kCSI + "28~"; break;
//                case kF16KeyCode:			if (mDECSCL == 2) text = kCSI + "29~"; break;
//                case kF17KeyCode:			if (mDECSCL == 2) text = kCSI + "31~"; break;
//                case kF18KeyCode:			if (mDECSCL == 2) text = kCSI + "32~"; break;
//                case kF19KeyCode:			if (mDECSCL == 2) text = kCSI + "33~"; break;
//                case kF20KeyCode:			if (mDECSCL == 2) text = kCSI + "34~"; break;
//            }
//        }
//        else if (inModifiers & kShiftKey)
//        {
//            // mimic xterm behaviour
//            switch (inKeyCode)
//            {
//                case kPageUpKeyCode:	Scroll(kScrollPageUp);		break;
//                case kPageDownKeyCode:	Scroll(kScrollPageDown);	break;
//            }
//        }
//        else if (inModifiers & kControlKey)
//        {
//#pragma warning("Test dit!!!")		// TODO test dit
//            // hope this works... (change in handling ctrl/shift in winprocmixin)
//            if (not text.empty())
//                inKeyCode = text[0];

//            switch (inKeyCode)
//            {
//                case kPageUpKeyCode:	Scroll(kScrollPageUp);		break;
//                case kPageDownKeyCode:	Scroll(kScrollPageDown);	break;
//                case kUpArrowKeyCode:	Scroll(kScrollLineUp);		break;
//                case kDownArrowKeyCode:	Scroll(kScrollLineDown);	break;
//                case kHomeKeyCode:		Scroll(kScrollToStart);		break;
//                case kEndKeyCode:		Scroll(kScrollToEnd);		break;
//                case '2':
//                case ' ':				text = '\000'; break;
//                case '3':				text = '\033'; break;
//                case '4':				text = '\034'; break;
//                case '5':				text = '\035'; break;
//                case '6':				text = '\036'; break;
//                case '7':				text = '\037'; break;
//                case '8':				text = '\177'; break;
//                default:
//                    // check to see if this is a decent control key
//                    if (inKeyCode >= '@' and inKeyCode < '`')
//                        text = char(inKeyCode - '@');
//                    else if (inKeyCode == kCancelKeyCode)
//                        text = kControlBreakMessage;
//                    break;
//            }
//        }
		
//        // brain dead for now
//        if (not text.empty())
//        {
//            mBuffer->ClearSelection();

//            if (mKAM or not mChannel->IsOpen())
//                PlaySound("warning");
//            else
//            {
//                SendCommand(text);
//                if (not mSRM)
//                    ReceiveData(text.c_str(), text.length());
//            }

//            handled = true;
//        }
//    }
	
//    if (handled)
//    {
//        mLastBlink = 0;
//        mBlinkOn = true;

//        if (mBuffer->IsDirty())
//            Invalidate();
		
//        ObscureCursor();
//    }
//    else
//        handled = MHandler::HandleKeydown(inKeyCode, inModifiers, inText, inRepeat);

//    return handled;
//}

//void MTerminalView::SendCommand(
//    string			inData)
//{
//    if (mChannel->IsOpen())
//    {
//        if (not mS8C1T)
//            mChannel->MTerminalChannel::SendData(inData);
//        else
//        {
//            ba::replace_all(inData, "\033D", "\204");	// IND
//            ba::replace_all(inData, "\033E", "\205");	// NEL
//            ba::replace_all(inData, "\033H", "\210");	// HTS
//            ba::replace_all(inData, "\033M", "\215");	// RI
//            ba::replace_all(inData, "\033N", "\216");	// SS2
//            ba::replace_all(inData, "\033O", "\217");	// SS3
//            ba::replace_all(inData, "\033P", "\220");	// DCS
//            ba::replace_all(inData, "\033[", "\233");	// CSI
//            ba::replace_all(inData, "\033\\", "\234");	// ST
//            ba::replace_all(inData, "\033]", "\235");	// OSC
//            mChannel->MTerminalChannel::SendData(inData);
//        }
//    }
//}

//void MTerminalView::Opened()
//{
//    value_changer<int32> x(mCursorX, 0), y(mCursorY, 0);
//    mStatusbar->SetStatusText(1, mChannel->GetEncryptionParams(), false);
//    Reset();
//    //EraseInDisplay(2);
//    Invalidate();
//}

//void MTerminalView::Closed()
//{
//    mStatusbar->SetStatusText(1, "", false);
//    mBlinkOn = true;
//    Invalidate();
//    const char kReconnectMsg[] = "Press enter or space to reconnect\r\n";
//    string reconnectMsg = _(kReconnectMsg);
//    ReceiveData(reconnectMsg.c_str(), reconnectMsg.length());
//}

//void MTerminalView::ChannelMessage(const string& inMessage)
//{
//    mStatusbar->SetStatusText(0, inMessage, false);
//}

//void MTerminalView::ChannelError(const string& inError)
//{
//    const char kInitForError[] = "\r\n\033[0J\r\n\033[0;1m";
//    ReceiveData(kInitForError, sizeof(kInitForError) - 1);
//    ReceiveData(inError.c_str(), inError.length());
//}

//void MTerminalView::ChannelBanner(const string& inBanner)
//{
//    value_changer<bool> save(mLNM, true);
//    ReceiveData(inBanner.c_str(), inBanner.length());
//}

//bool MTerminalView::WantPTY(
//    uint32&				outWidth,
//    uint32&				outHeight,
//    string&				outTerminalType,
//    bool&				outForwardAgent,
//    bool&				outForwardX11)
//{
//    outWidth = mTerminalWidth;
//    outHeight = mTerminalHeight;
//    outTerminalType = Preferences::GetString("terminal-type", "vt220");
//    outForwardAgent = Preferences::GetBoolean("forward-agent", true);
//    outForwardX11 = Preferences::GetBoolean("forward-x11", true);
//    return true;
//}

//// --------------------------------------------------------------------
////

//void MTerminalView::EraseInDisplay(
//    uint32				inMode,
//    bool				inSelective)
//{
//    mBuffer->EraseDisplay(mCursorX, mCursorY, inMode, inSelective);
//}

//void MTerminalView::EraseInLine(
//    uint32				inMode,
//    bool				inSelective)
//{
//    mBuffer->EraseLine(mCursorX, mCursorY, inMode, inSelective);
//}

//void MTerminalView::EraseCharacter(
//    uint32				inCount)
//{
//    mBuffer->EraseCharacter(mCursorX, mCursorY, inCount);
//}

//void MTerminalView::InsertLine()
//{
//    if (mCursorY >= mScrollTop and mCursorY <= mScrollBottom)
//        mBuffer->InsertLine(mCursorY, mScrollBottom);
//}

//void MTerminalView::DeleteLine()
//{
//    if (mCursorY >= mScrollTop and mCursorY <= mScrollBottom)
//        mBuffer->DeleteLine(mCursorY, mScrollBottom);
//}

//void MTerminalView::DeleteCharacter()
//{
//    if (mCursorY >= 0 and mCursorY < mTerminalHeight)
//        mBuffer->DeleteCharacter(mCursorY, mCursorX);
//}

//void MTerminalView::InsertCharacter()
//{
//    if (mCursorY >= 0 and mCursorY < mTerminalHeight)
//        mBuffer->InsertCharacter(mCursorY, mCursorX);
//}

//void MTerminalView::ScrollForward()
//{
//    if (mDECSCLM)
//        mNextSmoothScroll = GetLocalTime() + kSmoothScrollDelay;
//    else
//        mBuffer->ScrollForward(mScrollTop, mScrollBottom);
//}

//void MTerminalView::ScrollBackward()
//{
//    if (mDECSCLM)
//        mNextSmoothScroll = -(GetLocalTime() + kSmoothScrollDelay);
//    else
//        mBuffer->ScrollBackward(mScrollTop, mScrollBottom);
//}

//void MTerminalView::MoveCursorTo(
//    int32				inX,
//    int32				inY)
//{
//    if (mDECOM)
//        inY += mScrollTop;
	
//    mCursorX = inX;
//    if (mCursorX < 0)
//        mCursorX = 0;
//    if (mCursorX >= mTerminalWidth)
//        mCursorX = mTerminalWidth - 1;

//    mCursorY = inY;
//    if (mCursorY < 0)
//        mCursorY = 0;
//    if (mCursorY >= mTerminalHeight)
//        mCursorY = mTerminalHeight - 1;

//    if (mDECOM)
//    {
//        if (mCursorY < mScrollTop)
//            mCursorY = mScrollTop;
//        if (mCursorY > mScrollBottom)
//            mCursorY = mScrollBottom;
//    }

//    if (mCursorY > mScrollBottom)
//    {
//        ScrollForward();
//        mCursorY = mScrollBottom;
//    }
//}

//uint32 MTerminalView::GetParam(
//    uint32				inParamNr,
//    uint32				inDefault)
//{
//    uint32 result = inDefault;
//    if (inParamNr < mArgs.size())
//        result = mArgs[inParamNr];
//    return result;
//}

//void MTerminalView::EscapeVT52(
//    char				inChar)
//{
//    mEscape.clear();
//    switch (inChar)
//    {
//        case '<':	mDECANM = true;				break;
//        case 'A':	MoveCursor(kMoveUp);		break;
//        case 'B':	MoveCursor(kMoveDown);		break;
//        case 'C':	MoveCursor(kMoveRight);		break;
//        case 'D':	MoveCursor(kMoveLeft);		break;
//        case 'H':	MoveCursorTo(0, 0);			break;
//        case 'Y':	mEscape = boost::bind(&MTerminalView::EscapeVT52_Y, this, _1, true); break;
//        case 'I':	MoveCursor(kMoveRI);		break;
//        case '=':	mVT52KeyPadMode = true;		break;
//        case '>':	mVT52KeyPadMode = false;	break;
//        case 'F':	mCSGL = 1;					break;
//        case 'G':	mCSGL = 0;					break;
//        case 'K':	EraseInLine(0);				break;
//        case 'J':	EraseInDisplay(0);			break;
//        case 'W':
//        case 'X':
//        case 'V':
//        case ']':	/* ignore */				break;	// printing controls
//        case 'Z':	SendCommand("\033/Z");		break;
//    }
//}

//void MTerminalView::EscapeVT52_Y(
//    char				inChar,
//    bool				inLine)
//{
//    if (inLine)
//    {
//        mVT52Line = inChar - 037;
//        mEscape = boost::bind(&MTerminalView::EscapeVT52_Y, this, _1, false);
//    }
//    else
//    {
//        MoveCursorTo((inChar - 037) - 1, mVT52Line - 1);
//        mEscape.clear();
//    }
//}

//void MTerminalView::EscapeStart(
//    char				inChar)
//{
//    mEscape.clear();
//    mArgs.clear();
//    mState = 0;
	
//    switch (inChar)
//    {
//        // VT100 escape codes
//        case '<':	mDECANM = true;														break;
//        case '=':	mDECKPAM = true;													break;
//        case '>':	mDECKPAM = false;													break;
//        case 'D':	MoveCursor(kMoveIND);												break;
//        case 'M':	MoveCursor(kMoveRI);												break;
//        case 'E':	MoveCursor(kMoveCRLF);												break;
//        case '7':	SaveCursor();														break;
//        case '8':	RestoreCursor();													break;
//        case 'H':	SetTabstop();														break;
//        case '(':	mEscape = boost::bind(&MTerminalView::SelectCharSet, this, _1, 0);	break;
//        case ')':	mEscape = boost::bind(&MTerminalView::SelectCharSet, this, _1, 1);	break;
//        case 'N':	mSS = 2;															break;
//        case 'O':	mSS = 3;															break;
//        case '#':	mEscape = boost::bind(&MTerminalView::EscapeDouble, this, _1);		break;
//        case '[':	mEscape = boost::bind(&MTerminalView::EscapeCSI, this, _1);			break;
//        case 'Z':	SendCommand(kVT220Attributes);										break;
//        case 'c':	Reset();															break;

//        // VT220 escape codes
//        case 'P':	mEscape = boost::bind(&MTerminalView::EscapeDCS, this, _1, 0);		break;
//        case '*':	mEscape = boost::bind(&MTerminalView::SelectCharSet, this, _1, 2);	break;
//        case '+':	mEscape = boost::bind(&MTerminalView::SelectCharSet, this, _1, 3);	break;
//        case '~':	mCSGR = 1;															break;
//        case 'n':	mCSGL = 2;															break;
//        case '}':	mCSGR = 2;															break;
//        case 'o':	mCSGL = 3;															break;
//        case '|':	mCSGR = 3;															break;
//        case ' ':	mEscape = boost::bind(&MTerminalView::SelectControlTransmission, this, _1);
//                                                                                        break;
//        case '\\':	if (mNewDCS) CommitDCS();											break;

//        // xterm support
//        case ']':	mEscape = boost::bind(&MTerminalView::EscapeOSC, this, _1);			break;
		
//        default:	/* ignore */														break;
//    }
//}

//void MTerminalView::EscapeCSI(char inChar)
//{
//    bool done = false;
	
//    switch (mState)
//    {
//        case 0:
//            mCSIOption = 0;
//            mArgs.clear();
//            mArgs.push_back(0);
//            switch (inChar)
//            {
//                case '!':
//                case '?':
//                case '>':
//                    mCSIOption = inChar;
//                    mState = 1;
//                    break;
				
//                case ';':
//                    mArgs.push_back(0);
//                    mState = 1;
//                    break;
				
//                default:
//                    if (inChar >= '0' and inChar <= '9')
//                    {
//                        mArgs[0] = inChar - '0';
//                        mState = 1;
//                    }
//                    else
//                        done = true;
//                    break;
//            }
//            break;
		
//        case 1:
//            if (inChar >= '0' and inChar <= '9')
//                mArgs.back() = mArgs.back() * 10 + (inChar - '0');
//            else if (inChar == ';')
//                mArgs.push_back(0);
//            else if (inChar == '"' and (mArgs[0] == 61 or mArgs[0] == 62))	// for DECSCL
//                mState = 2;
//            else
//                done = true;
//            break;
		
//        case 2:
//            mEscape.clear();	
//            if (inChar == 'p')
//            {
//                if (mArgs[0] == 61)
//                {
//                    mDECSCL = 1;
//                    mS8C1T = false;
//                }
//                else if (mArgs[0] == 62)
//                {
//                    mDECSCL = 2;
//                    if (mArgs.size() == 1 or mArgs[1] == 0 or mArgs[1] == 2)
//                        mS8C1T = true;
//                    else if (mArgs[1] == 1)
//                        mS8C1T = false;
//                }
//            }
//            break;
//    }
	
//    if (done)
//    {
//        mEscape.clear();

//        bool set = false;

//        uint32 n = GetParam(0, 1);
//        if (n == 0)
//            n = 1;

//        switch (inChar)
//        {
//            case '@':	while (n-- > 0) InsertCharacter();								break;
//            case 'A':	while (n-- > 0) MoveCursor(kMoveUp);							break;
//            case 'B':	while (n-- > 0) MoveCursor(kMoveDown);							break;
//            case 'C':	while (n-- > 0) MoveCursor(kMoveRight);							break;
//            case 'D':	while (n-- > 0) MoveCursor(kMoveLeft);							break;
//            case 'E':	while (n-- > 0) { MoveCursor(kMoveCRLF); }						break;
//            case 'F':	while (n-- > 0) { MoveCursor(kMoveCR); MoveCursor(kMoveUp); }	break;
//            case 'G':	MoveCursorTo(GetParam(0, 1) - 1, mCursorY);						break;
//            case 'H':	MoveCursorTo(GetParam(1, 1) - 1, GetParam(0, 1) - 1);			break;
//            case 'J':	EraseInDisplay(GetParam(0, 0), mCSIOption == '?');				break;
//            case 'K':	EraseInLine(GetParam(0, 0), mCSIOption == '?');					break;
//            case 'L':	while (n-- > 0) InsertLine();									break;
//            case 'M':	while (n-- > 0) DeleteLine();									break;
//            case 'P':	while (n-- > 0) DeleteCharacter();								break;
//            case 'X':	EraseCharacter(n);												break;
			
//            case 'c':
//                if (mCSIOption == '>')
//                    SendCommand("\033[>1;20;0c"); // VT220 (version 2.0 of salt)
//                else
//                    SendCommand(kVT220Attributes);
//                break;
//            case 'f':	MoveCursorTo(GetParam(1, 1) - 1, GetParam(0, 1) - 1);			break;
//            case 'g':
//                if (GetParam(0, 0) == 3)
//                    fill(mTabStops.begin(), mTabStops.end(), false);
//                else if (GetParam(0, 0) == 0 and mCursorX < mTerminalWidth)
//                    mTabStops[mCursorX] = false;
//                break;

//            case 'h':	set = true;
//            case 'l':
//                foreach (uint32 a, mArgs)
//                    SetResetMode(a, mCSIOption == 0, set);
//                break;
			
//            case 'm':
//                foreach (uint32 a, mArgs)
//                {
//                    switch (a)
//                    {
//                        case 0:			mCurStyle = kStyleNormal; break;
//                        case 1:			mCurStyle |= kStyleBold; break;
//                        case 4:			mCurStyle |= kStyleUnderline; break;
//                        case 5:			mCurStyle |= kStyleBlink; break;
//                        case 7:			mCurStyle |= kStyleInverse; break;
//                        case 8:			mCurStyle |= kStyleInvisible; break;
//                        case 22:		mCurStyle &= ~kStyleBold; break;
//                        case 24:		mCurStyle &= ~kStyleUnderline; break;
//                        case 25:		mCurStyle &= ~kStyleBlink; break;
//                        case 27:		mCurStyle &= ~kStyleInverse; break;
//                    }
//                }
//                break;
			
//            case 'n':
//                switch (mArgs[0])
//                {
//                    case 5:	 SendCommand("\033[0n");		break; // terminal OK
//                    case 6:  SendCommand(boost::format("\033[%d;%dR") % (mCursorY + 1) % (mCursorX + 1));
//                                                            break;
//                    case 15: SendCommand("\033[?13n");		break; // we have no printer
//                    case 25: SendCommand(boost::format("\033[?2%dn") % (mDCS != nullptr and mDCS->locked));
//                                                            break;
//#pragma message("Find out the keyboard layout")
//                    case 26: SendCommand("\033[?27;1n");	break; // report a US keyboard for now
//                }
//                break;
			
//            case 'p':	 // DECSTR
//                if (mCSIOption == '!')
//                    SoftReset();
//                break;
			
//            case 'r':
//            {
//                int32 top = GetParam(0, 1);
//                if (top < 1)
//                    top = 1;
//                int32 bottom = GetParam(1, mTerminalHeight);
//                if (bottom > mTerminalHeight)
//                    bottom = mTerminalHeight;
//                if (bottom < top + 1)
//                    bottom = top + 1;
				
//                mScrollTop = top - 1;
//                mScrollBottom = bottom - 1;

//                MoveCursorTo(0, 0);
//                break;
//            }
			
//            case 's':
//                SaveCursor();
//                break;
			
//            case 'x':
//                SendCommand((boost::format("\033[%d;1;1;128;128;1;0x") % (mArgs[0] + 2)).str());
//                break;
			
//            case '"':	mEscape = boost::bind(&MTerminalView::SelectCharAttr, this, _1); break;
//        }
//    }
//}

//void MTerminalView::SelectCharSet(
//    char				inChar,
//    int					inCharSet)
//{
//    mEscape.clear();
	
//    if (mDECSCL == 1 and inCharSet > 1)
//    {
//        PRINT(("Unsupported set G%d", inCharSet));
//        return;
//    }
	
//    switch (inChar)
//    {
//        case 'A':	mCharSetG[inCharSet] = kUKCharSet;			break;
//        case 'B':	mCharSetG[inCharSet] = kUSCharSet;			break;
//        case '4':	mCharSetG[inCharSet] = kNLCharSet;			break;
//        case 'C':
//        case '5':	mCharSetG[inCharSet] = kFICharSet;			break;
//        case 'R':	mCharSetG[inCharSet] = kFRCharSet;			break;
//        case 'Q':	mCharSetG[inCharSet] = kCACharSet;			break;
//        case 'K':	mCharSetG[inCharSet] = kDECharSet;			break;
//        case 'Y':	mCharSetG[inCharSet] = kITCharSet;			break;
//        case 'E':
//        case '6':	mCharSetG[inCharSet] = kDKCharSet;			break;
//        case 'Z':	mCharSetG[inCharSet] = kSPCharSet;			break;
//        case 'H':
//        case '7':	mCharSetG[inCharSet] = kSECharSet;			break;
//        case '=':	mCharSetG[inCharSet] = kCHCharSet;			break;
//        case '0':	mCharSetG[inCharSet] = kLineCharSet;		break;
//        case '1':	mCharSetG[inCharSet] = kUSCharSet;			break;
//        case '2':	mCharSetG[inCharSet] = kLineCharSet;		break;
//    }
//}

//void MTerminalView::SelectControlTransmission(char inCode)
//{
//    if (inCode == 'F')
//        mS8C1T = false;
//    else if (inCode == 'G' and mDECSCL == 2)
//        mS8C1T = true;
//    mEscape.clear();
//}

//void MTerminalView::SelectCharAttr(char inChar)
//{
//    mEscape.clear();
//    if (inChar == 'q')
//    {
//        switch (mArgs[0])
//        {
//            case 0:		
//            case 2:		mCurStyle &= ~kUnerasable; break;
//            case 1:		mCurStyle |= kUnerasable; break;
//        }
//    }
//}

//void MTerminalView::EscapeDouble(char inDoubleMode)
//{
//    mEscape.clear();

//    switch (inDoubleMode)
//    {
//        case '3':		mBuffer->SetLineDoubleHeight(mCursorY, true); break;
//        case '4':		mBuffer->SetLineDoubleHeight(mCursorY, false); break;
//        case '5':		mBuffer->SetLineSingleWidth(mCursorY); break;
//        case '6':		mBuffer->SetLineDoubleWidth(mCursorY); break;
//        case '8':		mBuffer->FillWithE(); break;
//    }
//}

//void MTerminalView::CommitDCS()
//{
//    foreach (auto k, mNewDCS->key)
//    {
//        string s;
//        for (uint32 i = 0; i < k.second.length(); i += 2)
//        {
//            char c1 = tolower(k.second[i]);
//            char c2 = 0;
//            if (i < k.second.length()) c2 = tolower(k.second[i + 1]);
			
//            if (c1 >= '0' and c1 <= '9') c1 -= '0';
//            else c1 -= 'a' + 10;

//            if (c2 >= '0' and c2 <= '9') c2 -= '0';
//            else c2 -= 'a' + 10;
			
//            s += char((c1 << 4) | c2);
//        }
//        mNewDCS->key[k.first] = s;
//    }

//    if (mNewDCS->clear or mDCS == nullptr)
//    {
//        delete mDCS;
//        mDCS = mNewDCS;
//    }
//    else
//    {
//        foreach (auto k, mNewDCS->key)
//            mDCS->key[k.first] = k.second;
//        delete mNewDCS;
//    }
	
//    mNewDCS = nullptr;
//}

//void MTerminalView::EscapeDCS(char inChar, uint32 inKey)
//{
//    if (mNewDCS == nullptr)
//    {
//        mNewDCS = new MDCS;
//        mNewDCS->clear = mNewDCS->locked = true;
//    }
	
//    if (inChar == '\220')
//    {
//        CommitDCS();
//        return;
//    }
	
//    switch (mState)
//    {
//        case 0:
//            if (inChar == '0')
//                mState = 1;
//            else if (inChar == '1')
//            {
//                mNewDCS->clear = false;
//                mState = 1;
//            }
//            else if (inChar == ';')
//                mState = 2;
//            else
//                mEscape.clear();
//            break;
		
//        case 1:
//            if (inChar == ';')
//                mState = 2;
//            else
//                mEscape.clear();
//            break;

//        case 2:
//            if (inChar == '0')
//                mState = 3;
//            else if (inChar == '1')
//            {
//                mNewDCS->locked = false;
//                mState = 3;
//            }
//            else if (inChar == '|')
//                mState = 4;
//            else
//                mEscape.clear();
//            break;
		
//        case 3:
//            if (inChar == '|')
//                mState = 4;
//            else
//                mEscape.clear();
//            break;

//        case 4:
//            if (inChar >= '0' and inChar <= '9')
//            {
//                inKey = inKey * 10 + (inChar - '0');
//                mEscape = boost::bind(&MTerminalView::EscapeDCS, this, _1, inKey);
//            }
//            else if (inChar == '/')
//                mState = 5;
//            else
//                mEscape.clear();
//            break;
		
//        case 5:
//            if (inChar == ';')
//                mEscape = boost::bind(&MTerminalView::EscapeDCS, this, _1, 0);
//            else if (isxdigit(inChar))
//                mNewDCS->key[inKey] += inChar;
//            else
//                mEscape.clear();
//            break;
//    }
	
//    if (not mEscape)
//    {
//        delete mNewDCS;
//        mNewDCS = nullptr;
//    }
//}

//void MTerminalView::EscapeOSC(
//    char				inChar)
//{
//    if (inChar == '\007' or inChar == '\234')
//    {
//        // done
//        mEscape.clear();
		
//        switch (mArgs[0])
//        {
//            case 0:
//            case 1:
//            case 2:		GetWindow()->SetTitle(mArgString); break;
//            default:	break;
//        }
//    }
//    else
//    {
//        switch (mState)
//        {
//            case 0:		// start, expect a number, or ';'
//                mArgs.clear();
//                mArgs.push_back(0);
//                mArgString.clear();
	
//                if (inChar == ';')
//                {
//                    mArgs.push_back(0);
//                    mState = 2;
//                }
//                else if (inChar >= '0' and inChar <= '9')
//                {
//                    mArgs[0] = inChar - '0';
//                    mState = 1;
//                }
//                else
//                {
//                    mArgString += inChar;
//                    mState = 2;
//                }
//                break;
			
//            case 1:
//                if (inChar >= '0' and inChar <= '9')
//                    mArgs.back() = mArgs.back() * 10 + (inChar - '0');
//                else if (inChar == ';')
//                    mArgs.push_back(0);
//                else	// error
//                {
//                    mArgString += inChar;
//                    mState = 2;
//                }
//                break;
			
//            case 2:
//                mArgString += inChar;
//                break;
//        }
//    }
//}

//void MTerminalView::RestoreCursor(void)
//{
//    if (mSavedCursor)
//    {
//        mCharSetG[0] = mSaveCharSetG[0];
//        mCharSetG[1] = mSaveCharSetG[1];
//        mCSGL = mSaveCSGL;
//        mCSGR = mSaveCSGR;
//        mCurStyle = mSaveCurStyle;
//        mCursorX = mSaveCursorX;
//        mCursorY = mSaveCursorY;
//        mDECOM = mSaveOrigin;
////		= mSaveSelectiveErase;
		
//        mSavedCursor = false;
//    }
//    else
//        MoveCursorTo(0, 0);
//}

//void MTerminalView::SaveCursor(void)
//{
//    mSaveCharSetG[0] = mCharSetG[0];
//    mSaveCharSetG[1] = mCharSetG[1];
//    mSaveCSGL = mCSGL;
//    mSaveCSGR = mCSGR;
//    mSaveCurStyle = mCurStyle;
//    mSaveCursorX = mCursorX;
//    mSaveCursorY = mCursorY;
//    mSaveOrigin = mDECOM;
////	mSaveSelectiveErase = 
//    mSavedCursor = true;
//}

//void MTerminalView::Beep()
//{
//    if (abs(GetLocalTime() - mLastBeep) > 0.5)
//    {
//        PlaySound("warning");
//        mLastBeep = GetLocalTime();
//    }
//}

//void MTerminalView::SetResetMode(
//    uint32				inMode,
//    bool				inANSI,
//    bool				inSet)
//{
//    if (inANSI)
//    {
//        switch (inMode)
//        {
//            case 2:		mKAM = inSet; break;
//            case 4:		mIRM = inSet; break;
//            case 12:	mSRM = inSet; break;
//            case 20:	mLNM = inSet; break;
//        }
//    }
//    else
//    {
//        switch (inMode)
//        {
//            case 1:		mDECCKM = inSet; break;
//            case 2:		mDECANM = inSet; break;
//            case 3:		ResizeTerminal(inSet ? 132 : 80); break;
//            case 4:		mDECSCLM = inSet; break;
//            case 5:		mDECSCNM = inSet; break;
//            case 6:		mDECOM = inSet; if (inSet) MoveCursorTo(0, 0); break;
//            case 7:		mDECAWM = inSet; break;
//            case 8:		mDECARM = inSet; break;
//            case 18:	mDECPFF = inSet; break;
//            case 19:	mDECPEX = inSet; break;
//            case 25:	mDECTCEM = inSet; break;
//            case 42:	mDECNRCM = inSet; break;
//            default:	PRINT(("Ignored %s of option %d", inSet ? "set" : "reset", inMode)); break;
//        }
//    }
//}


}
