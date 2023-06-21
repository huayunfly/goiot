using System.Collections.Generic;
using System.Threading.Tasks;
using System;
using YashenWebApp.Models;

namespace YashenWebApp.Services
{
    public interface IPublishNewsService
    {
        Task AddAsync(NewsItem chapter);
        Task AddRangeAsync(IEnumerable<NewsItem> chapters);
        Task<IEnumerable<NewsItem>> GetAllAsync();
        Task<NewsItem?> FindAsync(Guid id);
        Task<NewsItem?> RemoveAsync(Guid id);
        Task<NewsItem?> UpdateAsync(NewsItem chapter);
    }
}
