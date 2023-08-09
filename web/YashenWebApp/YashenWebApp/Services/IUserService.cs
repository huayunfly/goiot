using Microsoft.Extensions.Configuration;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using YashenWebApp.Models;

namespace YashenWebApp.Services
{
    public interface IUserService
    {
        /// <summary>
        /// Login
        /// </summary>
        /// <param name="login">Login information</param>
        /// <param name="configuration">IConfiguration</param>
        /// <returns>null if it failed, otherwise a token.</returns>
        Task<string> LoginAsync(LoginInfo login, IConfiguration configuration);

        /// <summary>
        /// Validate token.
        /// </summary>
        /// <param name="token">Token</param>
        /// <param name="configuration">IConfiguration</param>
        /// <returns>null if it failed, otherwise the username</returns>
        Task<string> ValidateAsync(string token, IConfiguration configuration);
    }
}
