namespace SRB2Updater
{
    partial class Form1
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
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle3 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.prgDownload = new System.Windows.Forms.ProgressBar();
            this.lblProgress = new System.Windows.Forms.Label();
            this.fileList = new System.Windows.Forms.DataGridView();
            this.name = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.filename = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.status = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.localmd5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.md5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.optional = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.update_optional = new System.Windows.Forms.CheckBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.label1 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.fileList)).BeginInit();
            this.SuspendLayout();
            // 
            // prgDownload
            // 
            this.prgDownload.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.prgDownload.Location = new System.Drawing.Point(12, 335);
            this.prgDownload.Name = "prgDownload";
            this.prgDownload.Size = new System.Drawing.Size(507, 23);
            this.prgDownload.TabIndex = 3;
            // 
            // lblProgress
            // 
            this.lblProgress.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.lblProgress.AutoSize = true;
            this.lblProgress.Font = new System.Drawing.Font("Tahoma", 12.75F);
            this.lblProgress.ForeColor = System.Drawing.Color.Black;
            this.lblProgress.Location = new System.Drawing.Point(8, 312);
            this.lblProgress.Name = "lblProgress";
            this.lblProgress.Size = new System.Drawing.Size(130, 21);
            this.lblProgress.TabIndex = 8;
            this.lblProgress.Text = "Checking Files...";
            this.lblProgress.Click += new System.EventHandler(this.lblProgress_Click);
            // 
            // fileList
            // 
            this.fileList.AllowUserToAddRows = false;
            this.fileList.AllowUserToDeleteRows = false;
            this.fileList.AllowUserToResizeRows = false;
            this.fileList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.fileList.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
            this.fileList.BackgroundColor = System.Drawing.Color.FromArgb(((int)(((byte)(223)))), ((int)(((byte)(230)))), ((int)(((byte)(244)))));
            this.fileList.ColumnHeadersBorderStyle = System.Windows.Forms.DataGridViewHeaderBorderStyle.None;
            dataGridViewCellStyle1.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(17)))), ((int)(((byte)(136)))), ((int)(((byte)(255)))));
            dataGridViewCellStyle1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            dataGridViewCellStyle1.ForeColor = System.Drawing.Color.White;
            dataGridViewCellStyle1.Padding = new System.Windows.Forms.Padding(2);
            dataGridViewCellStyle1.SelectionBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(17)))), ((int)(((byte)(136)))), ((int)(((byte)(255)))));
            dataGridViewCellStyle1.SelectionForeColor = System.Drawing.Color.White;
            dataGridViewCellStyle1.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.fileList.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle1;
            this.fileList.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.fileList.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.name,
            this.filename,
            this.status,
            this.localmd5,
            this.md5,
            this.optional});
            this.fileList.GridColor = System.Drawing.Color.FromArgb(((int)(((byte)(223)))), ((int)(((byte)(230)))), ((int)(((byte)(244)))));
            this.fileList.Location = new System.Drawing.Point(12, 89);
            this.fileList.MultiSelect = false;
            this.fileList.Name = "fileList";
            this.fileList.ReadOnly = true;
            dataGridViewCellStyle3.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleLeft;
            dataGridViewCellStyle3.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle3.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle3.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle3.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle3.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle3.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.fileList.RowHeadersDefaultCellStyle = dataGridViewCellStyle3;
            this.fileList.RowHeadersVisible = false;
            this.fileList.RowTemplate.ReadOnly = true;
            this.fileList.Size = new System.Drawing.Size(507, 209);
            this.fileList.TabIndex = 10;
//            this.fileList.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.fileList_CellContentClick);
            // 
            // name
            // 
            this.name.DataPropertyName = "name";
            dataGridViewCellStyle2.BackColor = System.Drawing.Color.White;
            this.name.DefaultCellStyle = dataGridViewCellStyle2;
            this.name.FillWeight = 128.7982F;
            this.name.HeaderText = "Name";
            this.name.Name = "name";
            this.name.ReadOnly = true;
            // 
            // filename
            // 
            this.filename.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.None;
            this.filename.DataPropertyName = "filename";
            this.filename.HeaderText = "File";
            this.filename.Name = "filename";
            this.filename.ReadOnly = true;
            this.filename.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
            this.filename.Width = 150;
            // 
            // status
            // 
            this.status.DataPropertyName = "status";
            this.status.FillWeight = 128.7982F;
            this.status.HeaderText = "Status";
            this.status.Name = "status";
            this.status.ReadOnly = true;
            // 
            // localmd5
            // 
            this.localmd5.DataPropertyName = "localmd5";
            this.localmd5.FillWeight = 13.60544F;
            this.localmd5.HeaderText = "localmd5";
            this.localmd5.Name = "localmd5";
            this.localmd5.ReadOnly = true;
            this.localmd5.Visible = false;
            // 
            // md5
            // 
            this.md5.DataPropertyName = "md5";
            this.md5.FillWeight = 128.7982F;
            this.md5.HeaderText = "md5";
            this.md5.Name = "md5";
            this.md5.ReadOnly = true;
            this.md5.Visible = false;
            // 
            // optional
            // 
            this.optional.DataPropertyName = "optional";
            this.optional.HeaderText = "optional";
            this.optional.Name = "optional";
            this.optional.ReadOnly = true;
            this.optional.Visible = false;
            // 
            // update_optional
            // 
            this.update_optional.AutoSize = true;
            this.update_optional.Location = new System.Drawing.Point(360, 364);
            this.update_optional.Name = "update_optional";
            this.update_optional.Size = new System.Drawing.Size(159, 17);
            this.update_optional.TabIndex = 12;
            this.update_optional.Text = "Download Optional Updates";
            this.update_optional.UseVisualStyleBackColor = true;
            this.update_optional.CheckedChanged += new System.EventHandler(this.update_Reload);
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.Color.Yellow;
            this.panel1.BackgroundImage = global::SRB2Updater.Properties.Resources.updaterbanner;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(532, 80);
            this.panel1.TabIndex = 11;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Cursor = System.Windows.Forms.Cursors.Hand;
            this.label1.Location = new System.Drawing.Point(13, 364);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(84, 13);
            this.label1.TabIndex = 13;
            this.label1.Text = "View Changelog";
            this.label1.Click += new System.EventHandler(this.label1_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(213)))), ((int)(((byte)(227)))), ((int)(((byte)(255)))));
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.ClientSize = new System.Drawing.Size(531, 387);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.update_optional);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.fileList);
            this.Controls.Add(this.lblProgress);
            this.Controls.Add(this.prgDownload);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Show;
            this.Text = "Sonic Robo Blast 2 Automatic Updater";
            this.Load += new System.EventHandler(this.update_Load);
            ((System.ComponentModel.ISupportInitialize)(this.fileList)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form1_FormClosed);

        }

        #endregion

        private System.Windows.Forms.ProgressBar prgDownload;
        private System.Windows.Forms.Label lblProgress;
        private System.Windows.Forms.DataGridView fileList;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.CheckBox update_optional;
        private System.Windows.Forms.DataGridViewTextBoxColumn name;
        private System.Windows.Forms.DataGridViewTextBoxColumn filename;
        private System.Windows.Forms.DataGridViewTextBoxColumn status;
        private System.Windows.Forms.DataGridViewTextBoxColumn localmd5;
        private System.Windows.Forms.DataGridViewTextBoxColumn md5;
        private System.Windows.Forms.DataGridViewTextBoxColumn optional;
        private System.Windows.Forms.Label label1;
    }
}
