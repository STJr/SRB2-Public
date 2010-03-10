namespace SRB2Updater
{
    partial class Changelog
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
            this.changelog_browser = new System.Windows.Forms.WebBrowser();
            this.SuspendLayout();
            // 
            // changelog_browser
            // 
            this.changelog_browser.AccessibleName = "Changelog";
            this.changelog_browser.AccessibleRole = System.Windows.Forms.AccessibleRole.None;
            this.changelog_browser.AllowNavigation = false;
            this.changelog_browser.AllowWebBrowserDrop = false;
            this.changelog_browser.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.changelog_browser.IsWebBrowserContextMenuEnabled = false;
            this.changelog_browser.Location = new System.Drawing.Point(0, 0);
            this.changelog_browser.Margin = new System.Windows.Forms.Padding(0);
            this.changelog_browser.MinimumSize = new System.Drawing.Size(20, 20);
            this.changelog_browser.Name = "changelog_browser";
            this.changelog_browser.ScriptErrorsSuppressed = true;
            this.changelog_browser.Size = new System.Drawing.Size(333, 437);
            this.changelog_browser.TabIndex = 0;
            this.changelog_browser.Url = new System.Uri("http://update.srb2.org/files/changelog.html", System.UriKind.Absolute);
            this.changelog_browser.WebBrowserShortcutsEnabled = false;
            // 
            // Changelog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(333, 437);
            this.Controls.Add(this.changelog_browser);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "Changelog";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Changelog";
            this.Load += new System.EventHandler(this.Changelog_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.WebBrowser changelog_browser;
    }
}
