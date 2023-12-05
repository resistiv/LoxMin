using System;
using System.Diagnostics;
using System.Text;

namespace LoxTester
{
    public static class Tools
    {
        private static StringBuilder _output;

        /// <summary>
        /// Runs a file within LoxMin, reads the output into an array, and prints the output.
        /// </summary>
        /// <param name="filename">The name of the file to run.</param>
        /// <returns>Output of the script, split at newlines.</returns>
        public static string[] RunFile(string filename)
        {
            // Initialize string storage
            _output = new();

            // Initialize a process
            Process proc = new();
            proc.StartInfo.FileName = $"../../../../../LoxMin/LoxMin{(OperatingSystem.IsWindows() ? ".exe" : "")}";
            proc.StartInfo.Arguments = $"{filename} -q";
            proc.StartInfo.UseShellExecute = false;
            proc.StartInfo.RedirectStandardOutput = true;
            proc.StartInfo.RedirectStandardError = true;
            proc.StartInfo.StandardOutputEncoding = Encoding.UTF8;
            proc.StartInfo.StandardErrorEncoding = Encoding.UTF8;

            // Set output handler
            proc.OutputDataReceived += new DataReceivedEventHandler(ConsoleOutputHandler);
            proc.ErrorDataReceived += new DataReceivedEventHandler(ConsoleOutputHandler);

            // Start process and begin listening
            proc.Start();
            proc.BeginOutputReadLine();
            proc.BeginErrorReadLine();
            proc.WaitForExit();

            // Extract output and return
            Console.WriteLine(_output.ToString());
            string[] outputStr = _output.ToString().Trim().Split(new string[] { "\r\n", "\n" }, StringSplitOptions.None);
            _output = null;
            return outputStr;
        }

        /// <summary>
        /// Handles console output of a process.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public static void ConsoleOutputHandler(object sender, DataReceivedEventArgs args)
        {
            _output.Append(args.Data);
            if (args.Data != null && !string.IsNullOrEmpty(args.Data))
            {
                _output.Append(Environment.NewLine);
            }
        }
    }
}
