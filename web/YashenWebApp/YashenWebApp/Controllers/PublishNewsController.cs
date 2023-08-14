using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Primitives;
using System;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Processing;
using System.IO;
using System.Threading.Tasks;
using YashenWebApp.Models;
using YashenWebApp.Services;

namespace YashenWebApp.Controllers
{
    [Produces("application/json")]
    [Route("api/[controller]")]
    [ApiController]
    public class PublishNewsController : Controller
    {
        private readonly IPublishNewsService _publishNewsService;
        private readonly IConfiguration _configuration;
        private readonly IUserService _userService;

        public PublishNewsController(IPublishNewsService publishNewsService, 
            IConfiguration configuration, IUserService userService) 
        {
            if (publishNewsService == null || configuration == null ||
                userService == null)
            {
                throw new ArgumentNullException(nameof(PublishNewsController));
            }
            _publishNewsService = publishNewsService;
            _configuration = configuration;
            _userService = userService;
        }

        // POST: PublishNewsController/Create
        [HttpPost]
        [Route("create")]
        [Consumes("multipart/form-data")]
        async public Task<ActionResult> Create(IFormCollection collection)
        {
            try
            {
                string token = string.Empty;
                string title = string.Empty;
                string content = string.Empty;
                DateTime publish_date = DateTime.Today;
                StringValues values;
                if (collection.TryGetValue("token", out values))
                {
                    token = values.ToString();
                }
                if (collection.TryGetValue("title", out values))
                { 
                    title = values.ToString();
                }
                if (collection.TryGetValue("content", out values))
                {
                    content = values.ToString();
                }
                if (collection.TryGetValue("publishDate", out values))
                {
                    DateTime.TryParse(values.ToString(), out publish_date);
                }
                // Validate
                if (string.IsNullOrWhiteSpace(token) || string.IsNullOrWhiteSpace(title) || 
                    string.IsNullOrWhiteSpace(content)) 
                {
                    var error_obj = new
                    {
                        message = $"Api post {nameof(Create)} failed",
                        error = "Empty fields",
                        statusCode = "400"
                    };
                    return BadRequest(error_obj);
                }

                string username = await _userService.ValidateAsync(token, _configuration);
                if (string.IsNullOrWhiteSpace(username))
                {
                    var error_obj = new
                    {
                        message = $"Api post {nameof(Create)} failed",
                        error = "Error token",
                        statusCode = "400"
                    };
                    return BadRequest(error_obj);
                }

                if (title.Length > 140 || content.Length > 1024 || publish_date.Date > DateTime.Today ||
                    publish_date.Date < (DateTime.Today.AddDays(-180))) 
                {
                    var error_obj = new
                    {
                        message = $"Api post {nameof(Create)} failed",
                        error = "Field length is out of limit",
                        statusCode = "400"
                    };
                    return BadRequest(error_obj);
                }

                // Replace \r\n with @@ in the content with the same UI protocal.
                content = content.Replace("\r\n", "@@");

                // Get and resize image.
                var formfile = collection.Files.GetFile("imageFile");
                if (formfile == null || 
                    (formfile.Length == 0 || formfile.Length > 1048576) ||
                    (!formfile.ContentType.Equals("image/jpeg") && !formfile.ContentType.Equals("image/png"))
                    )
                {
                    var error_obj = new
                    {
                        message = $"Api post {nameof(Create)} failed",
                        error = "Image size exceeds 1M",
                        statusCode = "400"
                    };
                    return BadRequest(error_obj);
                }

                string picture_path = string.Empty;
                if (formfile != null)
                {
                    Random rnd = new Random();
                    string picture_name = $"news_{publish_date.ToString("yyyyMMdd")}_{rnd.Next(32768)}.jpg";
                    picture_path = Path.Combine("\\img", picture_name); // Store an absolute webapp path
                    string file_path = Path.Combine(_configuration["StoredFilesPath"], "img", picture_name);
                    using var img = await Image.LoadAsync(formfile.OpenReadStream());
                    img.Mutate(x => x.Resize(225, 300));
                    img.Save(file_path);
                }
                // to DB
                NewsItem item = new NewsItem(Guid.NewGuid(), title, content,
                    publish_date.ToUniversalTime(), picture_path, username, false, DateTime.Now.ToUniversalTime());
                await _publishNewsService.AddAsync(item);
                var obj = new
                {
                    message = $"Api post {nameof(Create)} ok",
                    result = new { username },
                    statusCode = "200"
                };
                return Ok(obj);
            }
            catch(Exception ex)
            {
                var error_obj = new
                {
                    message = $"Api post {nameof(Create)} failed",
                    error = $"Internal error: {ex.Message}",
                    statusCode = "400"
                };
                return BadRequest(error_obj);
            }
        }

        //// GET: PublishNewsController/Edit/5
        //public ActionResult Edit(int id)
        //{
        //    return View();
        //}

        // POST: PublishNewsController/Edit/5
        [HttpPost]
        //[ValidateAntiForgeryToken]
        [Route("edit")]
        [Consumes("application/x-www-form-urlencoded")]
        public ActionResult Edit(int id, IFormCollection collection)
        {
            try
            {
                return RedirectToAction(nameof(Index));
            }
            catch
            {
                return View();
            }
        }
    }
}
