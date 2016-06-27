using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace monitor_customhmd
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            //add relative path so openvr_api dll can be loaded
            var path = Environment.GetEnvironmentVariable("PATH");            
            path += ";" + Path.GetDirectoryName(Environment.CurrentDirectory + @"\..\..\..\bin\win" + (Environment.Is64BitProcess ? "64" : "32") + @"\");
            Environment.SetEnvironmentVariable("PATH", path);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MonitorForm());
        }
    }
}
