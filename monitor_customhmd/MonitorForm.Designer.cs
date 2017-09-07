namespace monitor_customhmd
{
    partial class MonitorForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.tmrConsumer = new System.Windows.Forms.Timer(this.components);
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.lblSequence = new System.Windows.Forms.ToolStripStatusLabel();
            this.tabSources = new System.Windows.Forms.TabControl();
            this.tabDefault = new System.Windows.Forms.TabPage();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.rdDisabled = new System.Windows.Forms.RadioButton();
            this.rdPSVR = new System.Windows.Forms.RadioButton();
            this.rdCustomHmd = new System.Windows.Forms.RadioButton();
            this.rdOpenTrack = new System.Windows.Forms.RadioButton();
            this.txtOpenTrackPort = new System.Windows.Forms.TextBox();
            this.btnReloadDistortion = new System.Windows.Forms.Button();
            this.btnResetRotation = new System.Windows.Forms.Button();
            this.chkWatchDog = new System.Windows.Forms.CheckBox();
            this.trayMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.mnuDebug = new System.Windows.Forms.ToolStripMenuItem();
            this.mnuExit = new System.Windows.Forms.ToolStripMenuItem();
            this.tabSources.SuspendLayout();
            this.tabDefault.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.trayMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // tmrConsumer
            // 
            this.tmrConsumer.Interval = 1;
            this.tmrConsumer.Tick += new System.EventHandler(this.tmrConsumer_Tick);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Location = new System.Drawing.Point(0, 358);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(506, 22);
            this.statusStrip1.TabIndex = 1;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // lblSequence
            // 
            this.lblSequence.AutoSize = false;
            this.lblSequence.Name = "lblSequence";
            this.lblSequence.Size = new System.Drawing.Size(50, 17);
            // 
            // tabSources
            // 
            this.tabSources.Controls.Add(this.tabDefault);
            this.tabSources.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabSources.Location = new System.Drawing.Point(0, 0);
            this.tabSources.Name = "tabSources";
            this.tabSources.SelectedIndex = 0;
            this.tabSources.Size = new System.Drawing.Size(506, 358);
            this.tabSources.TabIndex = 25;
            // 
            // tabDefault
            // 
            this.tabDefault.Controls.Add(this.groupBox1);
            this.tabDefault.Controls.Add(this.btnReloadDistortion);
            this.tabDefault.Controls.Add(this.btnResetRotation);
            this.tabDefault.Controls.Add(this.chkWatchDog);
            this.tabDefault.Location = new System.Drawing.Point(4, 22);
            this.tabDefault.Name = "tabDefault";
            this.tabDefault.Padding = new System.Windows.Forms.Padding(3);
            this.tabDefault.Size = new System.Drawing.Size(498, 332);
            this.tabDefault.TabIndex = 0;
            this.tabDefault.Text = "General";
            this.tabDefault.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.rdDisabled);
            this.groupBox1.Controls.Add(this.rdPSVR);
            this.groupBox1.Controls.Add(this.rdCustomHmd);
            this.groupBox1.Controls.Add(this.rdOpenTrack);
            this.groupBox1.Controls.Add(this.txtOpenTrackPort);
            this.groupBox1.Location = new System.Drawing.Point(8, 75);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(413, 158);
            this.groupBox1.TabIndex = 12;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Tracking Input";
            // 
            // rdDisabled
            // 
            this.rdDisabled.AutoSize = true;
            this.rdDisabled.Checked = true;
            this.rdDisabled.Location = new System.Drawing.Point(6, 19);
            this.rdDisabled.Name = "rdDisabled";
            this.rdDisabled.Size = new System.Drawing.Size(66, 17);
            this.rdDisabled.TabIndex = 14;
            this.rdDisabled.TabStop = true;
            this.rdDisabled.Text = "Disabled";
            this.rdDisabled.UseVisualStyleBackColor = true;
            this.rdDisabled.CheckedChanged += new System.EventHandler(this.rdOpenTrack_CheckedChanged);
            // 
            // rdPSVR
            // 
            this.rdPSVR.AutoSize = true;
            this.rdPSVR.Location = new System.Drawing.Point(6, 88);
            this.rdPSVR.Name = "rdPSVR";
            this.rdPSVR.Size = new System.Drawing.Size(54, 17);
            this.rdPSVR.TabIndex = 13;
            this.rdPSVR.Text = "PSVR";
            this.rdPSVR.UseVisualStyleBackColor = true;
            this.rdPSVR.CheckedChanged += new System.EventHandler(this.rdOpenTrack_CheckedChanged);
            // 
            // rdCustomHmd
            // 
            this.rdCustomHmd.AutoSize = true;
            this.rdCustomHmd.Location = new System.Drawing.Point(6, 42);
            this.rdCustomHmd.Name = "rdCustomHmd";
            this.rdCustomHmd.Size = new System.Drawing.Size(88, 17);
            this.rdCustomHmd.TabIndex = 12;
            this.rdCustomHmd.Text = "Custom HMD";
            this.rdCustomHmd.UseVisualStyleBackColor = true;
            this.rdCustomHmd.CheckedChanged += new System.EventHandler(this.rdOpenTrack_CheckedChanged);
            // 
            // rdOpenTrack
            // 
            this.rdOpenTrack.AutoSize = true;
            this.rdOpenTrack.Location = new System.Drawing.Point(6, 65);
            this.rdOpenTrack.Name = "rdOpenTrack";
            this.rdOpenTrack.Size = new System.Drawing.Size(115, 17);
            this.rdOpenTrack.TabIndex = 0;
            this.rdOpenTrack.Text = "OpenTrack on port";
            this.rdOpenTrack.UseVisualStyleBackColor = true;
            this.rdOpenTrack.CheckedChanged += new System.EventHandler(this.rdOpenTrack_CheckedChanged);
            // 
            // txtOpenTrackPort
            // 
            this.txtOpenTrackPort.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.txtOpenTrackPort.Location = new System.Drawing.Point(127, 65);
            this.txtOpenTrackPort.Name = "txtOpenTrackPort";
            this.txtOpenTrackPort.Size = new System.Drawing.Size(37, 20);
            this.txtOpenTrackPort.TabIndex = 11;
            this.txtOpenTrackPort.Text = "4242";
            this.txtOpenTrackPort.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // btnReloadDistortion
            // 
            this.btnReloadDistortion.Location = new System.Drawing.Point(128, 29);
            this.btnReloadDistortion.Name = "btnReloadDistortion";
            this.btnReloadDistortion.Size = new System.Drawing.Size(114, 40);
            this.btnReloadDistortion.TabIndex = 9;
            this.btnReloadDistortion.Text = "Reload Distortion Map";
            this.btnReloadDistortion.UseVisualStyleBackColor = true;
            this.btnReloadDistortion.Click += new System.EventHandler(this.btnResetDistortion_Click);
            // 
            // btnResetRotation
            // 
            this.btnResetRotation.Location = new System.Drawing.Point(8, 29);
            this.btnResetRotation.Name = "btnResetRotation";
            this.btnResetRotation.Size = new System.Drawing.Size(114, 40);
            this.btnResetRotation.TabIndex = 6;
            this.btnResetRotation.Text = "Send Identity Rotation";
            this.btnResetRotation.UseVisualStyleBackColor = true;
            this.btnResetRotation.Click += new System.EventHandler(this.btnResetRotation_Click);
            // 
            // chkWatchDog
            // 
            this.chkWatchDog.AutoSize = true;
            this.chkWatchDog.Location = new System.Drawing.Point(8, 6);
            this.chkWatchDog.Name = "chkWatchDog";
            this.chkWatchDog.Size = new System.Drawing.Size(114, 17);
            this.chkWatchDog.TabIndex = 0;
            this.chkWatchDog.Text = "Enable WatchDog";
            this.chkWatchDog.UseVisualStyleBackColor = true;
            this.chkWatchDog.CheckedChanged += new System.EventHandler(this.chkWatchDog_CheckedChanged);
            // 
            // trayMenu
            // 
            this.trayMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuDebug,
            this.mnuExit});
            this.trayMenu.Name = "trayMenu";
            this.trayMenu.Size = new System.Drawing.Size(137, 48);
            // 
            // mnuDebug
            // 
            this.mnuDebug.Name = "mnuDebug";
            this.mnuDebug.Size = new System.Drawing.Size(136, 22);
            this.mnuDebug.Text = "Debug Data";
            this.mnuDebug.Click += new System.EventHandler(this.mnuDebug_Click);
            // 
            // mnuExit
            // 
            this.mnuExit.Name = "mnuExit";
            this.mnuExit.Size = new System.Drawing.Size(136, 22);
            this.mnuExit.Text = "Exit";
            this.mnuExit.Click += new System.EventHandler(this.mnuExit_Click);
            // 
            // MonitorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(506, 380);
            this.Controls.Add(this.tabSources);
            this.Controls.Add(this.statusStrip1);
            this.Name = "MonitorForm";
            this.Text = "Custom HMD Monitor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MonitorForm_FormClosing);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MonitorForm_FormClosed);
            this.Load += new System.EventHandler(this.MonitorForm_Load);
            this.Resize += new System.EventHandler(this.MonitorForm_Resize);
            this.tabSources.ResumeLayout(false);
            this.tabDefault.ResumeLayout(false);
            this.tabDefault.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.trayMenu.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Timer tmrConsumer;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel lblSequence;
        private System.Windows.Forms.TabControl tabSources;
        private System.Windows.Forms.ContextMenuStrip trayMenu;
        private System.Windows.Forms.ToolStripMenuItem mnuDebug;
        private System.Windows.Forms.ToolStripMenuItem mnuExit;
        private System.Windows.Forms.TabPage tabDefault;
        private System.Windows.Forms.CheckBox chkWatchDog;
        private System.Windows.Forms.Button btnResetRotation;
        private System.Windows.Forms.Button btnReloadDistortion;
        private System.Windows.Forms.TextBox txtOpenTrackPort;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton rdCustomHmd;
        private System.Windows.Forms.RadioButton rdOpenTrack;
        private System.Windows.Forms.RadioButton rdPSVR;
        private System.Windows.Forms.RadioButton rdDisabled;
    }
}

