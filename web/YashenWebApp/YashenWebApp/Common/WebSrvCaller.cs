
/// <summary>
/// @Created Date: 2018-09-28
/// </summary>

namespace YashenWebApp.Common
{
    using System;
    using System.Xml;
    using System.Collections;
    using System.Net;
    using System.Text;
    using System.IO;
    using System.Xml.Serialization;
    using System.Collections.Generic;
    using System.Threading.Tasks;
    using System.Net.Http;
    using System.Linq;
    using System.Threading;
    using System.IO.Compression;

    public class WSDLParameter
    {
        public string Name { get; set; }
        public object Value { get; set; }
    }

    /// <summary>
    /// WebRequestResponseMeta is similiar with WSDLOperation, appending Url
    /// </summary>
    public class WebRequestResponseMeta
    {
        public string Url { get; set; }
        public string NameSpace { get; set; }
        /// <summary>
        /// RequestMeta's WSDLParameter name is SOAP parameter name, value is the SOAP parameter value. 
        /// </summary>
        public IList<WSDLParameter> RequestMeta { get; set; }
        /// <summary>
        /// ResponseMeta's WSDLParameter name is SOAP parameter name, value is the SOAP parameter value. 
        /// </summary>
        public IList<WSDLParameter> ResponseMeta { get; set; }
        public string InputSoapAction { get; set; } = string.Empty;
        public string OutputSoapAction { get; set; } = string.Empty;
        public string InputMessageName { get; set; } = string.Empty;
        public string OutputMessageName { get; set; } = string.Empty;

        /// <summary>
        /// Help function for querying value by name in RequestMeta and Response Meta.
        /// </summary>
        /// <param name="paramList">Request meta or Response meta</param>
        /// <param name="name"></param>
        /// <returns>the name matched value. Null if it is not existed.</returns>
        public static string GetValueByName(IList<WSDLParameter> paramList, string name)
        {
            if (paramList == null || string.IsNullOrEmpty(name))
            {
                return null;
            }
            foreach (var param in paramList)
            {
                if (param.Name.Equals(name, StringComparison.OrdinalIgnoreCase) &&
                    (param.Value is string))
                {
                    return param.Value.ToString();
                }
            }
            return null;
        }
    }

    public class WSDLComplexTypeArray
    {
        public string Name { get; set; }
        public List<string> Array { get; set; }
    }

    public class MessageResponseException : Exception
    {
        public MessageResponseException(string message) : base(message)
        {

        }

        public MessageResponseException(string message, Exception innerException)
            : base(message, innerException)
        {

        }
    }

    /// <summary>
    /// Using WebRequest/WebResponse for WebService calling helper functions.
    /// </summary>
    public class WebSvcCaller
    {
        //Cache xmlNamespace，avoiding call GetNamespace() repeatedly
        private static Hashtable _xmlNamespaces = new Hashtable();

        private static HttpClient _httpClient = new HttpClient();

        static WebSvcCaller()
        {
            _httpClient.Timeout = new TimeSpan(12, 0, 0);
        }

        /// <summary>
        /// Send networking state copy command to the remote node.
        /// </summary>
        /// <param name="url">Request url</param>
        /// <param name="state">State string</param>
        /// <returns>void</returns>
        public static async Task NetworkCopyStateAsync(string url, string state)
        {
            if (string.IsNullOrWhiteSpace(url) || string.IsNullOrWhiteSpace(state))
            {
                throw new ArgumentException("url or state is null or empty.");
            }
            byte[] data = Encoding.UTF8.GetBytes(state);
            HttpContent content = new ByteArrayContent(data);
            content.Headers.Add("Content-Type", "application/json; charset=utf-8");
            HttpResponseMessage resp = await _httpClient.PostAsync(url, content);
            resp.EnsureSuccessStatusCode();
        }

        public static async Task<string> GetAsyncAsString(string url, CancellationToken token)
        {
            if (string.IsNullOrWhiteSpace(url))
            {
                throw new ArgumentException("url is null or empty.");
            }
            try
            {
                HttpResponseMessage response = await _httpClient.GetAsync(url, token);
                response.EnsureSuccessStatusCode();
                return await response.Content.ReadAsStringAsync();
            }
            catch (HttpRequestException ex)
            {
                throw new MessageResponseException((ex.InnerException?.InnerException?.Message ?? ex.InnerException?.Message ?? ex.Message) + "> " + url);
            }
            catch (TaskCanceledException ex)
            {
                throw new MessageResponseException("Timeout " + ex.Message + "> " + url);
            }
        }

        public static async Task<Stream> GetAsyncAsStream(string url, CancellationToken token)
        {
            if (string.IsNullOrWhiteSpace(url))
            {
                throw new ArgumentException("url is null or empty.");
            }
            try
            {
                HttpResponseMessage response = await _httpClient.GetAsync(url, token);
                response.EnsureSuccessStatusCode();
                return await response.Content.ReadAsStreamAsync();
            }
            catch (HttpRequestException ex)
            {
                throw new MessageResponseException((ex.InnerException?.InnerException?.Message ?? ex.InnerException?.Message ?? ex.Message) + "> " + url);
            }
            catch (TaskCanceledException ex)
            {
                throw new MessageResponseException("Timeout " + ex.Message + "> " + url);
            }
        }

        /// <summary>
        /// Post Asynchrously string content.
        /// </summary>
        /// <param name="url">URL</param>
        /// <param name="reqContentType">Request content-Type, can be empty</param>
        /// <param name="strContent">Content string</param>
        /// <param name="token">A cancellation token</param>
        /// <returns>Response byte array, response content-type, response content-disposition(can be NULL).
        /// response Content-Encoding(gzip signal, can be NULL).
        /// </returns>
        public static async Task<Tuple<byte[], string, string, string>> PostAsync(string url, string reqContentType, string strContent, CancellationToken token)
        {
            return await PostAsync(url, reqContentType, Encoding.UTF8.GetBytes(strContent), token);
        }

        /// <summary>
        /// Post Asynchrously byte array content.
        /// </summary>
        /// <param name="url">URL</param>
        /// <param name="reqContentType">Request content-Type, can be empty</param>
        /// <param name="byteContent">Content byte array</param>
        /// <param name="token">A cancellation token</param>
        /// <returns>Response byte array, response content-type, response content-disposition(can be NULL).
        /// response Content-Encoding(gzip signal, can be NULL).
        /// </returns>
        public static async Task<Tuple<byte[], string, string, string>> PostAsync(string url, string reqContentType, byte[] byteContent, CancellationToken token)
        {
            if (string.IsNullOrWhiteSpace(url) || string.IsNullOrWhiteSpace(reqContentType) ||
                byteContent == null)
            {
                throw new ArgumentException("url, state or content is null or empty.");
            }
            HttpResponseMessage resp;
            byte[] byteArray;
            try
            {
                HttpContent content = new ByteArrayContent(byteContent);
                content.Headers.Add("Content-Type", reqContentType);      
                resp = await _httpClient.PostAsync(url, content, cancellationToken: token).ConfigureAwait(false);
                resp.EnsureSuccessStatusCode();
                byteArray = await resp.Content.ReadAsByteArrayAsync().ConfigureAwait(false);
            }
            catch (HttpRequestException ex)
            {
                throw new MessageResponseException((ex.InnerException?.InnerException?.Message ?? ex.InnerException?.Message ?? ex.Message) + "> " + url);
            }
            catch (TaskCanceledException ex)
            {
                throw new MessageResponseException("Timeout " + ex.Message + "> " + url);
            }

            string respContentType = null;
            IEnumerable<string> enumContentType = null;
            if (resp.Content.Headers.TryGetValues("Content-Type", out enumContentType))
            {
                respContentType = enumContentType.FirstOrDefault();
            }
            string respContentDisposition = null;
            IEnumerable<string> enumContentDisposition = null;
            if (resp.Content.Headers.TryGetValues("Content-Disposition", out enumContentDisposition))
            {
                respContentDisposition = enumContentDisposition.FirstOrDefault();
            }
            string respContentEncoding = null;
            IEnumerable<string> enumContentEncoding = null;
            if (resp.Content.Headers.TryGetValues("Content-Encoding", out enumContentEncoding))
            {
                respContentEncoding = enumContentEncoding.FirstOrDefault();
            }
            return new Tuple<byte[], string, string, string>(byteArray, respContentType, respContentDisposition, respContentEncoding);
        }

        public static async Task<XmlDocument> QuerySoapWebServiceAsync(WebRequestResponseMeta meta, CancellationToken token)
        {

            byte[] data = EncodeParsToSoap(meta.RequestMeta,
                meta.NameSpace, meta.InputMessageName, string.IsNullOrEmpty(meta.InputSoapAction));
            HttpContent content = new ByteArrayContent(data);
            content.Headers.Add("Content-Type", "text/xml; charset=utf-8");
            content.Headers.Add("SOAPAction", meta.InputSoapAction);

            string retXml = null;
            try
            {
#if (DEBUG)
                Console.WriteLine("BeginPostAsync");
#endif
                HttpResponseMessage resp = await _httpClient.PostAsync(meta.Url, content, token).ConfigureAwait(false);
                resp.EnsureSuccessStatusCode();
                if (string.IsNullOrEmpty(meta.OutputMessageName))
                {
                    XmlDocument emptyDoc = new XmlDocument();
                    emptyDoc.LoadXml("<root></root>");
                    AddDelaration(emptyDoc);
                    return emptyDoc;
                }
#if (DEBUG)
                Console.WriteLine("BeginReadStream");
#endif
                var respStream = await resp.Content.ReadAsStreamAsync().ConfigureAwait(false);
#if (DEBUG)
                Console.WriteLine("EndReadStream");
#endif
                // Decompress by "Content-Encoding"
                Stream outputStream = respStream;
                string respContentEncoding = null;
                IEnumerable<string> enumContentEncoding = null;
                if (resp.Content.Headers.TryGetValues("Content-Encoding", out enumContentEncoding))
                {
                    respContentEncoding = enumContentEncoding.FirstOrDefault();
                    if (respContentEncoding.Contains("gzip"))
                    {
                        outputStream = new MemoryStream();   // Reset outputStream here for decompression.
                        using (var compressStream = new GZipStream(respStream, CompressionMode.Decompress))
                        {
                            compressStream.CopyTo(outputStream);
                            outputStream.Seek(0, SeekOrigin.Begin);
                        }
                    }
                }
                using (var reader = new StreamReader(outputStream, Encoding.UTF8, 
                    detectEncodingFromByteOrderMarks: true, bufferSize: 4096, leaveOpen: false))
                {
                    retXml = reader.ReadToEnd();
                }
            }
            catch (HttpRequestException ex)
            {
                throw new MessageResponseException((ex.InnerException?.InnerException?.Message ?? ex.InnerException?.Message ?? ex.Message) + "> " + meta.Url);
            }
            catch (TaskCanceledException ex)
            {
                throw new MessageResponseException("Timeout " + ex.Message + "> " + meta.Url);
            }

            XmlDocument doc = new XmlDocument();
            doc.LoadXml(retXml);
            return doc;
        }

        /// <summary>
        /// Encode parameters to SOAP head and body.
        /// </summary>
        /// <param name="pairs">Parameter key-value pairs</param>
        /// <param name="XmlNs">Namespace</param>
        /// <param name="methodName">Soap operation name.</param>
        /// <param name="isSoapActionEmpty">If "SOAPAction:" is empty, then we don't use prefix for parameter key.
        /// Else, we use prefix for parameter key: For example, using 'apsst' prefix :<apsst:token>6371d79b6afc4916b42b5962d9655df8</apsst:token>
        /// </param>
        /// <returns></returns>
        private static byte[] EncodeParsToSoap(IList<WSDLParameter> pairs, String XmlNs, String methodName, bool isSoapActionEmpty)
        {
            string namespaceURI = "http://schemas.xmlsoap.org/soap/envelope/";
            string namespaceArrayURI = "http://schemas.microsoft.com/2003/10/Serialization/Arrays";
            string apsNamespaceURI = XmlNs;
            XmlDocument doc = new XmlDocument();
            doc.LoadXml($"<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:apsst=\"{XmlNs}\" xmlns:arr=\"{namespaceArrayURI}\"></soapenv:Envelope>");
            AddDelaration(doc);

            XmlElement soapHeader = doc.CreateElement("soapenv", "Header", namespaceURI);
            XmlElement soapBody = doc.CreateElement("soapenv", "Body", namespaceURI);
            XmlElement soapMethod = doc.CreateElement("apsst", methodName, apsNamespaceURI);
            foreach (var param in pairs)
            {
                XmlElement soapPar = null;
                if (isSoapActionEmpty)
                {
                    soapPar = doc.CreateElement(param.Name);
                }
                else
                {
                    soapPar = doc.CreateElement("apsst", param.Name, apsNamespaceURI);
                }
                //soapPar.InnerXml = ObjectToSoapXml(param.Value);
                if (param.Value is string)
                {
                    soapPar.InnerText = param.Value.ToString();
                }
                else if (param.Value is WSDLComplexTypeArray)
                {
                    WSDLComplexTypeArray complexTypeArray = (WSDLComplexTypeArray)param.Value;
                    foreach (var strItem in complexTypeArray.Array)
                    {
                        XmlElement element = doc.CreateElement("arr", complexTypeArray.Name, namespaceArrayURI);
                        element.InnerText = strItem;
                        soapPar.AppendChild(element);
                    }
                }
                soapMethod.AppendChild(soapPar);
            }
            soapBody.AppendChild(soapMethod);
            doc.DocumentElement.AppendChild(soapHeader);
            doc.DocumentElement.AppendChild(soapBody);
            return Encoding.UTF8.GetBytes(doc.OuterXml);
        }

        private static String ParsToString(Hashtable pairs)
        {
            StringBuilder sb = new StringBuilder();
            foreach (string k in pairs.Keys)
            {
                if (sb.Length > 0)
                {
                    sb.Append("&");
                }
                throw new NotImplementedException();
            }
            return sb.ToString();
        }

        private static XmlDocument ReadXmlResponse(WebResponse response)
        {
            using (var sr = new StreamReader(response.GetResponseStream(), Encoding.UTF8))
            {
                string retXml = sr.ReadToEnd();
                XmlDocument doc = new XmlDocument();
                doc.LoadXml(retXml);
                return doc;
            }
        }

        private static void AddDelaration(XmlDocument doc)
        {
            XmlDeclaration decl = doc.CreateXmlDeclaration("1.0", "utf-8", null);
            doc.InsertBefore(decl, doc.DocumentElement);
        }
    }

}
