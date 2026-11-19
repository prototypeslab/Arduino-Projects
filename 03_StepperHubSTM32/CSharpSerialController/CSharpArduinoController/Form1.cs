using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;

namespace ComPort
{
    public partial class Form1 : Form
    {
        string dataOUT;
        string sendWith;
        string dataIN;

        public Form1()
        {
            InitializeComponent();
        }

 
        private void Form1_Load(object sender, EventArgs e)
        {
            string[] ports = SerialPort.GetPortNames();
            cBoxCOMPORT.Items.AddRange(ports);

            btnOpen.Enabled = true;
            btnClose.Enabled = false;

            chBoxDtrEnable.Checked = false;
            serialPort1.DtrEnable = false;
            chBoxRTSEnable.Checked = false;
            serialPort1.RtsEnable = false;
            btnSendData.Enabled = false;
            chBoxWriteLine.Checked = false;
            chBoxWrite.Checked = true;
            sendWith = "Write";

            chBoxAddToOldData.Checked = true;
            chBoxAlwaysUpdate.Checked = false;
        }

        private void btnOpen_Click(object sender, EventArgs e)
        {
            try
            {
                serialPort1.PortName = cBoxCOMPORT.Text;
                serialPort1.BaudRate = Convert.ToInt32(CBoxBaudRate.Text);
                serialPort1.DataBits = Convert.ToInt32(cBoxDataBits.Text);
                serialPort1.StopBits = (StopBits)Enum.Parse(typeof(StopBits), cBoxStopBits.Text);
                serialPort1.Parity = (Parity)Enum.Parse(typeof(Parity), cBoxParityBits.Text);

                serialPort1.Open();
                progressBar1.Value = 100;
                btnOpen.Enabled = false;
                btnClose.Enabled = true;
                lblStatusCom.Text = "ON";
                
            }

            catch (Exception err)
            {
                MessageBox.Show(err.Message,"Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                btnOpen.Enabled = true;
                btnClose.Enabled = false;
                lblStatusCom.Text = "OFF";
            }
        

        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            if(serialPort1.IsOpen)
            {
                serialPort1.Close();
                progressBar1.Value = 0;
                btnOpen.Enabled = true;
                btnClose.Enabled = false;
                lblStatusCom.Text = "OFF";
            }
        }

        private void btnSendData_Click(object sender, EventArgs e)
        {
            if(serialPort1.IsOpen)
            {
                dataOUT = tBoxDataOut.Text;
                if (sendWith == "WriteLine")
                {
                    serialPort1.WriteLine(dataOUT);
                }
                else if(sendWith=="Write")
                {
                    serialPort1.Write(dataOUT);
                }
                
            }
        }

        private void chBoxDtrEnable_CheckedChanged(object sender, EventArgs e)
        {
            if(chBoxDtrEnable.Checked)
            {
                serialPort1.DtrEnable = true;
                MessageBox.Show("DTR Enable", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else { serialPort1.DtrEnable = false; }
        }

        private void chBoxRTSEnable_CheckedChanged(object sender, EventArgs e)
        {
            if(chBoxRTSEnable.Checked)
            {
                serialPort1.RtsEnable = true;
                MessageBox.Show("RTS Enable", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else { serialPort1.RtsEnable = false; }
        }

        private void btnClearDataOut_Click(object sender, EventArgs e)
        {
            if(tBoxDataOut.Text != "")
            {
                tBoxDataOut.Text = "";
            }
        }

        private void tBoxDataOut_TextChanged(object sender, EventArgs e)
        {
            int dataOUTLength = tBoxDataOut.TextLength;
            lblDataOutLength.Text = string.Format("{0:00}", dataOUTLength);
            if(chBoxUsingEnter.Checked)
            {
                tBoxDataOut.Text = tBoxDataOut.Text.Replace(Environment.NewLine, "");
            }
        }

        private void chBoxUsingButton_CheckedChanged(object sender, EventArgs e)
        {
            if(chBoxUsingButton.Checked)
            {
                btnSendData.Enabled = true;
            }
            else { btnSendData.Enabled = false; }
        }

        private void tBoxDataOut_KeyDown(object sender, KeyEventArgs e)
        {
            if(chBoxUsingEnter.Checked)
            {
                if(e.KeyCode == Keys.Enter)
                {
                    if (serialPort1.IsOpen)
                    {
                        dataOUT = tBoxDataOut.Text;
                        if (sendWith == "WriteLine")
                        {
                            serialPort1.WriteLine(dataOUT);
                        }
                        else if (sendWith == "Write")
                        {
                            serialPort1.Write(dataOUT);
                        }
                    }
                }
            }
        }

        private void chBoxWriteLine_CheckedChanged(object sender, EventArgs e)
        {
            if(chBoxWriteLine.Checked)
            {
                sendWith = "WriteLine";
                chBoxWrite.Checked = false;
                chBoxWriteLine.Checked = true;
            }
            
        }

        private void chBoxWrite_CheckedChanged(object sender, EventArgs e)
        {
            if(chBoxWrite.Checked)
            {
                sendWith = "Write";
                chBoxWrite.Checked = true;
                chBoxWriteLine.Checked = false; 
            }
        }

        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            dataIN = serialPort1.ReadExisting();
            this.Invoke(new EventHandler(ShowData));
        }

        private void ShowData(object sender, EventArgs e)
        {
            int dataINLength = dataIN.Length;
            lblDataInLength.Text = string.Format("{0:00}", dataINLength);

            if(chBoxAlwaysUpdate.Checked)
            {
                tBoxDataIN.Text = dataIN;
            }
            else if(chBoxAddToOldData.Checked)
            {
                tBoxDataIN.Text += dataIN;
            }
        }

        private void chBoxAlwaysUpdate_CheckedChanged(object sender, EventArgs e)
        {
            if(chBoxAlwaysUpdate.Checked)
            {
                chBoxAlwaysUpdate.Checked = true;
                chBoxAddToOldData.Checked = false;
            }
            else { chBoxAddToOldData.Checked = true; }
        }

        private void chBoxAddToOldData_CheckedChanged(object sender, EventArgs e)
        {
            if(chBoxAddToOldData.Checked)
            {
                chBoxAlwaysUpdate.Checked = false;
                chBoxAddToOldData.Checked = true;
            }
            else { chBoxAlwaysUpdate.Checked = true; }
        }

        private void btnClearDataIN_Click(object sender, EventArgs e)
        {
            if(tBoxDataIN.Text != "")
            {
                tBoxDataIN.Text = "";
            }
        }

        private void cbCommand_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cbCommand.Text == "ACCF") txtCommand.Text = "Set the maximum frequency for an axis (Hz)";
            if (cbCommand.Text == "ACCI") txtCommand.Text = "Set the frequency increment (Hz/step) for an axis";
            if (cbCommand.Text == "ACCS") txtCommand.Text = "Set the initial frequency for an axis (Hz)";
            if (cbCommand.Text == "AMOV") txtCommand.Text = "Move to a position (steps)";
            if (cbCommand.Text == "BAUD") txtCommand.Text = "Set the communications baud rate (bps)";
            if (cbCommand.Text == "DROF") txtCommand.Text = "Immediately switch a direction output off";
            if (cbCommand.Text == "DRON") txtCommand.Text = "Set a direction output on indefinitely or for a period (tenths of seconds)";
            if (cbCommand.Text == "DRST") txtCommand.Text = "Report the current status of a direction output";
            if (cbCommand.Text == "OPTN") txtCommand.Text = "Set checksum mode and configure how the controller responds to axis move commands";
            if (cbCommand.Text == "POSN") txtCommand.Text = "Set the current position of an axis";
            if (cbCommand.Text == "PSTT") txtCommand.Text = "Report the current position of all axes";
            if (cbCommand.Text == "RACC") txtCommand.Text = "Report the initial frequency, frequency increment and maximum frequency of an axis";
            if (cbCommand.Text == "RDAN") txtCommand.Text = "Report analog voltage at AN1, AN2, IO1, IO2 or VS";
            if (cbCommand.Text == "RDIO") txtCommand.Text = "Report the status of AN1, AN2, IO1 or IO2 as a digital input";
            if (cbCommand.Text == "REL1") txtCommand.Text = "Set relay 1 on or off";
            if (cbCommand.Text == "REL2") txtCommand.Text = "Set relay 2 on or off";
            if (cbCommand.Text == "RMOV") txtCommand.Text = "Move forward or backwards some number of steps";
            if (cbCommand.Text == "RSET") txtCommand.Text = "Reset the controller and load settings from non-volatile memory";
            if (cbCommand.Text == "SAMV") txtCommand.Text = "Move an axis to a position with initial frequency, frequency increment and maximum ";
            if (cbCommand.Text == "SRMV") txtCommand.Text = "Move an axis forward or backward by a number of steps with initial frequency, frequency increment and maximum frequency specified in the command";
            if (cbCommand.Text == "STAT") txtCommand.Text = "Report the status of axis movement, direction output and limit switch inputs";
            if (cbCommand.Text == "STOP") txtCommand.Text = "Immediately stop all axes";
            if (cbCommand.Text == "WDIO") txtCommand.Text = "Set the digital outputs IO1 and IO2 on or off";
        }

        private void bExit_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void bAddAndSend_Click(object sender, EventArgs e)
        {
           
            string checkSumString = "";
            /*
           if (serialPort1.IsOpen)
           {
               for (int i = 1; i < 21; i++)
               {
                   checkSumString = "9999" ;
                   dataOUT = "^" + cbCommand.Text + "," + numericX.Value + ","
                   + numericY.Value + "," + numericZ.Value + "," + numericA.Value + "," + checkSumString + "$";

                   tBoxDataOut.Text = dataOUT + "\r\n";
                   serialPort1.Write(dataOUT);
                   int milliseconds = (int)numericDelay.Value;
                   Thread.Sleep(milliseconds);
               }
           }
           */


            int checkSum = 9999;
           
            if (cbCheckSum.Checked)
            {
                checkSumString = "" + checkSum;
            }
            else
            {
                checkSumString = "";
            }

            tBoxDataOut.Text = "^"+ cbCommand.Text+ "," + numericX.Value + ","
                + numericY.Value + "," + numericZ.Value + "," + numericA.Value+ "," + checkSumString+"$";

            if (serialPort1.IsOpen)
            {
                dataOUT = tBoxDataOut.Text;
                if (sendWith == "WriteLine")
                {
                    serialPort1.WriteLine(dataOUT);
                }
                else if (sendWith == "Write")
                {
                    serialPort1.Write(dataOUT);
                }

            }
            
        }
    }
}
