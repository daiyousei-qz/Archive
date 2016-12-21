using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    class Program
    {
        static void Main(string[] args)
        {
            var reader = new StreamReader(@"D:\test.txt");
            var lexer = new CSLexer(reader);
            var analyzer = new CSAnalyzer();

            var stmt = analyzer.Analyze(lexer);
            var emitter = new SSMAEmitter();

            stmt.Expand(emitter);
            var ssma = emitter.ToString();
            File.WriteAllText(@"D:\ssma.txt", ssma);
            Console.ReadKey();
        }
    }
}