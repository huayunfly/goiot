using System;
using System.ComponentModel.DataAnnotations;

namespace YashenWebApp.Models
{
#if USERECORDS

    public record NewsItem(
Guid Id,
[property: Required, MaxLength(140)] string Title,
[property: Required, MaxLength(1024)] string Content,
[property: Required] DateTime PublishDate,
[property: Required, MaxLength(140)] string PicturePath,
[property: Required, MaxLength(60)] string Author,
bool Deleted,
DateTime CreateTime
        );
#else

    public class NewsItem
    {
        public NewsItem(Guid id, string title, string content, DateTime publishDate, string picturePath, string author, bool deleted, DateTime createTime)
        {
            Id = id;
            Title = title;
            Content = content;
            PublishDate = publishDate;
            PicturePath = picturePath;
            Author = author;
            Deleted = deleted;
            CreateTime = createTime;
        }

        public Guid Id { get; set; }  // Id or <type name>Id will be configured as the primary key of an entity.

        [Required]
        [MaxLength(140)]
        public string Title { get; set; }

        [Required]
        [MaxLength(1024)]
        public string Content { get; set; }

        [Required]
        public DateTime PublishDate { get; set; }

        [Required]
        [MaxLength(140)]
        public string PicturePath { get; set; }

        [Required]
        [MaxLength(60)]
        public string Author { get; set; }

        [Required]
        public bool Deleted { get; set; }

        [Required]
        public DateTime CreateTime { get; set; }
    }
#endif
}
