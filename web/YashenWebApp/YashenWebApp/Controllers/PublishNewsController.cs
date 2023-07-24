using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using System;
using YashenWebApp.Services;

namespace YashenWebApp.Controllers
{
    [Produces("application/json")]
    [Route("api/[controller]")]
    [ApiController]
    public class PublishNewsController : Controller
    {
        private readonly IPublishNewsService publishNewsService_;

        public PublishNewsController(IPublishNewsService publishNewsService) 
        {
            if (publishNewsService == null)
            {
                throw new ArgumentNullException(nameof(publishNewsService));
            }
            publishNewsService_ = publishNewsService;
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
        //[ValidateAntiForgeryToken]
        [Consumes("multipart/form-data")]
        public ActionResult Create(IFormCollection collection)
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
