namespace GateWayServer
{
    partial class MainForm
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다.
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마십시오.
        /// </summary>
        private void InitializeComponent()
        {
            this.listBoxLog = new System.Windows.Forms.ListBox();
            this.textBoxServerConfig = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.textBoxConnectCount = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // listBoxLog
            // 
            this.listBoxLog.FormattingEnabled = true;
            this.listBoxLog.HorizontalScrollbar = true;
            this.listBoxLog.ItemHeight = 12;
            this.listBoxLog.Location = new System.Drawing.Point(10, 72);
            this.listBoxLog.Name = "listBoxLog";
            this.listBoxLog.ScrollAlwaysVisible = true;
            this.listBoxLog.Size = new System.Drawing.Size(698, 232);
            this.listBoxLog.TabIndex = 14;
            // 
            // textBoxServerConfig
            // 
            this.textBoxServerConfig.Location = new System.Drawing.Point(10, 3);
            this.textBoxServerConfig.Multiline = true;
            this.textBoxServerConfig.Name = "textBoxServerConfig";
            this.textBoxServerConfig.ReadOnly = true;
            this.textBoxServerConfig.Size = new System.Drawing.Size(563, 53);
            this.textBoxServerConfig.TabIndex = 20;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(579, 30);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(129, 30);
            this.button1.TabIndex = 22;
            this.button1.Text = "---";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // textBoxConnectCount
            // 
            this.textBoxConnectCount.Location = new System.Drawing.Point(579, 3);
            this.textBoxConnectCount.Name = "textBoxConnectCount";
            this.textBoxConnectCount.ReadOnly = true;
            this.textBoxConnectCount.Size = new System.Drawing.Size(129, 21);
            this.textBoxConnectCount.TabIndex = 23;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(720, 317);
            this.Controls.Add(this.textBoxConnectCount);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.textBoxServerConfig);
            this.Controls.Add(this.listBoxLog);
            this.Name = "MainForm";
            this.Text = "Server";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox listBoxLog;
        private System.Windows.Forms.TextBox textBoxServerConfig;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox textBoxConnectCount;
    }
}

