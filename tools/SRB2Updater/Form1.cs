using System;
//using System.Collections.Generic;
//using System.ComponentModel;
using System.Data;
//using System.Data.OleDb;
using System.Xml;
//using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.IO;
using System.Threading;
using System.Security.Cryptography;
//using System.Runtime.InteropServices;
using System.Diagnostics;

namespace SRB2Updater
{
    public partial class Form1 : Form
    {
        // The thread inside which the download happens
        private Thread thrDownload;
        // The thread inside which the splash screen happens
        private Thread thrSplash;
        private Thread thrChangeLog;
        // The stream of data retrieved from the web server
        private Stream strResponse;
        // The stream of data that we write to the harddrive
        private Stream strLocal;
        // The request to the web server for file information
        private HttpWebRequest webRequest;
        // The response from the web server containing information about the file
        private HttpWebResponse webResponse;
        // The progress of the download in percentage
        private static int PercentProgress;
        // The delegate which we will call from the thread to update the form
        private delegate void UpdateProgessCallback(Int64 BytesRead, Int64 TotalBytes);
        // When to pause
        bool goPause = false;
        // Download Details
        string downFile;
        // Updating
        bool filesGot = false;
        bool downloadStatus = false;
        string formTitle = "Sonic Robo Blast 2 Automatic Updater";
        bool loadedBat = false;
        ProcessStartInfo startinfo = new ProcessStartInfo();

        public Form1()
        {
            InitializeComponent();
        }

        public string getMD5(string filename)
        {
            StringBuilder sb = new StringBuilder();
            FileInfo f = new FileInfo(filename);
            FileStream fs = f.OpenRead();
            MD5 md5 = new MD5CryptoServiceProvider();
            byte[] hash = md5.ComputeHash(fs);
            fs.Close();
            foreach (byte hex in hash)
                sb.Append(hex.ToString("x2"));
            string md5sum = sb.ToString();
            return md5sum;
        }

        private void updateList()
        {
            if (filesGot == false)
            {

                XmlDataDocument xmlDatadoc = new XmlDataDocument();
                xmlDatadoc.DataSet.ReadXml("http://update.srb2.org/files/files.xml");
                DataSet ds = new DataSet("Files DataSet");
                ds = xmlDatadoc.DataSet;
                fileList.DataSource = ds.DefaultViewManager;
                fileList.DataMember = "File";
                filesGot = true;
                thrSplash.Abort();
                thrChangeLog = new Thread(new ThreadStart(ShowChangeLog));
                thrChangeLog.SetApartmentState(ApartmentState.STA);
                thrChangeLog.Start();
            }
            if (downloadStatus == false)
            {
                foreach (DataGridViewRow fileRow in fileList.Rows)
                {
                    if (!File.Exists(fileRow.Cells["filename"].Value.ToString()) && fileRow.Cells["filename"].Value.ToString() != "srb2update.update")
                    {
                        fileRow.DefaultCellStyle.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
                        fileRow.Cells["localmd5"].Value = "not_found";
                    } else {
                        if (fileRow.Cells["filename"].Value.ToString() == "srb2update.update")
                            fileRow.Cells["localmd5"].Value = getMD5("srb2update.exe");
                        else
                            fileRow.Cells["localmd5"].Value = getMD5(fileRow.Cells["filename"].Value.ToString());
                    }
                    if (fileRow.Cells["localmd5"].Value.ToString() != fileRow.Cells["md5"].Value.ToString())
                    {
                        if (fileRow.Cells["localmd5"].Value.ToString() != "not_found")
                            fileRow.DefaultCellStyle.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(209)))), ((int)(((byte)(255)))));
                        fileRow.Cells["status"].Value = "Queued";
                    }
                    else
                    {
                        fileRow.Cells["status"].Value = "Up to date";
                    }
                }
                foreach (DataGridViewRow fileRow in fileList.Rows)
                {
                    if (fileRow.Cells["localmd5"].Value.ToString() != fileRow.Cells["md5"].Value.ToString())
                    {
                        if (fileRow.Cells["optional"].Value.ToString() == "1" && !update_optional.Checked)
                            fileRow.Cells["Status"].Value = "Skipped (Optional)";
                        else
                        {
                            downFile = fileRow.Cells["filename"].Value.ToString();
                            thrDownload = new Thread(new ParameterizedThreadStart(Download));
                            thrDownload.Start(0);
                            fileRow.Cells["Status"].Value = "Downloading...";
                            downloadStatus = true;
                            break;
                        }
                    }
                    else
                    {
                        fileRow.Cells["Status"].Value = "Up to date";
                    }
                }
            }
            if (downloadStatus == false)
            {
                thrSplash.Abort();
                DialogResult dr = MessageBox.Show("Updates are complete, would you like to run SRB2 now?", "Updates Complete", MessageBoxButtons.YesNo);
                switch(dr){
                    case DialogResult.Yes:
                        System.Diagnostics.Process.Start(@"srb2win.exe"); Environment.Exit(0); break;
                    case DialogResult.No: break;
                }
            }
        }

        private void UpdateProgress(Int64 BytesRead, Int64 TotalBytes)
        {
            // Calculate the download progress in percentages
            PercentProgress = Convert.ToInt32((BytesRead * 100) / TotalBytes);
            // Make progress on the progress bar
            prgDownload.Value = PercentProgress;
            // Display the current progress on the form
            lblProgress.Text = downFile + " - " + (BytesRead / 1024) + "KB of " + (TotalBytes / 1024) + "KB (" + PercentProgress + "%)";
            this.Text = formTitle + " :: Downloading " + downFile + " (" + PercentProgress + "%)";
            if (BytesRead >= TotalBytes - 1)
                    updateList();          
        }

        private void Download(object startpoint)
        {
            try
            {
                string filename = Convert.ToString(startpoint);
                // Create a request to the file we are downloading
                webRequest = (HttpWebRequest)WebRequest.Create("http://update.srb2.org/updater/" + downFile);
                // Set the starting point of the request
                webRequest.AddRange(0);

                // Set default authentication for retrieving the file
                webRequest.Credentials = CredentialCache.DefaultCredentials;
                // Retrieve the response from the server
                webResponse = (HttpWebResponse)webRequest.GetResponse();
                // Ask the server for the file size and store it
                Int64 fileSize = webResponse.ContentLength;
                
                // Open the URL for download 
                strResponse = webResponse.GetResponseStream();

                // Create a new file stream where we will be saving the data (local drive)
                strLocal = new FileStream(downFile, FileMode.Create, FileAccess.Write, FileShare.None);                
                // It will store the current number of bytes we retrieved from the server
                int bytesSize = 0;
                // A buffer for storing and writing the data retrieved from the server
                byte[] downBuffer = new byte[2048];

                // Loop through the buffer until the buffer is empty
                while ((bytesSize = strResponse.Read(downBuffer, 0, downBuffer.Length)) > 0)
                {
                    // Write the data from the buffer to the local hard drive
                    strLocal.Write(downBuffer, 0, bytesSize);
                    // Invoke the method that updates the form's label and progress bar
                    this.Invoke(new UpdateProgessCallback(this.UpdateProgress), new object[] { strLocal.Length, fileSize });

                    if (goPause == true)
                    {
                        break;
                    }
                }
            }
            finally
            {
                // When the above code has ended, close the streams
                strResponse.Close();
                strLocal.Close();
                // And update the row!
                downloadStatus = false;
                if (downFile == "srb2update.update" && loadedBat != true)
                {
                    MessageBox.Show("The updater will now restart to apply a patch.", "Self Update", MessageBoxButtons.OK);
                    CreateUpdaterBat();
                    startinfo.WindowStyle = ProcessWindowStyle.Hidden;
                    startinfo.FileName = "srb2update.bat";
                    System.Diagnostics.Process.Start(startinfo);
                    downloadStatus = false;
                    Environment.Exit(0);
                } else
                    updateList();
            }
        }

        private void ShowSplash()
        {
            Application.Run(new SRB2Updater.Splash());
        }

        private void CreateUpdaterBat()
        {
            File.WriteAllText("srb2update.bat", "ping 127.0.0.1\ncopy srb2update.update srb2update.exe\ndel srb2update.update\nsrb2update.exe\nexit");
        }

        private void ShowChangeLog()
        {
            Application.Run(new SRB2Updater.Changelog());
        }

        private void update_Load(object sender, EventArgs e)
        {
            thrSplash = new Thread(new ThreadStart(ShowSplash));
            thrSplash.Start();
            if (File.Exists("srb2update.bat"))
                File.Delete("srb2update.bat");
            updateList();
        }

        private void On_Close(object sender, EventArgs e)
        {
            Environment.Exit(0);
        }

        private void update_Reload(object sender, EventArgs e)
        {
            updateList();
        }

        private void lblProgress_Click(object sender, EventArgs e)
        {

        }

        private void btnMinimize_Click(object sender, EventArgs e)
        {
            this.WindowState = FormWindowState.Minimized;
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            Application.Exit();
        }

        private void label1_Click(object sender, EventArgs e)
        {
            thrChangeLog = new Thread(new ThreadStart(ShowChangeLog));
            thrChangeLog.SetApartmentState(ApartmentState.STA);
            thrChangeLog.Start();
        }
    }
}
