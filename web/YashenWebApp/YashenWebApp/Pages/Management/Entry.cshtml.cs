using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Microsoft.Extensions.Configuration;
using System;
using System.Configuration;
using System.Threading.Tasks;
using YashenWebApp.Services;

namespace YashenWebApp.Pages.Management
{
    public class EntryModel : PageModel
    {
        private readonly IUserService userService_;
        private readonly IConfiguration configuration_;

        public EntryModel(IConfiguration configuration, IUserService userService)
        {
            configuration_ = configuration;
            userService_ = userService;
        }

        public async Task<IActionResult> OnGet([FromRoute] string token = null)
        {
            ViewData["date"] = DateTime.Now.ToString("yyyy-MM-dd");
            string username = await userService_.ValidateAsync(token, configuration_);
            if (!string.IsNullOrWhiteSpace(username))
            {
                ViewData["token"] = token;
                ViewData["user"] = username;
                return Page();
            }
            else
            {
                return RedirectToPage("Index");
            }
        }
    }
}
