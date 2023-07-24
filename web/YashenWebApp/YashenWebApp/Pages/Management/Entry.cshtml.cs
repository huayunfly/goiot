using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using NuGet.Protocol;
using System;

namespace YashenWebApp.Pages.Management
{
    public class EntryModel : PageModel
    {
        public IActionResult OnGet([FromRoute] string token = null)
        {
            ViewData["date"] = DateTime.Now.ToString("yyyy-MM-dd");
            if (Validate(token))
            {
                ViewData["token"] = token;
                return Page();
            }
            else
            {
                return RedirectToPage("Index");
            }
        }

        /// <summary>
        /// Validate token by service.
        /// </summary>
        /// <param name="token">User token</param>
        /// <returns>True if it is succeeded.</returns>
        private bool Validate(string token)
        {
            if (string.IsNullOrWhiteSpace(token))
            {
                return false;
            }
            return true;
        }
    }
}
