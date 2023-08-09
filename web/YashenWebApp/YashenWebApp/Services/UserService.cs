using Microsoft.Extensions.Configuration;
using NuGet.Protocol.Plugins;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Mime;
using System.Security.Policy;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;
using YashenWebApp.Common;
using YashenWebApp.Models;

namespace YashenWebApp.Services
{
    public class UserService : IUserService
    {
        private readonly Int32 TIMEOUT_IN_MS = 5000;
        public async Task<string> LoginAsync(LoginInfo login, IConfiguration configuration)
        {
            if (login == null || configuration == null) 
            { 
                return null; 
            }
            string url = configuration["ServiceApi"] ?? string.Empty;
            string contentType = "application/json";
            ApiGetDataBody dataBody = new()
            {
                Name = "tenant",
                Token = string.Empty,
                Operation = "Login",
                Condition = new Dictionary<string, object>() {
                { "username", login.Username }, { "password", login.Password } }
            };
            JsonSerializerOptions options = new(JsonSerializerDefaults.Web)
            {
                WriteIndented = true
            };
            string strContent = JsonSerializer.Serialize(dataBody, options);
            var cts = new CancellationTokenSource(TIMEOUT_IN_MS);
            try
            {
                Tuple<byte[], string, string, string> resPost = 
                    await WebSvcCaller.PostAsync(url, contentType, strContent, cts.Token).ConfigureAwait(false);
                if (resPost == null || resPost.Item1 == null)
                {
                    throw new ArgumentNullException(nameof(resPost));
                }
                string strResponse = System.Text.Encoding.UTF8.GetString(resPost.Item1);
                if (string.IsNullOrWhiteSpace(strResponse))
                {
                    throw new ArgumentNullException(nameof(strResponse));
                }
                ApiResultBody resBody = JsonSerializer.Deserialize<ApiResultBody>(strResponse, options);
                if (resBody == null)
                {
                    throw new ArgumentNullException(nameof(resBody));
                }
                object token = null;
                if (resBody.StatusCode != "200" || resBody.Result == null || 
                    !resBody.Result.TryGetValue("token", out token))
                {
                    return null;
                }
                return token?.ToString();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            return null;
        }

        public async Task<string> ValidateAsync(string token, IConfiguration configuration)
        {
            if (string.IsNullOrWhiteSpace(token))
            {
                return null;
            }
            string url = configuration["ServiceApi"] ?? string.Empty;
            string contentType = "application/json";
            ApiGetDataBody dataBody = new()
            {
                Name = "tenant",
                Token = token,
                Operation = "Touch"
            };
            JsonSerializerOptions options = new(JsonSerializerDefaults.Web)
            {
                WriteIndented = true
            };
            string strContent = JsonSerializer.Serialize(dataBody, options);
            var cts = new CancellationTokenSource(TIMEOUT_IN_MS);
            try
            {
                Tuple<byte[], string, string, string> resPost =
                    await WebSvcCaller.PostAsync(url, contentType, strContent, cts.Token).ConfigureAwait(false);
                if (resPost == null || resPost.Item1 == null)
                {
                    throw new ArgumentNullException(nameof(resPost));
                }
                string strResponse = System.Text.Encoding.UTF8.GetString(resPost.Item1);
                if (string.IsNullOrWhiteSpace(strResponse))
                {
                    throw new ArgumentNullException(nameof(strResponse));
                }
                ApiResultBody resBody = JsonSerializer.Deserialize<ApiResultBody>(strResponse, options);
                if (resBody == null)
                {
                    throw new ArgumentNullException(nameof(resBody));
                }
                object username = null;
                if (resBody.StatusCode != "200" || resBody.Result == null ||
                    !resBody.Result.TryGetValue("username", out username))
                {
                    return null;
                }
                return username?.ToString();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            return null;
        }
    }
}
