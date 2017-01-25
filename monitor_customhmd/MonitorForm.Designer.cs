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
            this.button1 = new System.Windows.Forms.Button();
            this.tmrConsumer = new System.Windows.Forms.Timer(this.components);
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.lblSequence = new System.Windows.Forms.ToolStripStatusLabel();
            this.dgData = new System.Windows.Forms.DataGridView();
            this.Type = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Sequence = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.W = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.X = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Y = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Z = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.button2 = new System.Windows.Forms.Button();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.button3 = new System.Windows.Forms.Button();
            this.pb = new System.Windows.Forms.PictureBox();
            this.chkKalman = new System.Windows.Forms.CheckBox();
            this.numProcNoise = new System.Windows.Forms.NumericUpDown();
            this.numSensNoise = new System.Windows.Forms.NumericUpDown();
            this.btnScaleUp = new System.Windows.Forms.Button();
            this.btnScaleDown = new System.Windows.Forms.Button();
            this.btnResetScale = new System.Windows.Forms.Button();
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
            this.statusStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgData)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pb)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numProcNoise)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numSensNoise)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.axisscale)).BeginInit();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(12, 12);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 0;
            this.button1.Text = "button1";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // tmrConsumer
            // 
            this.tmrConsumer.Enabled = true;
            this.tmrConsumer.Interval = 1;
            this.tmrConsumer.Tick += new System.EventHandler(this.tmrConsumer_Tick);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.lblSequence});
            this.statusStrip1.Location = new System.Drawing.Point(0, 411);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(1288, 22);
            this.statusStrip1.TabIndex = 1;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // lblSequence
            // 
            this.lblSequence.AutoSize = false;
            this.lblSequence.Name = "lblSequence";
            this.lblSequence.Size = new System.Drawing.Size(50, 17);
            // 
            // dgData
            // 
            this.dgData.AllowUserToAddRows = false;
            this.dgData.AllowUserToDeleteRows = false;
            this.dgData.AllowUserToResizeRows = false;
            this.dgData.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgData.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Type,
            this.Sequence,
            this.W,
            this.X,
            this.Y,
            this.Z});
            this.dgData.EditMode = System.Windows.Forms.DataGridViewEditMode.EditProgrammatically;
            this.dgData.Location = new System.Drawing.Point(13, 52);
            this.dgData.MultiSelect = false;
            this.dgData.Name = "dgData";
            this.dgData.RowHeadersVisible = false;
            this.dgData.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dgData.Size = new System.Drawing.Size(605, 342);
            this.dgData.TabIndex = 2;
            // 
            // Type
            // 
            this.Type.HeaderText = "Type";
            this.Type.Name = "Type";
            this.Type.ReadOnly = true;
            // 
            // Sequence
            // 
            this.Sequence.HeaderText = "Sequence";
            this.Sequence.Name = "Sequence";
            this.Sequence.ReadOnly = true;
            // 
            // W
            // 
            this.W.HeaderText = "W";
            this.W.Name = "W";
            this.W.ReadOnly = true;
            // 
            // X
            // 
            this.X.HeaderText = "X";
            this.X.Name = "X";
            this.X.ReadOnly = true;
            // 
            // Y
            // 
            this.Y.HeaderText = "Y";
            this.Y.Name = "Y";
            this.Y.ReadOnly = true;
            // 
            // Z
            // 
            this.Z.HeaderText = "Z";
            this.Z.Name = "Z";
            this.Z.ReadOnly = true;
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(566, 12);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 3;
            this.button2.Text = "Debug";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // comboBox1
            // 
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(439, 13);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(121, 21);
            this.comboBox1.TabIndex = 6;
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(634, 251);
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
            this.pb.Location = new System.Drawing.Point(728, 52);
            this.pb.Name = "pb";
            this.pb.Size = new System.Drawing.Size(488, 342);
            this.pb.TabIndex = 8;
            this.pb.TabStop = false;
            this.pb.Paint += new System.Windows.Forms.PaintEventHandler(this.pb_Paint);
            // 
            // chkKalman
            // 
            this.chkKalman.AutoSize = true;
            this.chkKalman.Location = new System.Drawing.Point(656, 13);
            this.chkKalman.Name = "chkKalman";
            this.chkKalman.Size = new System.Drawing.Size(78, 17);
            this.chkKalman.TabIndex = 9;
            this.chkKalman.Text = "Kalman On";
            this.chkKalman.UseVisualStyleBackColor = true;
            this.chkKalman.CheckedChanged += new System.EventHandler(this.chkKalman_CheckedChanged);
            // 
            // numProcNoise
            // 
            this.numProcNoise.DecimalPlaces = 1;
            this.numProcNoise.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numProcNoise.Location = new System.Drawing.Point(754, 12);
            this.numProcNoise.Name = "numProcNoise";
            this.numProcNoise.Size = new System.Drawing.Size(120, 20);
            this.numProcNoise.TabIndex = 10;
            this.numProcNoise.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numProcNoise.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numProcNoise.ValueChanged += new System.EventHandler(this.numProcNoise_ValueChanged);
            // 
            // numSensNoise
            // 
            this.numSensNoise.DecimalPlaces = 1;
            this.numSensNoise.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numSensNoise.Location = new System.Drawing.Point(891, 12);
            this.numSensNoise.Name = "numSensNoise";
            this.numSensNoise.Size = new System.Drawing.Size(120, 20);
            this.numSensNoise.TabIndex = 11;
            this.numSensNoise.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numSensNoise.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numSensNoise.ValueChanged += new System.EventHandler(this.numSensNoise_ValueChanged);
            // 
            // btnScaleUp
            // 
            this.btnScaleUp.Location = new System.Drawing.Point(656, 299);
            this.btnScaleUp.Name = "btnScaleUp";
            this.btnScaleUp.Size = new System.Drawing.Size(24, 23);
            this.btnScaleUp.TabIndex = 12;
            this.btnScaleUp.Text = "+";
            this.btnScaleUp.UseVisualStyleBackColor = true;
            this.btnScaleUp.Click += new System.EventHandler(this.btnScaleUp_Click);
            // 
            // btnScaleDown
            // 
            this.btnScaleDown.Location = new System.Drawing.Point(656, 328);
            this.btnScaleDown.Name = "btnScaleDown";
            this.btnScaleDown.Size = new System.Drawing.Size(24, 23);
            this.btnScaleDown.TabIndex = 13;
            this.btnScaleDown.Text = "-";
            this.btnScaleDown.UseVisualStyleBackColor = true;
            this.btnScaleDown.Click += new System.EventHandler(this.btnScaleDown_Click);
            // 
            // btnResetScale
            // 
            this.btnResetScale.Location = new System.Drawing.Point(656, 357);
            this.btnResetScale.Name = "btnResetScale";
            this.btnResetScale.Size = new System.Drawing.Size(24, 23);
            this.btnResetScale.TabIndex = 14;
            this.btnResetScale.Text = "R";
            this.btnResetScale.UseVisualStyleBackColor = true;
            this.btnResetScale.Click += new System.EventHandler(this.btnResetScale_Click);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(625, 61);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(84, 24);
            this.label1.TabIndex = 15;
            this.label1.Text = "X Offset";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // txtPosX
            // 
            this.txtPosX.Location = new System.Drawing.Point(622, 84);
            this.txtPosX.Name = "txtPosX";
            this.txtPosX.Size = new System.Drawing.Size(49, 20);
            this.txtPosX.TabIndex = 16;
            this.txtPosX.Text = "1991";
            this.txtPosX.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // txtPosY
            // 
            this.txtPosY.Location = new System.Drawing.Point(622, 144);
            this.txtPosY.Name = "txtPosY";
            this.txtPosY.Size = new System.Drawing.Size(49, 20);
            this.txtPosY.TabIndex = 18;
            this.txtPosY.Text = "2033";
            this.txtPosY.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(625, 121);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(84, 24);
            this.label2.TabIndex = 17;
            this.label2.Text = "Y Offset";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // txtPosZ
            // 
            this.txtPosZ.Location = new System.Drawing.Point(622, 213);
            this.txtPosZ.Name = "txtPosZ";
            this.txtPosZ.Size = new System.Drawing.Size(49, 20);
            this.txtPosZ.TabIndex = 20;
            this.txtPosZ.Text = "1864";
            this.txtPosZ.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(625, 190);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(84, 24);
            this.label3.TabIndex = 19;
            this.label3.Text = "Z Offset";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // txtNegX
            // 
            this.txtNegX.Location = new System.Drawing.Point(677, 84);
            this.txtNegX.Name = "txtNegX";
            this.txtNegX.Size = new System.Drawing.Size(49, 20);
            this.txtNegX.TabIndex = 21;
            this.txtNegX.Text = "2094";
            this.txtNegX.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // txtNegZ
            // 
            this.txtNegZ.Location = new System.Drawing.Point(677, 213);
            this.txtNegZ.Name = "txtNegZ";
            this.txtNegZ.Size = new System.Drawing.Size(49, 20);
            this.txtNegZ.TabIndex = 23;
            this.txtNegZ.Text = "2194";
            this.txtNegZ.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // txtNegY
            // 
            this.txtNegY.Location = new System.Drawing.Point(677, 144);
            this.txtNegY.Name = "txtNegY";
            this.txtNegY.Size = new System.Drawing.Size(49, 20);
            this.txtNegY.TabIndex = 22;
            this.txtNegY.Text = "2084";
            this.txtNegY.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // axisscale
            // 
            this.axisscale.LargeChange = 100;
            this.axisscale.Location = new System.Drawing.Point(1222, 52);
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
            // MonitorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1288, 433);
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
            this.Controls.Add(this.btnResetScale);
            this.Controls.Add(this.btnScaleDown);
            this.Controls.Add(this.btnScaleUp);
            this.Controls.Add(this.numSensNoise);
            this.Controls.Add(this.numProcNoise);
            this.Controls.Add(this.chkKalman);
            this.Controls.Add(this.pb);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.comboBox1);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.dgData);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.button1);
            this.Name = "MonitorForm";
            this.Text = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MonitorForm_FormClosing);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MonitorForm_FormClosed);
            this.Load += new System.EventHandler(this.MonitorForm_Load);
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgData)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pb)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numProcNoise)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numSensNoise)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.axisscale)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Timer tmrConsumer;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel lblSequence;
        private System.Windows.Forms.DataGridView dgData;
        private System.Windows.Forms.DataGridViewTextBoxColumn Type;
        private System.Windows.Forms.DataGridViewTextBoxColumn Sequence;
        private System.Windows.Forms.DataGridViewTextBoxColumn W;
        private System.Windows.Forms.DataGridViewTextBoxColumn X;
        private System.Windows.Forms.DataGridViewTextBoxColumn Y;
        private System.Windows.Forms.DataGridViewTextBoxColumn Z;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.PictureBox pb;
        private System.Windows.Forms.CheckBox chkKalman;
        private System.Windows.Forms.NumericUpDown numProcNoise;
        private System.Windows.Forms.NumericUpDown numSensNoise;
        private System.Windows.Forms.Button btnScaleUp;
        private System.Windows.Forms.Button btnScaleDown;
        private System.Windows.Forms.Button btnResetScale;
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
    }
}

