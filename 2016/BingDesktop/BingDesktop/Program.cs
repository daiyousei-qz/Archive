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

        static readonly string pathBase        = Environment.GetFolderPath(Environment.SpecialFolder.MyPictures);
        static readonly string date            = DateTime.Now.ToString("yyyy-MM-dd");
        static readonly string time            = DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss");
        static readonly string pathImg         = $"{pathBase}\\Bing\\{date}.jpg";
        static readonly string pathLog         = $"{pathBase}\\Bing\\log.txt";
        static readonly string pathErroredPage = $"{pathBase}\\Bing\\error-{date}.html";

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
                    // note different network environment will yield different kinds of address
                    // this depends on http:\\cn.bing.com
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
            // if todays wallpaper already in archive, do nothing
            if (File.Exists(pathImg)) return;

            StreamWriter log = File.AppendText(pathLog);
            log.WriteLine("[{0}]", DateTime.Now);

            try
            {
                // ping first to check availability of Bing 
                Ping ping = new Ping();
                PingReply reply = ping.Send("cn.bing.com");

                DownloadWallpaper(log);
            }
            catch (PingException)
            {
                log.WriteLine("Failed to reach cn.bing.com");
            }
            catch (AggregateException ex)
            {
                log.WriteLine("Exception thrown, saying \"{0}\"", ex.InnerException.Message);
            }
            catch (Exception ex)
            {
                log.WriteLine("Exception thrown, saying \"{0}\"", ex.Message);
            }

            log.WriteLine();
            log.Close();
        }
    }
}
