namespace ChatServerHost
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
            this.textBoxMaxUserPerLobby = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.textBoxLobbyCount = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxPacketThreadCount = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label8 = new System.Windows.Forms.Label();
            this.textBoxMaxPacketSize = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.textBoxMaxBufferSize = new System.Windows.Forms.TextBox();
            this.textBoxMaxUserCount = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.textBoxDBThreadCount = new System.Windows.Forms.TextBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxCurrentUserCount = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.listBoxLog = new System.Windows.Forms.ListBox();
            this.listViewLobbyInfo = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // textBoxMaxUserPerLobby
            // 
            this.textBoxMaxUserPerLobby.Location = new System.Drawing.Point(647, 42);
            this.textBoxMaxUserPerLobby.Name = "textBoxMaxUserPerLobby";
            this.textBoxMaxUserPerLobby.Size = new System.Drawing.Size(52, 21);
            this.textBoxMaxUserPerLobby.TabIndex = 7;
            this.textBoxMaxUserPerLobby.Text = "100";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(538, 46);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(105, 12);
            this.label6.TabIndex = 6;
            this.label6.Text = "로비 최대 유저 수:";
            // 
            // textBoxLobbyCount
            // 
            this.textBoxLobbyCount.Location = new System.Drawing.Point(646, 16);
            this.textBoxLobbyCount.Name = "textBoxLobbyCount";
            this.textBoxLobbyCount.Size = new System.Drawing.Size(53, 21);
            this.textBoxLobbyCount.TabIndex = 3;
            this.textBoxLobbyCount.Text = "10";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(593, 21);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(49, 12);
            this.label3.TabIndex = 2;
            this.label3.Text = "로비 수:";
            // 
            // textBoxPacketThreadCount
            // 
            this.textBoxPacketThreadCount.Location = new System.Drawing.Point(279, 15);
            this.textBoxPacketThreadCount.Name = "textBoxPacketThreadCount";
            this.textBoxPacketThreadCount.Size = new System.Drawing.Size(32, 21);
            this.textBoxPacketThreadCount.TabIndex = 1;
            this.textBoxPacketThreadCount.Text = "3";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(158, 20);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(117, 12);
            this.label4.TabIndex = 0;
            this.label4.Text = "패킷 처리 스레드 수:";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label8);
            this.groupBox1.Controls.Add(this.textBoxMaxPacketSize);
            this.groupBox1.Controls.Add(this.label9);
            this.groupBox1.Controls.Add(this.textBoxMaxBufferSize);
            this.groupBox1.Controls.Add(this.textBoxMaxUserCount);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.textBoxDBThreadCount);
            this.groupBox1.Controls.Add(this.textBoxMaxUserPerLobby);
            this.groupBox1.Controls.Add(this.textBoxPort);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.textBoxLobbyCount);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.textBoxPacketThreadCount);
            this.groupBox1.Location = new System.Drawing.Point(12, 7);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(810, 82);
            this.groupBox1.TabIndex = 6;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "서버 정보";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(381, 46);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(89, 12);
            this.label8.TabIndex = 12;
            this.label8.Text = "최대 패킷 크기:";
            // 
            // textBoxMaxPacketSize
            // 
            this.textBoxMaxPacketSize.Location = new System.Drawing.Point(475, 41);
            this.textBoxMaxPacketSize.Name = "textBoxMaxPacketSize";
            this.textBoxMaxPacketSize.Size = new System.Drawing.Size(33, 21);
            this.textBoxMaxPacketSize.TabIndex = 13;
            this.textBoxMaxPacketSize.Text = "4021";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(337, 19);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(133, 12);
            this.label9.TabIndex = 10;
            this.label9.Text = "세션 당 최대 버퍼 크기:";
            // 
            // textBoxMaxBufferSize
            // 
            this.textBoxMaxBufferSize.Location = new System.Drawing.Point(473, 14);
            this.textBoxMaxBufferSize.Name = "textBoxMaxBufferSize";
            this.textBoxMaxBufferSize.Size = new System.Drawing.Size(45, 21);
            this.textBoxMaxBufferSize.TabIndex = 11;
            this.textBoxMaxBufferSize.Text = "32000";
            // 
            // textBoxMaxUserCount
            // 
            this.textBoxMaxUserCount.Location = new System.Drawing.Point(99, 42);
            this.textBoxMaxUserCount.Name = "textBoxMaxUserCount";
            this.textBoxMaxUserCount.Size = new System.Drawing.Size(46, 21);
            this.textBoxMaxUserCount.TabIndex = 8;
            this.textBoxMaxUserCount.Text = "1000";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(8, 46);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(89, 12);
            this.label7.TabIndex = 7;
            this.label7.Text = "최대 접속자 수:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(166, 47);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(109, 12);
            this.label5.TabIndex = 8;
            this.label5.Text = "DB 처리 스레드 수:";
            // 
            // textBoxDBThreadCount
            // 
            this.textBoxDBThreadCount.Location = new System.Drawing.Point(279, 42);
            this.textBoxDBThreadCount.Name = "textBoxDBThreadCount";
            this.textBoxDBThreadCount.Size = new System.Drawing.Size(32, 21);
            this.textBoxDBThreadCount.TabIndex = 9;
            this.textBoxDBThreadCount.Text = "4";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(99, 16);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(46, 21);
            this.textBoxPort.TabIndex = 3;
            this.textBoxPort.Text = "25251";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(38, 22);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(59, 12);
            this.label2.TabIndex = 2;
            this.label2.Text = "서버 Port:";
            // 
            // textBoxCurrentUserCount
            // 
            this.textBoxCurrentUserCount.Location = new System.Drawing.Point(111, 95);
            this.textBoxCurrentUserCount.Name = "textBoxCurrentUserCount";
            this.textBoxCurrentUserCount.Size = new System.Drawing.Size(60, 21);
            this.textBoxCurrentUserCount.TabIndex = 1;
            this.textBoxCurrentUserCount.Text = "0";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(20, 99);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(89, 12);
            this.label1.TabIndex = 0;
            this.label1.Text = "동시 접속자 수:";
            // 
            // listBoxLog
            // 
            this.listBoxLog.FormattingEnabled = true;
            this.listBoxLog.ItemHeight = 12;
            this.listBoxLog.Location = new System.Drawing.Point(12, 160);
            this.listBoxLog.Name = "listBoxLog";
            this.listBoxLog.Size = new System.Drawing.Size(684, 328);
            this.listBoxLog.TabIndex = 5;
            // 
            // listViewLobbyInfo
            // 
            this.listViewLobbyInfo.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.listViewLobbyInfo.Location = new System.Drawing.Point(702, 160);
            this.listViewLobbyInfo.Name = "listViewLobbyInfo";
            this.listViewLobbyInfo.Size = new System.Drawing.Size(121, 328);
            this.listViewLobbyInfo.TabIndex = 18;
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
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(834, 500);
            this.Controls.Add(this.listViewLobbyInfo);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.listBoxLog);
            this.Controls.Add(this.textBoxCurrentUserCount);
            this.Controls.Add(this.label1);
            this.Name = "MainForm";
            this.Text = "ChatServer";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxMaxUserPerLobby;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox textBoxLobbyCount;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxPacketThreadCount;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox textBoxDBThreadCount;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBoxCurrentUserCount;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ListBox listBoxLog;
        private System.Windows.Forms.TextBox textBoxMaxUserCount;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.ListView listViewLobbyInfo;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox textBoxMaxPacketSize;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox textBoxMaxBufferSize;
    }
}

