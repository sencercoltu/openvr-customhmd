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
            this.button2 = new System.Windows.Forms.Button();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.button3 = new System.Windows.Forms.Button();
            this.pb = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.txtPosX = new System.Windows.Forms.TextBox();
            this.txtPosY = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.txtPosZ = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtNegX = new System.Windows.Forms.TextBox();
            this.txtNegZ = new System.Windows.Forms.TextBox();
            this.txtNegY = new System.Windows.Forms.TextBox();
            this.axisscale = new System.Windows.Forms.TrackBar();
            this.tabSources = new System.Windows.Forms.TabControl();
            ((System.ComponentModel.ISupportInitialize)(this.pb)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.axisscale)).BeginInit();
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
            this.statusStrip1.Location = new System.Drawing.Point(0, 636);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(1384, 22);
            this.statusStrip1.TabIndex = 1;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // lblSequence
            // 
            this.lblSequence.AutoSize = false;
            this.lblSequence.Name = "lblSequence";
            this.lblSequence.Size = new System.Drawing.Size(50, 17);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(933, 14);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 3;
            this.button2.Text = "Switch State";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // comboBox1
            // 
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(806, 14);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(121, 21);
            this.comboBox1.TabIndex = 6;
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(1222, 605);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(75, 23);
            this.button3.TabIndex = 7;
            this.button3.Text = "Calib";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // pb
            // 
            this.pb.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pb.Location = new System.Drawing.Point(833, 52);
            this.pb.Name = "pb";
            this.pb.Size = new System.Drawing.Size(488, 342);
            this.pb.TabIndex = 8;
            this.pb.TabStop = false;
            this.pb.Paint += new System.Windows.Forms.PaintEventHandler(this.pb_Paint);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(1213, 415);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(84, 24);
            this.label1.TabIndex = 15;
            this.label1.Text = "X Offset";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // txtPosX
            // 
            this.txtPosX.Location = new System.Drawing.Point(1210, 438);
            this.txtPosX.Name = "txtPosX";
            this.txtPosX.Size = new System.Drawing.Size(49, 20);
            this.txtPosX.TabIndex = 16;
            this.txtPosX.Text = "1991";
            this.txtPosX.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // txtPosY
            // 
            this.txtPosY.Location = new System.Drawing.Point(1210, 498);
            this.txtPosY.Name = "txtPosY";
            this.txtPosY.Size = new System.Drawing.Size(49, 20);
            this.txtPosY.TabIndex = 18;
            this.txtPosY.Text = "2033";
            this.txtPosY.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(1213, 475);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(84, 24);
            this.label2.TabIndex = 17;
            this.label2.Text = "Y Offset";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // txtPosZ
            // 
            this.txtPosZ.Location = new System.Drawing.Point(1210, 567);
            this.txtPosZ.Name = "txtPosZ";
            this.txtPosZ.Size = new System.Drawing.Size(49, 20);
            this.txtPosZ.TabIndex = 20;
            this.txtPosZ.Text = "1864";
            this.txtPosZ.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(1213, 544);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(84, 24);
            this.label3.TabIndex = 19;
            this.label3.Text = "Z Offset";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // txtNegX
            // 
            this.txtNegX.Location = new System.Drawing.Point(1265, 438);
            this.txtNegX.Name = "txtNegX";
            this.txtNegX.Size = new System.Drawing.Size(49, 20);
            this.txtNegX.TabIndex = 21;
            this.txtNegX.Text = "2094";
            this.txtNegX.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // txtNegZ
            // 
            this.txtNegZ.Location = new System.Drawing.Point(1265, 567);
            this.txtNegZ.Name = "txtNegZ";
            this.txtNegZ.Size = new System.Drawing.Size(49, 20);
            this.txtNegZ.TabIndex = 23;
            this.txtNegZ.Text = "2194";
            this.txtNegZ.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // txtNegY
            // 
            this.txtNegY.Location = new System.Drawing.Point(1265, 498);
            this.txtNegY.Name = "txtNegY";
            this.txtNegY.Size = new System.Drawing.Size(49, 20);
            this.txtNegY.TabIndex = 22;
            this.txtNegY.Text = "2084";
            this.txtNegY.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // axisscale
            // 
            this.axisscale.LargeChange = 100;
            this.axisscale.Location = new System.Drawing.Point(1327, 48);
            this.axisscale.Maximum = 2000;
            this.axisscale.Minimum = 1;
            this.axisscale.Name = "axisscale";
            this.axisscale.Orientation = System.Windows.Forms.Orientation.Vertical;
            this.axisscale.Size = new System.Drawing.Size(45, 342);
            this.axisscale.TabIndex = 24;
            this.axisscale.TickFrequency = 100;
            this.axisscale.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.axisscale.Value = 1000;
            // 
            // tabSources
            // 
            this.tabSources.Location = new System.Drawing.Point(13, 13);
            this.tabSources.Name = "tabSources";
            this.tabSources.SelectedIndex = 0;
            this.tabSources.Size = new System.Drawing.Size(582, 381);
            this.tabSources.TabIndex = 25;
            // 
            // MonitorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1384, 658);
            this.Controls.Add(this.tabSources);
            this.Controls.Add(this.axisscale);
            this.Controls.Add(this.txtNegZ);
            this.Controls.Add(this.txtNegY);
            this.Controls.Add(this.txtNegX);
            this.Controls.Add(this.txtPosZ);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.txtPosY);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtPosX);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.pb);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.comboBox1);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.statusStrip1);
            this.Name = "MonitorForm";
            this.Text = "Custom HMD Monitor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MonitorForm_FormClosing);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MonitorForm_FormClosed);
            this.Load += new System.EventHandler(this.MonitorForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pb)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.axisscale)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Timer tmrConsumer;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel lblSequence;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.PictureBox pb;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtPosX;
        private System.Windows.Forms.TextBox txtPosY;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox txtPosZ;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtNegX;
        private System.Windows.Forms.TextBox txtNegZ;
        private System.Windows.Forms.TextBox txtNegY;
        private System.Windows.Forms.TrackBar axisscale;
        private System.Windows.Forms.TabControl tabSources;
    }
}

