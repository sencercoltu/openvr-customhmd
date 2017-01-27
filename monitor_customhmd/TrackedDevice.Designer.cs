namespace monitor_customhmd
{
    partial class TrackedDevice
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.cmbSwitchState = new System.Windows.Forms.ComboBox();
            this.lblIncoming = new System.Windows.Forms.Label();
            this.lblOutgoing = new System.Windows.Forms.Label();
            this.lblLastSequence = new System.Windows.Forms.Label();
            this.lblMissed = new System.Windows.Forms.Label();
            this.lblQuat = new System.Windows.Forms.Label();
            this.lblTriggers = new System.Windows.Forms.Label();
            this.lblVibrate = new System.Windows.Forms.Label();
            this.lblAccel = new System.Windows.Forms.Label();
            this.lblGyro = new System.Windows.Forms.Label();
            this.lblMag = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // cmbSwitchState
            // 
            this.cmbSwitchState.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.cmbSwitchState.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbSwitchState.FormattingEnabled = true;
            this.cmbSwitchState.Location = new System.Drawing.Point(540, 3);
            this.cmbSwitchState.Name = "cmbSwitchState";
            this.cmbSwitchState.Size = new System.Drawing.Size(121, 21);
            this.cmbSwitchState.TabIndex = 8;
            this.cmbSwitchState.SelectedIndexChanged += new System.EventHandler(this.cmbSwitchState_DropDownClosed);
            this.cmbSwitchState.DropDownClosed += new System.EventHandler(this.cmbSwitchState_DropDownClosed);
            // 
            // lblIncoming
            // 
            this.lblIncoming.AutoSize = true;
            this.lblIncoming.Location = new System.Drawing.Point(4, 3);
            this.lblIncoming.Name = "lblIncoming";
            this.lblIncoming.Size = new System.Drawing.Size(104, 13);
            this.lblIncoming.TabIndex = 10;
            this.lblIncoming.Text = "Incoming Packets: 0";
            // 
            // lblOutgoing
            // 
            this.lblOutgoing.AutoSize = true;
            this.lblOutgoing.Location = new System.Drawing.Point(4, 16);
            this.lblOutgoing.Name = "lblOutgoing";
            this.lblOutgoing.Size = new System.Drawing.Size(104, 13);
            this.lblOutgoing.TabIndex = 11;
            this.lblOutgoing.Text = "Outgoing Packets: 0";
            // 
            // lblLastSequence
            // 
            this.lblLastSequence.AutoSize = true;
            this.lblLastSequence.Location = new System.Drawing.Point(4, 29);
            this.lblLastSequence.Name = "lblLastSequence";
            this.lblLastSequence.Size = new System.Drawing.Size(91, 13);
            this.lblLastSequence.TabIndex = 12;
            this.lblLastSequence.Text = "Last Sequence: 0";
            // 
            // lblMissed
            // 
            this.lblMissed.AutoSize = true;
            this.lblMissed.Location = new System.Drawing.Point(4, 42);
            this.lblMissed.Name = "lblMissed";
            this.lblMissed.Size = new System.Drawing.Size(94, 13);
            this.lblMissed.TabIndex = 13;
            this.lblMissed.Text = "Missed Packets: 0";
            // 
            // lblQuat
            // 
            this.lblQuat.AutoSize = true;
            this.lblQuat.Location = new System.Drawing.Point(4, 94);
            this.lblQuat.Name = "lblQuat";
            this.lblQuat.Size = new System.Drawing.Size(73, 13);
            this.lblQuat.TabIndex = 17;
            this.lblQuat.Text = "Quaternions []";
            // 
            // lblTriggers
            // 
            this.lblTriggers.AutoSize = true;
            this.lblTriggers.Location = new System.Drawing.Point(4, 107);
            this.lblTriggers.Name = "lblTriggers";
            this.lblTriggers.Size = new System.Drawing.Size(54, 13);
            this.lblTriggers.TabIndex = 18;
            this.lblTriggers.Text = "Triggers []";
            // 
            // lblVibrate
            // 
            this.lblVibrate.AutoSize = true;
            this.lblVibrate.Location = new System.Drawing.Point(4, 120);
            this.lblVibrate.Name = "lblVibrate";
            this.lblVibrate.Size = new System.Drawing.Size(46, 13);
            this.lblVibrate.TabIndex = 19;
            this.lblVibrate.Text = "Vibrate: ";
            // 
            // lblAccel
            // 
            this.lblAccel.AutoSize = true;
            this.lblAccel.Location = new System.Drawing.Point(4, 168);
            this.lblAccel.Name = "lblAccel";
            this.lblAccel.Size = new System.Drawing.Size(43, 13);
            this.lblAccel.TabIndex = 20;
            this.lblAccel.Text = "Accel []";
            // 
            // lblGyro
            // 
            this.lblGyro.AutoSize = true;
            this.lblGyro.Location = new System.Drawing.Point(4, 181);
            this.lblGyro.Name = "lblGyro";
            this.lblGyro.Size = new System.Drawing.Size(38, 13);
            this.lblGyro.TabIndex = 21;
            this.lblGyro.Text = "Gyro []";
            // 
            // lblMag
            // 
            this.lblMag.AutoSize = true;
            this.lblMag.Location = new System.Drawing.Point(4, 194);
            this.lblMag.Name = "lblMag";
            this.lblMag.Size = new System.Drawing.Size(37, 13);
            this.lblMag.TabIndex = 22;
            this.lblMag.Text = "Mag []";
            // 
            // TrackedDevice
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.lblMag);
            this.Controls.Add(this.lblGyro);
            this.Controls.Add(this.lblAccel);
            this.Controls.Add(this.lblVibrate);
            this.Controls.Add(this.lblTriggers);
            this.Controls.Add(this.lblQuat);
            this.Controls.Add(this.lblMissed);
            this.Controls.Add(this.lblLastSequence);
            this.Controls.Add(this.lblOutgoing);
            this.Controls.Add(this.lblIncoming);
            this.Controls.Add(this.cmbSwitchState);
            this.Name = "TrackedDevice";
            this.Size = new System.Drawing.Size(664, 487);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbSwitchState;
        private System.Windows.Forms.Label lblIncoming;
        private System.Windows.Forms.Label lblOutgoing;
        private System.Windows.Forms.Label lblLastSequence;
        private System.Windows.Forms.Label lblMissed;
        private System.Windows.Forms.Label lblQuat;
        private System.Windows.Forms.Label lblTriggers;
        private System.Windows.Forms.Label lblVibrate;
        private System.Windows.Forms.Label lblAccel;
        private System.Windows.Forms.Label lblGyro;
        private System.Windows.Forms.Label lblMag;
    }
}
