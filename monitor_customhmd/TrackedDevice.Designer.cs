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
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle4 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle5 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle6 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle7 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle8 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle9 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle10 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle11 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle12 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle13 = new System.Windows.Forms.DataGridViewCellStyle();
            this.cmbSwitchState = new System.Windows.Forms.ComboBox();
            this.lblVibrate = new System.Windows.Forms.Label();
            this.cmbSensor = new System.Windows.Forms.ComboBox();
            this.btnSetCalib = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.btnGetCalib = new System.Windows.Forms.Button();
            this.minX = new System.Windows.Forms.TextBox();
            this.minY = new System.Windows.Forms.TextBox();
            this.minZ = new System.Windows.Forms.TextBox();
            this.maxZ = new System.Windows.Forms.TextBox();
            this.maxY = new System.Windows.Forms.TextBox();
            this.maxX = new System.Windows.Forms.TextBox();
            this.dgProperties = new System.Windows.Forms.DataGridView();
            this.Property = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Value = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.dgQuaternion = new System.Windows.Forms.DataGridView();
            this.dataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.label5 = new System.Windows.Forms.Label();
            this.dgButtons = new System.Windows.Forms.DataGridView();
            this.dataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.ButX = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.ButY = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.label6 = new System.Windows.Forms.Label();
            this.dgRaw = new System.Windows.Forms.DataGridView();
            this.dataGridViewTextBoxColumn5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Prop2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn6 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.ValueY = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.ValueZ = new System.Windows.Forms.DataGridViewTextBoxColumn();
            ((System.ComponentModel.ISupportInitialize)(this.dgProperties)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgQuaternion)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgButtons)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgRaw)).BeginInit();
            this.SuspendLayout();
            // 
            // cmbSwitchState
            // 
            this.cmbSwitchState.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbSwitchState.FormattingEnabled = true;
            this.cmbSwitchState.Location = new System.Drawing.Point(452, 0);
            this.cmbSwitchState.Name = "cmbSwitchState";
            this.cmbSwitchState.Size = new System.Drawing.Size(199, 21);
            this.cmbSwitchState.TabIndex = 8;
            this.cmbSwitchState.SelectedIndexChanged += new System.EventHandler(this.cmbSwitchState_DropDownClosed);
            this.cmbSwitchState.DropDownClosed += new System.EventHandler(this.cmbSwitchState_DropDownClosed);
            // 
            // lblVibrate
            // 
            this.lblVibrate.AutoSize = true;
            this.lblVibrate.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(162)));
            this.lblVibrate.Location = new System.Drawing.Point(195, 302);
            this.lblVibrate.Name = "lblVibrate";
            this.lblVibrate.Size = new System.Drawing.Size(55, 13);
            this.lblVibrate.TabIndex = 19;
            this.lblVibrate.Text = "Vibrate: ";
            // 
            // cmbSensor
            // 
            this.cmbSensor.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbSensor.FormattingEnabled = true;
            this.cmbSensor.Items.AddRange(new object[] {
            "NONE",
            "ACCEL",
            "GYRO",
            "MAG"});
            this.cmbSensor.Location = new System.Drawing.Point(386, 321);
            this.cmbSensor.Name = "cmbSensor";
            this.cmbSensor.Size = new System.Drawing.Size(121, 21);
            this.cmbSensor.TabIndex = 23;
            // 
            // btnSetCalib
            // 
            this.btnSetCalib.Location = new System.Drawing.Point(510, 320);
            this.btnSetCalib.Name = "btnSetCalib";
            this.btnSetCalib.Size = new System.Drawing.Size(70, 23);
            this.btnSetCalib.TabIndex = 24;
            this.btnSetCalib.Text = "Calibrate";
            this.btnSetCalib.UseVisualStyleBackColor = true;
            this.btnSetCalib.Click += new System.EventHandler(this.btnSetCalib_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(587, 276);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(24, 13);
            this.label1.TabIndex = 25;
            this.label1.Text = "Min";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(587, 302);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(27, 13);
            this.label2.TabIndex = 26;
            this.label2.Text = "Max";
            // 
            // btnGetCalib
            // 
            this.btnGetCalib.Location = new System.Drawing.Point(581, 320);
            this.btnGetCalib.Name = "btnGetCalib";
            this.btnGetCalib.Size = new System.Drawing.Size(70, 23);
            this.btnGetCalib.TabIndex = 27;
            this.btnGetCalib.Text = "Get Calib";
            this.btnGetCalib.UseVisualStyleBackColor = true;
            this.btnGetCalib.Click += new System.EventHandler(this.btnGetCalib_Click);
            // 
            // minX
            // 
            this.minX.Location = new System.Drawing.Point(416, 348);
            this.minX.Name = "minX";
            this.minX.Size = new System.Drawing.Size(74, 20);
            this.minX.TabIndex = 28;
            this.minX.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // minY
            // 
            this.minY.Location = new System.Drawing.Point(496, 349);
            this.minY.Name = "minY";
            this.minY.Size = new System.Drawing.Size(74, 20);
            this.minY.TabIndex = 29;
            this.minY.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // minZ
            // 
            this.minZ.Location = new System.Drawing.Point(576, 349);
            this.minZ.Name = "minZ";
            this.minZ.Size = new System.Drawing.Size(74, 20);
            this.minZ.TabIndex = 30;
            this.minZ.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // maxZ
            // 
            this.maxZ.Location = new System.Drawing.Point(576, 375);
            this.maxZ.Name = "maxZ";
            this.maxZ.Size = new System.Drawing.Size(74, 20);
            this.maxZ.TabIndex = 33;
            this.maxZ.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // maxY
            // 
            this.maxY.Location = new System.Drawing.Point(496, 375);
            this.maxY.Name = "maxY";
            this.maxY.Size = new System.Drawing.Size(74, 20);
            this.maxY.TabIndex = 32;
            this.maxY.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // maxX
            // 
            this.maxX.Location = new System.Drawing.Point(416, 374);
            this.maxX.Name = "maxX";
            this.maxX.Size = new System.Drawing.Size(74, 20);
            this.maxX.TabIndex = 31;
            this.maxX.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // dgProperties
            // 
            this.dgProperties.AllowUserToAddRows = false;
            this.dgProperties.AllowUserToDeleteRows = false;
            this.dgProperties.AllowUserToResizeColumns = false;
            this.dgProperties.AllowUserToResizeRows = false;
            this.dgProperties.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgProperties.ColumnHeadersVisible = false;
            this.dgProperties.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Property,
            this.Value});
            this.dgProperties.Location = new System.Drawing.Point(6, 19);
            this.dgProperties.MultiSelect = false;
            this.dgProperties.Name = "dgProperties";
            this.dgProperties.ReadOnly = true;
            this.dgProperties.RowHeadersVisible = false;
            this.dgProperties.ShowEditingIcon = false;
            this.dgProperties.ShowRowErrors = false;
            this.dgProperties.Size = new System.Drawing.Size(164, 270);
            this.dgProperties.TabIndex = 43;
            // 
            // Property
            // 
            this.Property.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle1.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            this.Property.DefaultCellStyle = dataGridViewCellStyle1;
            this.Property.HeaderText = "Property";
            this.Property.Name = "Property";
            this.Property.ReadOnly = true;
            this.Property.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.Property.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // Value
            // 
            this.Value.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            dataGridViewCellStyle2.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleRight;
            this.Value.DefaultCellStyle = dataGridViewCellStyle2;
            this.Value.HeaderText = "Value";
            this.Value.Name = "Value";
            this.Value.ReadOnly = true;
            this.Value.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.Value.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(162)));
            this.label3.Location = new System.Drawing.Point(3, 3);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(59, 13);
            this.label3.TabIndex = 44;
            this.label3.Text = "Statistics";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(162)));
            this.label4.Location = new System.Drawing.Point(3, 302);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(121, 13);
            this.label4.TabIndex = 46;
            this.label4.Text = "Rotation Quaternion";
            // 
            // dgQuaternion
            // 
            this.dgQuaternion.AllowUserToAddRows = false;
            this.dgQuaternion.AllowUserToDeleteRows = false;
            this.dgQuaternion.AllowUserToResizeColumns = false;
            this.dgQuaternion.AllowUserToResizeRows = false;
            this.dgQuaternion.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgQuaternion.ColumnHeadersVisible = false;
            this.dgQuaternion.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dataGridViewTextBoxColumn1,
            this.dataGridViewTextBoxColumn2});
            this.dgQuaternion.Location = new System.Drawing.Point(6, 318);
            this.dgQuaternion.MultiSelect = false;
            this.dgQuaternion.Name = "dgQuaternion";
            this.dgQuaternion.ReadOnly = true;
            this.dgQuaternion.RowHeadersVisible = false;
            this.dgQuaternion.ShowEditingIcon = false;
            this.dgQuaternion.ShowRowErrors = false;
            this.dgQuaternion.Size = new System.Drawing.Size(164, 91);
            this.dgQuaternion.TabIndex = 45;
            // 
            // dataGridViewTextBoxColumn1
            // 
            this.dataGridViewTextBoxColumn1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle3.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            this.dataGridViewTextBoxColumn1.DefaultCellStyle = dataGridViewCellStyle3;
            this.dataGridViewTextBoxColumn1.HeaderText = "Property";
            this.dataGridViewTextBoxColumn1.Name = "dataGridViewTextBoxColumn1";
            this.dataGridViewTextBoxColumn1.ReadOnly = true;
            this.dataGridViewTextBoxColumn1.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridViewTextBoxColumn1.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.dataGridViewTextBoxColumn1.Width = 50;
            // 
            // dataGridViewTextBoxColumn2
            // 
            this.dataGridViewTextBoxColumn2.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            dataGridViewCellStyle4.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleRight;
            dataGridViewCellStyle4.Format = "N6";
            dataGridViewCellStyle4.NullValue = null;
            this.dataGridViewTextBoxColumn2.DefaultCellStyle = dataGridViewCellStyle4;
            this.dataGridViewTextBoxColumn2.HeaderText = "Value";
            this.dataGridViewTextBoxColumn2.Name = "dataGridViewTextBoxColumn2";
            this.dataGridViewTextBoxColumn2.ReadOnly = true;
            this.dataGridViewTextBoxColumn2.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridViewTextBoxColumn2.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(162)));
            this.label5.Location = new System.Drawing.Point(195, 3);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(50, 13);
            this.label5.TabIndex = 48;
            this.label5.Text = "Buttons";
            // 
            // dgButtons
            // 
            this.dgButtons.AllowUserToAddRows = false;
            this.dgButtons.AllowUserToDeleteRows = false;
            this.dgButtons.AllowUserToResizeColumns = false;
            this.dgButtons.AllowUserToResizeRows = false;
            this.dgButtons.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgButtons.ColumnHeadersVisible = false;
            this.dgButtons.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dataGridViewTextBoxColumn3,
            this.ButX,
            this.ButY});
            this.dgButtons.Location = new System.Drawing.Point(198, 19);
            this.dgButtons.MultiSelect = false;
            this.dgButtons.Name = "dgButtons";
            this.dgButtons.ReadOnly = true;
            this.dgButtons.RowHeadersVisible = false;
            this.dgButtons.ShowEditingIcon = false;
            this.dgButtons.ShowRowErrors = false;
            this.dgButtons.Size = new System.Drawing.Size(164, 270);
            this.dgButtons.TabIndex = 47;
            // 
            // dataGridViewTextBoxColumn3
            // 
            this.dataGridViewTextBoxColumn3.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle5.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            this.dataGridViewTextBoxColumn3.DefaultCellStyle = dataGridViewCellStyle5;
            this.dataGridViewTextBoxColumn3.HeaderText = "Property";
            this.dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
            this.dataGridViewTextBoxColumn3.ReadOnly = true;
            this.dataGridViewTextBoxColumn3.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridViewTextBoxColumn3.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.dataGridViewTextBoxColumn3.Width = 50;
            // 
            // ButX
            // 
            this.ButX.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle6.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleRight;
            dataGridViewCellStyle6.Format = "N3";
            dataGridViewCellStyle6.NullValue = null;
            this.ButX.DefaultCellStyle = dataGridViewCellStyle6;
            this.ButX.HeaderText = "ButX";
            this.ButX.Name = "ButX";
            this.ButX.ReadOnly = true;
            this.ButX.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.ButX.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.ButX.Width = 50;
            // 
            // ButY
            // 
            this.ButY.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            dataGridViewCellStyle7.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleRight;
            dataGridViewCellStyle7.Format = "N3";
            this.ButY.DefaultCellStyle = dataGridViewCellStyle7;
            this.ButY.HeaderText = "ButY";
            this.ButY.Name = "ButY";
            this.ButY.ReadOnly = true;
            this.ButY.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.ButY.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(162)));
            this.label6.Location = new System.Drawing.Point(383, 3);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(63, 13);
            this.label6.TabIndex = 50;
            this.label6.Text = "Raw Data";
            // 
            // dgRaw
            // 
            this.dgRaw.AllowUserToAddRows = false;
            this.dgRaw.AllowUserToDeleteRows = false;
            this.dgRaw.AllowUserToResizeColumns = false;
            this.dgRaw.AllowUserToResizeRows = false;
            dataGridViewCellStyle8.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle8.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle8.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(162)));
            dataGridViewCellStyle8.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle8.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle8.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle8.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.dgRaw.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle8;
            this.dgRaw.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgRaw.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dataGridViewTextBoxColumn5,
            this.Prop2,
            this.dataGridViewTextBoxColumn6,
            this.ValueY,
            this.ValueZ});
            this.dgRaw.Location = new System.Drawing.Point(386, 19);
            this.dgRaw.MultiSelect = false;
            this.dgRaw.Name = "dgRaw";
            this.dgRaw.ReadOnly = true;
            this.dgRaw.RowHeadersVisible = false;
            this.dgRaw.ShowEditingIcon = false;
            this.dgRaw.ShowRowErrors = false;
            this.dgRaw.Size = new System.Drawing.Size(265, 296);
            this.dgRaw.TabIndex = 49;
            // 
            // dataGridViewTextBoxColumn5
            // 
            this.dataGridViewTextBoxColumn5.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle9.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            this.dataGridViewTextBoxColumn5.DefaultCellStyle = dataGridViewCellStyle9;
            this.dataGridViewTextBoxColumn5.HeaderText = "Sensor";
            this.dataGridViewTextBoxColumn5.Name = "dataGridViewTextBoxColumn5";
            this.dataGridViewTextBoxColumn5.ReadOnly = true;
            this.dataGridViewTextBoxColumn5.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridViewTextBoxColumn5.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.dataGridViewTextBoxColumn5.Width = 50;
            // 
            // Prop2
            // 
            this.Prop2.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle10.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            this.Prop2.DefaultCellStyle = dataGridViewCellStyle10;
            this.Prop2.HeaderText = "";
            this.Prop2.Name = "Prop2";
            this.Prop2.ReadOnly = true;
            this.Prop2.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.Prop2.Width = 30;
            // 
            // dataGridViewTextBoxColumn6
            // 
            this.dataGridViewTextBoxColumn6.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle11.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleRight;
            dataGridViewCellStyle11.Format = "N0";
            dataGridViewCellStyle11.NullValue = "N0";
            this.dataGridViewTextBoxColumn6.DefaultCellStyle = dataGridViewCellStyle11;
            this.dataGridViewTextBoxColumn6.HeaderText = "X";
            this.dataGridViewTextBoxColumn6.Name = "dataGridViewTextBoxColumn6";
            this.dataGridViewTextBoxColumn6.ReadOnly = true;
            this.dataGridViewTextBoxColumn6.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridViewTextBoxColumn6.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.dataGridViewTextBoxColumn6.Width = 60;
            // 
            // ValueY
            // 
            this.ValueY.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle12.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleRight;
            dataGridViewCellStyle12.Format = "N0";
            this.ValueY.DefaultCellStyle = dataGridViewCellStyle12;
            this.ValueY.HeaderText = "Y";
            this.ValueY.Name = "ValueY";
            this.ValueY.ReadOnly = true;
            this.ValueY.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.ValueY.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.ValueY.Width = 60;
            // 
            // ValueZ
            // 
            this.ValueZ.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            dataGridViewCellStyle13.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleRight;
            dataGridViewCellStyle13.Format = "N0";
            this.ValueZ.DefaultCellStyle = dataGridViewCellStyle13;
            this.ValueZ.HeaderText = "Z";
            this.ValueZ.Name = "ValueZ";
            this.ValueZ.ReadOnly = true;
            this.ValueZ.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.ValueZ.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.ValueZ.Width = 60;
            // 
            // TrackedDevice
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.label6);
            this.Controls.Add(this.dgRaw);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.dgButtons);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.dgQuaternion);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.dgProperties);
            this.Controls.Add(this.maxZ);
            this.Controls.Add(this.maxY);
            this.Controls.Add(this.maxX);
            this.Controls.Add(this.minZ);
            this.Controls.Add(this.minY);
            this.Controls.Add(this.minX);
            this.Controls.Add(this.btnGetCalib);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnSetCalib);
            this.Controls.Add(this.cmbSensor);
            this.Controls.Add(this.lblVibrate);
            this.Controls.Add(this.cmbSwitchState);
            this.Name = "TrackedDevice";
            this.Size = new System.Drawing.Size(655, 423);
            ((System.ComponentModel.ISupportInitialize)(this.dgProperties)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgQuaternion)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgButtons)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dgRaw)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbSwitchState;
        private System.Windows.Forms.Label lblVibrate;
        private System.Windows.Forms.ComboBox cmbSensor;
        private System.Windows.Forms.Button btnSetCalib;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btnGetCalib;
        private System.Windows.Forms.TextBox minX;
        private System.Windows.Forms.TextBox minY;
        private System.Windows.Forms.TextBox minZ;
        private System.Windows.Forms.TextBox maxZ;
        private System.Windows.Forms.TextBox maxY;
        private System.Windows.Forms.TextBox maxX;
        private System.Windows.Forms.DataGridView dgProperties;
        private System.Windows.Forms.DataGridViewTextBoxColumn Property;
        private System.Windows.Forms.DataGridViewTextBoxColumn Value;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.DataGridView dgQuaternion;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.DataGridView dgButtons;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn2;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.DataGridView dgRaw;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn5;
        private System.Windows.Forms.DataGridViewTextBoxColumn Prop2;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn6;
        private System.Windows.Forms.DataGridViewTextBoxColumn ValueY;
        private System.Windows.Forms.DataGridViewTextBoxColumn ValueZ;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
        private System.Windows.Forms.DataGridViewTextBoxColumn ButX;
        private System.Windows.Forms.DataGridViewTextBoxColumn ButY;
    }
}
