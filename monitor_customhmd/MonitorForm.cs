using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Valve.VR;

namespace monitor_customhmd
{
    public partial class MonitorForm : Form
    {
        private CVRSystem _vrSystem;

        public MonitorForm()
        {
            InitializeComponent();
        }

        private void MonitorForm_Load(object sender, EventArgs e)
        {
            var err = EVRInitError.Driver_Failed;
            _vrSystem = OpenVR.Init(ref err, EVRApplicationType.VRApplication_Background);
        }
    }
}
