using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Microsoft.Phone.Controls;
using System.Windows.Media.Imaging;
using System.Security.Cryptography;

namespace Salt
{
	public partial class TerminalPage : PhoneApplicationPage
	{
		private Terminal term;
		private SSH.Connection connection;

		public TerminalPage()
		{
			InitializeComponent();

			this.term = new Terminal(this.terminal);

			connection = new SSH.Connection();
			connection.Connect("mrs.cmbi.ru.nl", 2022);
			//connection.Connect("mrs.cmbi.ru.nl", 22);
			//connection.Connect("qnap", 2022);
		}
	}
}