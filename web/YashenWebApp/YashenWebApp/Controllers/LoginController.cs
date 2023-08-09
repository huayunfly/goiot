using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text.Json;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using YashenWebApp.Models;
using YashenWebApp.Services;

// For more information on enabling Web API for empty projects, visit https://go.microsoft.com/fwlink/?LinkID=397860

namespace YashenWebApp.Controllers
{
    [Produces("application/json")]
    [Route("api/[controller]")]
    [ApiController]
    public class LoginController : ControllerBase
    {
        private readonly IUserService userService_;
        private readonly IConfiguration configuration_;
        public LoginController(IUserService userService, IConfiguration configuration)
        {
            userService_ = userService;
            configuration_ = configuration;
        }

        // GET api/<LoginController>/5
        [HttpGet("{id}")]
        public string Get(int id)
        {
            if (id == 1)
            {
                var obj = new
                {
                    message = id
                };
                return JsonSerializer.Serialize(obj); // Create escaped string.
            }
            return "value";
        }

        // POST api/<LoginController>
        //[HttpPost]
        //[Consumes("application/json")] // 默认情况下，操作支持所有可用的请求内容类型。此为限定
        //public IActionResult PostJson(string value)
        //{
        //    return RedirectToPage("Author");
        //}

        // POST api/<LoginController>
        [HttpPost]
        [Consumes("application/x-www-form-urlencoded")]
        public async Task<ActionResult<string>> PostForm([FromForm] LoginInfo info)
        {
            if (info != null)
            {
                string token = await userService_.LoginAsync(info, configuration_);
                if (!string.IsNullOrWhiteSpace(token))
                {
                    var obj = new
                    {
                        message = $"Api post {nameof(PostForm)} ok",
                        result = new { token },
                        statusCode = "200"
                    };
                    //string content = JsonSerializer.Serialize(obj); // Create escaped string.
                    return Ok(obj);
                }
                else
                {
                    var obj = new
                    {
                        message = $"Api post {nameof(PostForm)} failed",
                        error = "Username or password error",
                        statusCode = "404"
                    };
                    return NotFound(obj);
                }
            }
            else
            {
                var obj = new
                {
                    message = $"Api post {nameof(PostForm)} failed",
                    error = "Username or password error",
                    statusCode = "400"
                };
                return BadRequest(obj);
            }
        }
    }
}
