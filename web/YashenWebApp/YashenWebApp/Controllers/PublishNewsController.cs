using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.FlowAnalysis.DataFlow;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Primitives;
using System;
using System.IO;
using System.Linq;
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
        private readonly IPublishNewsService publishNewsService_;
        private readonly IConfiguration configuration_;

        public PublishNewsController(IPublishNewsService publishNewsService, IConfiguration configuration) 
        {
            if (publishNewsService == null)
            {
                throw new ArgumentNullException(nameof(publishNewsService));
            }
            publishNewsService_ = publishNewsService;
            configuration_ = configuration;
        }
        //// GET: PublishNewsController
        //public ActionResult Index()
        //{ 
        //    return View();
        //}

        //// GET: PublishNewsController/Details/5
        //public ActionResult Details(int id)
        //{
        //    return View();
        //}

        //// GET: PublishNewsController/Create
        //public ActionResult Create()
        //{
        //    return View();
        //}

        // POST: PublishNewsController/Create
        [HttpPost]
        [Route("create")]
        [Consumes("multipart/form-data")]
        async public Task<ActionResult> Create(IFormCollection collection)
        {
            try
            {
                string title = string.Empty;
                string content = string.Empty;
                DateTime publish_date = DateTime.Today;
                StringValues values;
                if (collection.TryGetValue("title", out values))
                { 
                    title = values.ToString();
                }
                if (collection.TryGetValue("content", out values))
                {
                    content = values.ToString();
                }
                if (collection.TryGetValue("publishdate", out values))
                {
                    DateTime.TryParse(values.ToString(), out publish_date);
                }
                // Validate
                if (string.IsNullOrEmpty(title) || string.IsNullOrEmpty(content)) 
                {
                    var error_obj = new
                    {
                        message = $"Api post {nameof(Create)} failed",
                        error = "Null or empty fields",
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
                content = content.Replace("\r\n", "@@"); // Return characters protocal the same with UI
                string author = "a@acom";
                var formfile = collection.Files.GetFile("imagefile");
                if (formfile != null && (formfile.Length == 0 || formfile.Length > 1048576))
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
                    string file_path = Path.Combine(configuration_["StoredFilesPath"], "img", picture_name);
                    using (var stream = System.IO.File.Create(file_path))
                    {
                        await formfile.CopyToAsync(stream);
                    }
                }
                // to DB
                NewsItem item = new NewsItem(Guid.NewGuid(), title, content,
                    publish_date.ToUniversalTime(), picture_path, author, false, DateTime.Now.ToUniversalTime());
                await publishNewsService_.AddAsync(item);
                var obj = new
                {
                    message = $"Api post {nameof(Create)} ok",
                    result = new { token = "adfafad89j&jafjasjf[map*ladfjlj" },
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

        //// GET: PublishNewsController/Delete/5
        //public ActionResult Delete(int id)
        //{
        //    return View();
        //}

        //// POST: PublishNewsController/Delete/5
        //[HttpPost]
        //[ValidateAntiForgeryToken]
        //public ActionResult Delete(int id, IFormCollection collection)
        //{
        //    try
        //    {
        //        return RedirectToAction(nameof(Index));
        //    }
        //    catch
        //    {
        //        return View();
        //    }
        //}
    }
}
