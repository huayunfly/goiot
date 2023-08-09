using System.Collections.Generic;

namespace YashenWebApp.Services
{
    /// <summary>
    /// API result body.
    /// </summary>
    public class ApiResultBody
    {
        public string Message { get; set; }
        public Dictionary<string, object> Result { get; set; }
        public string StatusCode { get; set; }
        public string Error { get; set; }
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
}
