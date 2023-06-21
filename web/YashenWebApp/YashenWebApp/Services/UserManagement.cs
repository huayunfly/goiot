using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace YashenWebApp.Services
{
    internal class UserManagement
    {
        private readonly IUserService userService_;
        public UserManagement(IUserService userService)
        {
            userService_ = userService;
        }
    }
}
