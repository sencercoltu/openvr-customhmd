using System;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using System.Reflection;

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
            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;

            //add relative path so openvr_api dll can be loaded
            var path = Environment.GetEnvironmentVariable("PATH");            
            path += ";" + Path.GetDirectoryName(Environment.CurrentDirectory + @"\..\..\..\bin\win" + (Environment.Is64BitProcess ? "64" : "32") + @"\");
            Environment.SetEnvironmentVariable("PATH", path);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MonitorForm());

            
        }


        private static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {            
            var assemblyName = args.Name.Split(',').First();            
            var resourceName = Assembly.GetExecutingAssembly().GetManifestResourceNames().FirstOrDefault(r => r.EndsWith((assemblyName + ".dll")));
            if (string.IsNullOrEmpty(resourceName))
                return null;
            using (var s = Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
            {
                if (s == null)
                {                    
                    return null;
                }
                var assBytes = new byte[s.Length];
                s.Read(assBytes, 0, assBytes.Length);
                return Assembly.Load(assBytes);
            }
        }
    }
}
