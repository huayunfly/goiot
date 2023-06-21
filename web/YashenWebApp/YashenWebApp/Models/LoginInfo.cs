using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace YashenWebApp.Models
{
    using System.ComponentModel.DataAnnotations;
    public class LoginInfo
    {
        [Required]
        [MaxLength(100)]
        public string Username { get; set; }

        [Required]
        [MaxLength(140)]
        public string Password { get; set; }
    }
}
