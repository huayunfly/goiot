using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using YashenWebApp.Models;
using YashenWebApp.Services;

namespace YashenWebApp.Pages
{
    public class NewsModel : PageModel
    {
        #region Fields

        private readonly IPublishNewsService publishNewsService_;

        private IEnumerable<NewsItem> newsItems_;

        #endregion

        public NewsModel(IPublishNewsService publishNewsService)
        {
            if (publishNewsService == null)
            {
                throw new ArgumentNullException(nameof(publishNewsService));
            }
            publishNewsService_ = publishNewsService;
        }

        #region Properties

        public IEnumerable<NewsItem> NewsItems
        {
            get => newsItems_;
        }

        #endregion

        public async Task OnGet()
        {
            newsItems_ = await publishNewsService_.GetAllAsync(); 
        }

        public string GetLastUpdateInfo(DateTime publishDate)
        {
            var span = (DateTime.Now - publishDate);
            if (span.TotalDays > 30)
            {
                return $"������ {Math.Round(span.TotalDays / 30)} ��ǰ";
            }
            else
            {
                return $"������ {Convert.ToInt32(span.TotalDays)} ��ǰ";
            }
        }
    }
}
