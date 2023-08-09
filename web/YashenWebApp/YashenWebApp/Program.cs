using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using YashenWebApp.DB;
using YashenWebApp.Services;

namespace YashenWebApp
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var host = CreateHostBuilder(args).Build();
            PrepareDB(host);
            host.Run();
        }

        private static void PrepareDB(IHost host)
        {
            var scope = host.Services.CreateScope();
            var newsContext = scope.ServiceProvider.GetRequiredService<NewsItemContext>();
            newsContext.Database.EnsureCreated();

            // Mockup data
            //var sampleData = scope.ServiceProvider.GetRequiredService<SampleNews>();
            //sampleData.CreateSampleNews();
        }

        public static IHostBuilder CreateHostBuilder(string[] args) =>
            Host.CreateDefaultBuilder(args)
                .ConfigureWebHostDefaults(webBuilder =>
                {
                    webBuilder.UseStartup<Startup>();
                });
    }
}
