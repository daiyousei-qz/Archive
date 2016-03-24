using System;
using System.Drawing;
using System.IO;
using System.Net.Http;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace BingDesktop
{
    static class Program
    {
        public const int SPI_SETDESKWALLPAPER = 20;
        public const int SPIF_UPDATEINIFILE = 1;
        public const int SPIF_SENDCHANGE = 2;

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern int SystemParametersInfo(
          int uAction, int uParam, string lpvParam, int fuWinIni);

        static void SetWallpaper(string path)
        {
            SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, path, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        }

        static readonly string pathBase = Environment.GetFolderPath(Environment.SpecialFolder.MyPictures);
        static readonly string date = DateTime.Now.ToString("yyyy-MM-dd");
        static readonly string time = DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss");
        static readonly string pathImg = string.Format("{0}\\Bing\\{1}.jpg", pathBase, date);
        static readonly string pathLog = string.Format("{0}\\Bing\\log.txt", pathBase);
        static readonly string pathErroredPage = string.Format("{0}\\Bing\\error-{1}.html", pathBase, date);

        static readonly string pattern = "(?<=;g_img={url:')(http:\\/\\/\\w+(\\.\\w+)*)?(\\/[\\w0-9_\\-]+)*\\.jpg(?=',id)";

        static void DownloadWallpaper(StreamWriter log)
        {
            using (HttpClient client = new HttpClient())
            {
                log.WriteLine("Connecting to cn.bing.com");
                var taskPage = client.GetStringAsync("http://cn.bing.com/");
                Task.WaitAll(taskPage);

                var match = Regex.Match(taskPage.Result, pattern);
                if (match.Success)
                {
                    // note different network environment will acquire different kinds of address
                    var addr = (match.Value.StartsWith("http:") ? "" : "http://cn.bing.com") + match.Value;
                    log.WriteLine("Find today's wall paper at \"{0}\"", addr);

                    var taskImg = client.GetByteArrayAsync(addr);
                    Task.WaitAll(taskImg);
                    Image img = Image.FromStream(new MemoryStream(taskImg.Result));

                    img.Save(pathImg);
                    SetWallpaper(pathImg);
                    log.WriteLine("Wallpaper successfully set.");
                }
                else
                {
                    log.WriteLine("Regex match failed.");

                    File.WriteAllText(pathErroredPage, taskPage.Result);
                    log.WriteLine("Invalid page data saved at \"{0}\"", pathErroredPage);
                }
            }
        }

        [STAThread]
        static void Main()
        {
            StreamWriter log = File.AppendText(pathLog);
            log.WriteLine("[{0}]", DateTime.Now);

            try
            {
                if (!File.Exists(pathImg))
                {
                    // ping first to check availability of Bing 
                    Ping ping = new Ping();
                    PingReply reply = ping.Send("cn.bing.com");

                    DownloadWallpaper(log);
                }
                else
                {
                    log.WriteLine("Today's wallpaper already set.");
                }
            }
            catch(PingException)
            {
                log.WriteLine("Failed to reach cn.bing.com");
            }
            catch(Exception ex)
            {
                log.WriteLine("Unknown exception saying \"{0}\"", ex.Message);
            }

            log.WriteLine();
            log.Close();
        }
    }
}
