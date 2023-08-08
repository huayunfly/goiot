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
    public class ApiResultBody
    {
        public string Message { get; set; }
        public Dictionary<string, object> Result { get; set; }
        public string StatusCode { get; set; }
    }

    /// <summary>
    /// API request common body.
    /// </summary>
    public class ApiRequestBody
    {
        public string Name { get; set; }
        public string Token { get; set; }
        public string Operation { get; set; }
    }

    /// <summary>
    /// Get data request body.
    /// </summary>
    public class ApiGetDataBody : ApiRequestBody
    {
        public Dictionary<string, object> Condition { get; set; }
    }

    /// <summary>
    /// Set data request body.
    /// </summary>
    public class ApiSetDataBody : ApiRequestBody
    {
        public Dictionary<string, object> Data { get; set; }
    }

    public class UserService : IUserService
    {
        public async Task<string> LoginAsync(LoginInfo login)
        {
            string url = "http://127.0.0.1:6301/api";
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
            var cts = new CancellationTokenSource(5000);
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
    }
}
