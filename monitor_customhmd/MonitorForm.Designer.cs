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
            this.btnResetRotation = new System.Windows.Forms.Button();
            this.chkDiff = new System.Windows.Forms.CheckBox();
            this.chkPreview = new System.Windows.Forms.CheckBox();
            this.pbRight = new System.Windows.Forms.PictureBox();
            this.pbLeft = new System.Windows.Forms.PictureBox();
            this.chkWatchDog = new System.Windows.Forms.CheckBox();
            this.trayMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.mnuDebug = new System.Windows.Forms.ToolStripMenuItem();
            this.mnuExit = new System.Windows.Forms.ToolStripMenuItem();
            this.tabSources.SuspendLayout();
            this.tabDefault.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbRight)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbLeft)).BeginInit();
            this.trayMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // tmrConsumer
            // 
            this.tmrConsumer.Enabled = true;
            this.tmrConsumer.Interval = 1;
            this.tmrConsumer.Tick += new System.EventHandler(this.tmrConsumer_Tick);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Location = new System.Drawing.Point(0, 300);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(456, 22);
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
            this.tabSources.Size = new System.Drawing.Size(456, 300);
            this.tabSources.TabIndex = 25;
            // 
            // tabDefault
            // 
            this.tabDefault.Controls.Add(this.btnResetRotation);
            this.tabDefault.Controls.Add(this.chkDiff);
            this.tabDefault.Controls.Add(this.chkPreview);
            this.tabDefault.Controls.Add(this.pbRight);
            this.tabDefault.Controls.Add(this.pbLeft);
            this.tabDefault.Controls.Add(this.chkWatchDog);
            this.tabDefault.Location = new System.Drawing.Point(4, 22);
            this.tabDefault.Name = "tabDefault";
            this.tabDefault.Padding = new System.Windows.Forms.Padding(3);
            this.tabDefault.Size = new System.Drawing.Size(448, 274);
            this.tabDefault.TabIndex = 0;
            this.tabDefault.Text = "General";
            this.tabDefault.UseVisualStyleBackColor = true;
            // 
            // btnResetRotation
            // 
            this.btnResetRotation.Location = new System.Drawing.Point(140, 6);
            this.btnResetRotation.Name = "btnResetRotation";
            this.btnResetRotation.Size = new System.Drawing.Size(75, 40);
            this.btnResetRotation.TabIndex = 6;
            this.btnResetRotation.Text = "Reset Rotation";
            this.btnResetRotation.UseVisualStyleBackColor = true;
            this.btnResetRotation.Click += new System.EventHandler(this.btnResetRotation_Click);
            // 
            // chkDiff
            // 
            this.chkDiff.AutoSize = true;
            this.chkDiff.Location = new System.Drawing.Point(8, 29);
            this.chkDiff.Name = "chkDiff";
            this.chkDiff.Size = new System.Drawing.Size(75, 17);
            this.chkDiff.TabIndex = 5;
            this.chkDiff.Text = "Difference";
            this.chkDiff.UseVisualStyleBackColor = true;
            // 
            // chkPreview
            // 
            this.chkPreview.AutoSize = true;
            this.chkPreview.Location = new System.Drawing.Point(8, 6);
            this.chkPreview.Name = "chkPreview";
            this.chkPreview.Size = new System.Drawing.Size(104, 17);
            this.chkPreview.TabIndex = 4;
            this.chkPreview.Text = "Preview Remote";
            this.chkPreview.UseVisualStyleBackColor = true;
            // 
            // pbRight
            // 
            this.pbRight.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.pbRight.Location = new System.Drawing.Point(188, 52);
            this.pbRight.Name = "pbRight";
            this.pbRight.Size = new System.Drawing.Size(174, 193);
            this.pbRight.TabIndex = 3;
            this.pbRight.TabStop = false;
            // 
            // pbLeft
            // 
            this.pbLeft.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.pbLeft.Location = new System.Drawing.Point(8, 52);
            this.pbLeft.Name = "pbLeft";
            this.pbLeft.Size = new System.Drawing.Size(174, 193);
            this.pbLeft.TabIndex = 2;
            this.pbLeft.TabStop = false;
            // 
            // chkWatchDog
            // 
            this.chkWatchDog.AutoSize = true;
            this.chkWatchDog.Location = new System.Drawing.Point(248, 6);
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
            this.ClientSize = new System.Drawing.Size(456, 322);
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
            ((System.ComponentModel.ISupportInitialize)(this.pbRight)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbLeft)).EndInit();
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
        private System.Windows.Forms.PictureBox pbLeft;
        private System.Windows.Forms.PictureBox pbRight;
        private System.Windows.Forms.CheckBox chkPreview;
        private System.Windows.Forms.CheckBox chkDiff;
        private System.Windows.Forms.Button btnResetRotation;
    }
}

