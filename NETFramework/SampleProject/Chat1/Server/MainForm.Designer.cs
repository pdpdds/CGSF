namespace ChatServer1
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
            this.listViewLobbyInfo = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.textBoxServerConfig = new System.Windows.Forms.TextBox();
            this.textBoxConnectCount = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // listBoxLog
            // 
            this.listBoxLog.FormattingEnabled = true;
            this.listBoxLog.HorizontalScrollbar = true;
            this.listBoxLog.ItemHeight = 12;
            this.listBoxLog.Location = new System.Drawing.Point(10, 71);
            this.listBoxLog.Name = "listBoxLog";
            this.listBoxLog.ScrollAlwaysVisible = true;
            this.listBoxLog.Size = new System.Drawing.Size(582, 220);
            this.listBoxLog.TabIndex = 14;
            // 
            // listViewLobbyInfo
            // 
            this.listViewLobbyInfo.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.listViewLobbyInfo.Location = new System.Drawing.Point(598, 35);
            this.listViewLobbyInfo.Name = "listViewLobbyInfo";
            this.listViewLobbyInfo.Size = new System.Drawing.Size(121, 253);
            this.listViewLobbyInfo.TabIndex = 17;
            this.listViewLobbyInfo.UseCompatibleStateImageBehavior = false;
            this.listViewLobbyInfo.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "로비ID";
            this.columnHeader1.Width = 52;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "인원";
            this.columnHeader2.Width = 39;
            // 
            // textBoxServerConfig
            // 
            this.textBoxServerConfig.Location = new System.Drawing.Point(10, 3);
            this.textBoxServerConfig.Multiline = true;
            this.textBoxServerConfig.Name = "textBoxServerConfig";
            this.textBoxServerConfig.ReadOnly = true;
            this.textBoxServerConfig.Size = new System.Drawing.Size(582, 62);
            this.textBoxServerConfig.TabIndex = 20;
            // 
            // textBoxConnectCount
            // 
            this.textBoxConnectCount.Location = new System.Drawing.Point(598, 3);
            this.textBoxConnectCount.Multiline = true;
            this.textBoxConnectCount.Name = "textBoxConnectCount";
            this.textBoxConnectCount.ReadOnly = true;
            this.textBoxConnectCount.Size = new System.Drawing.Size(120, 26);
            this.textBoxConnectCount.TabIndex = 21;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(730, 296);
            this.Controls.Add(this.textBoxConnectCount);
            this.Controls.Add(this.textBoxServerConfig);
            this.Controls.Add(this.listViewLobbyInfo);
            this.Controls.Add(this.listBoxLog);
            this.Name = "MainForm";
            this.Text = "ChatServer1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox listBoxLog;
        private System.Windows.Forms.ListView listViewLobbyInfo;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.TextBox textBoxServerConfig;
        private System.Windows.Forms.TextBox textBoxConnectCount;
    }
}

