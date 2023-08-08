using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using YashenWebApp.Models;

namespace YashenWebApp.Services
{
    public interface IUserService
    {
        Task<string> LoginAsync(LoginInfo login); 
    }
}
