namespace ChatClient1
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
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.checkBoxLocalHostIP = new System.Windows.Forms.CheckBox();
            this.textBoxIP = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.labelStatus = new System.Windows.Forms.Label();
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.listBoxChat = new System.Windows.Forms.ListBox();
            this.textBoxSendChat = new System.Windows.Forms.TextBox();
            this.button5 = new System.Windows.Forms.Button();
            this.listBoxLog = new System.Windows.Forms.ListBox();
            this.textBoxUserID = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.textBoxLobbyID = new System.Windows.Forms.TextBox();
            this.btnLobbyEnterLeave = new System.Windows.Forms.Button();
            this.groupBox5.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.textBoxPort);
            this.groupBox5.Controls.Add(this.label10);
            this.groupBox5.Controls.Add(this.checkBoxLocalHostIP);
            this.groupBox5.Controls.Add(this.textBoxIP);
            this.groupBox5.Controls.Add(this.label9);
            this.groupBox5.Location = new System.Drawing.Point(12, 12);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(403, 52);
            this.groupBox5.TabIndex = 23;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Socket 더미 클라이언트 설정";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(225, 20);
            this.textBoxPort.MaxLength = 6;
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(51, 21);
            this.textBoxPort.TabIndex = 18;
            this.textBoxPort.Text = "25251";
            this.textBoxPort.WordWrap = false;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(163, 24);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(61, 12);
            this.label10.TabIndex = 17;
            this.label10.Text = "포트 번호:";
            // 
            // checkBoxLocalHostIP
            // 
            this.checkBoxLocalHostIP.AutoSize = true;
            this.checkBoxLocalHostIP.Checked = true;
            this.checkBoxLocalHostIP.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxLocalHostIP.Location = new System.Drawing.Point(285, 24);
            this.checkBoxLocalHostIP.Name = "checkBoxLocalHostIP";
            this.checkBoxLocalHostIP.Size = new System.Drawing.Size(103, 16);
            this.checkBoxLocalHostIP.TabIndex = 15;
            this.checkBoxLocalHostIP.Text = "localhost 사용";
            this.checkBoxLocalHostIP.UseVisualStyleBackColor = true;
            // 
            // textBoxIP
            // 
            this.textBoxIP.Location = new System.Drawing.Point(68, 19);
            this.textBoxIP.MaxLength = 6;
            this.textBoxIP.Name = "textBoxIP";
            this.textBoxIP.Size = new System.Drawing.Size(87, 21);
            this.textBoxIP.TabIndex = 11;
            this.textBoxIP.Text = "0.0.0.0";
            this.textBoxIP.WordWrap = false;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(6, 23);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(61, 12);
            this.label9.TabIndex = 10;
            this.label9.Text = "서버 주소:";
            // 
            // labelStatus
            // 
            this.labelStatus.AutoSize = true;
            this.labelStatus.Location = new System.Drawing.Point(12, 566);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(111, 12);
            this.labelStatus.TabIndex = 24;
            this.labelStatus.Text = "서버 접속 상태: ???";
            // 
            // btnConnect
            // 
            this.btnConnect.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.btnConnect.Location = new System.Drawing.Point(421, 25);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(81, 30);
            this.btnConnect.TabIndex = 24;
            this.btnConnect.Text = "접속하기";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.btnDisconnect.Location = new System.Drawing.Point(508, 25);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(77, 30);
            this.btnDisconnect.TabIndex = 26;
            this.btnDisconnect.Text = "접속 끊기";
            this.btnDisconnect.UseVisualStyleBackColor = true;
            this.btnDisconnect.Click += new System.EventHandler(this.button3_Click);
            // 
            // listBoxChat
            // 
            this.listBoxChat.FormattingEnabled = true;
            this.listBoxChat.ItemHeight = 12;
            this.listBoxChat.Location = new System.Drawing.Point(12, 106);
            this.listBoxChat.Name = "listBoxChat";
            this.listBoxChat.Size = new System.Drawing.Size(573, 268);
            this.listBoxChat.TabIndex = 31;
            // 
            // textBoxSendChat
            // 
            this.textBoxSendChat.Location = new System.Drawing.Point(12, 383);
            this.textBoxSendChat.Name = "textBoxSendChat";
            this.textBoxSendChat.Size = new System.Drawing.Size(521, 21);
            this.textBoxSendChat.TabIndex = 32;
            // 
            // button5
            // 
            this.button5.Location = new System.Drawing.Point(539, 383);
            this.button5.Name = "button5";
            this.button5.Size = new System.Drawing.Size(45, 23);
            this.button5.TabIndex = 33;
            this.button5.Text = "채팅";
            this.button5.UseVisualStyleBackColor = true;
            this.button5.Click += new System.EventHandler(this.button5_Click);
            // 
            // listBoxLog
            // 
            this.listBoxLog.FormattingEnabled = true;
            this.listBoxLog.HorizontalScrollbar = true;
            this.listBoxLog.ItemHeight = 12;
            this.listBoxLog.Location = new System.Drawing.Point(12, 417);
            this.listBoxLog.Name = "listBoxLog";
            this.listBoxLog.Size = new System.Drawing.Size(573, 136);
            this.listBoxLog.TabIndex = 35;
            // 
            // textBoxUserID
            // 
            this.textBoxUserID.Location = new System.Drawing.Point(15, 71);
            this.textBoxUserID.MaxLength = 6;
            this.textBoxUserID.Name = "textBoxUserID";
            this.textBoxUserID.Size = new System.Drawing.Size(87, 21);
            this.textBoxUserID.TabIndex = 36;
            this.textBoxUserID.Text = "test1";
            this.textBoxUserID.WordWrap = false;
            // 
            // button1
            // 
            this.button1.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.button1.Location = new System.Drawing.Point(108, 67);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(81, 30);
            this.button1.TabIndex = 37;
            this.button1.Text = "로그인";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // textBoxLobbyID
            // 
            this.textBoxLobbyID.Location = new System.Drawing.Point(211, 74);
            this.textBoxLobbyID.MaxLength = 6;
            this.textBoxLobbyID.Name = "textBoxLobbyID";
            this.textBoxLobbyID.Size = new System.Drawing.Size(35, 21);
            this.textBoxLobbyID.TabIndex = 38;
            this.textBoxLobbyID.Text = "1";
            this.textBoxLobbyID.WordWrap = false;
            // 
            // btnLobbyEnterLeave
            // 
            this.btnLobbyEnterLeave.Font = new System.Drawing.Font("맑은 고딕", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.btnLobbyEnterLeave.Location = new System.Drawing.Point(252, 68);
            this.btnLobbyEnterLeave.Name = "btnLobbyEnterLeave";
            this.btnLobbyEnterLeave.Size = new System.Drawing.Size(94, 30);
            this.btnLobbyEnterLeave.TabIndex = 39;
            this.btnLobbyEnterLeave.Text = "로비 입장";
            this.btnLobbyEnterLeave.UseVisualStyleBackColor = true;
            this.btnLobbyEnterLeave.Click += new System.EventHandler(this.btnLobbyEnterLeave_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(597, 587);
            this.Controls.Add(this.btnLobbyEnterLeave);
            this.Controls.Add(this.textBoxLobbyID);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.textBoxUserID);
            this.Controls.Add(this.listBoxLog);
            this.Controls.Add(this.button5);
            this.Controls.Add(this.textBoxSendChat);
            this.Controls.Add(this.listBoxChat);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.groupBox5);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "MainForm";
            this.Text = "Chat Client";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.CheckBox checkBoxLocalHostIP;
        private System.Windows.Forms.TextBox textBoxIP;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnDisconnect;
        private System.Windows.Forms.ListBox listBoxChat;
        private System.Windows.Forms.TextBox textBoxSendChat;
        private System.Windows.Forms.Button button5;
        private System.Windows.Forms.ListBox listBoxLog;
        private System.Windows.Forms.TextBox textBoxUserID;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox textBoxLobbyID;
        private System.Windows.Forms.Button btnLobbyEnterLeave;
    }
}

