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
            OpenVR.GetGenericInterface(OpenVR.IVRCompositor_Version, ref err);
            OpenVR.GetGenericInterface(OpenVR.IVROverlay_Version, ref err);
        }

        ulong ulMainHandle, ulThumbHandle;

        private void button1_Click(object sender, EventArgs e)
        {
            var overlay = OpenVR.Overlay;

            if (ulMainHandle != 0)
                overlay.DestroyOverlay(ulMainHandle);
            var err = overlay.CreateDashboardOverlay("TEST", "sencer", ref ulMainHandle, ref ulThumbHandle);
            err = overlay.SetOverlayFromFile(ulMainHandle, @"D:\Programs\Steam\steamapps\common\SteamVR\drivers\customhmd\bin\hl3.jpg");
            err = overlay.SetOverlayWidthInMeters(ulMainHandle, 2.5f);
            err = overlay.SetOverlayInputMethod(ulMainHandle, VROverlayInputMethod.Mouse);
            err = overlay.ShowOverlay(ulMainHandle);
        }

        private void MonitorForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (ulMainHandle != 0)
                OpenVR.Overlay.DestroyOverlay(ulMainHandle);
            OpenVR.Shutdown();
        }
    }
}
