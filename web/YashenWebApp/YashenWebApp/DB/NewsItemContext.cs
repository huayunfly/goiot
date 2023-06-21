using Microsoft.EntityFrameworkCore;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using YashenWebApp.Models;
using YashenWebApp.Services;

namespace YashenWebApp.DB
{

    public class NewsItemContext : DbContext, IPublishNewsService
    {
        #region Fields

        private const string ConnectionString = @"Host=127.0.0.1;Username=postgres;Password=hello@123;Database=yashenwebapp";

        public DbSet<NewsItem> NewsList => Set<NewsItem>();

        #endregion

        public NewsItemContext(DbContextOptions<NewsItemContext> options) :
            base(options)
        {
            ChangeTracker.QueryTrackingBehavior = QueryTrackingBehavior.NoTracking;
        }

        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            modelBuilder.Entity<NewsItem>().Property(b => b.Title).HasMaxLength(120);
        }

        public async Task AddAsync(NewsItem item)
        {
            await NewsList.AddAsync(item);
            await SaveChangesAsync();
        }

        public async Task AddRangeAsync(IEnumerable<NewsItem> items)
        {
            await NewsList.AddRangeAsync(items);
            await SaveChangesAsync();
        }

        public async Task<IEnumerable<NewsItem>> GetAllAsync()
        {
            var items = await NewsList.ToListAsync();
            return items;
        }

        public async Task<NewsItem> FindAsync(Guid id)
        {
            var item = await NewsList.FindAsync(id);
            return item;
        }

        public async Task<NewsItem> RemoveAsync(Guid id)
        {
            var item = await NewsList.FindAsync(id);
            NewsList.Remove(item);
            await SaveChangesAsync();
            return item;
        }

        public async Task<NewsItem> UpdateAsync(NewsItem item)
        {
            NewsList.Update(item);
            await SaveChangesAsync();
            return item;
        }
    }
}
