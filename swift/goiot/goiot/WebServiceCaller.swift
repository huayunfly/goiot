//
//  WebServiceCaller.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/22.
//

import Foundation

enum WebServiceError: Error {
    case noConnection
    case lowBandwidth
    case fileNotFound
    case timeout
    case invalidServerResponse
    case invalidJSONEncode
    case invalidJSONDecode
    case invalidTypeConversion
    case statusError
}

class WebServiceCaller {
    
    /// Post Data
    ///
    /// - Parameter url: The URL
    /// - Parameter reqContentType: content type
    /// - Parameter sendData: json to send
    /// - Returns: Data
    class func Post(to url: String, contentType reqContentType: String, with sendData: Data) async throws -> Data {
        let url = URL(string: url)!
        var request = URLRequest(url: url)

        // Change the URLRequest to a POST request
        request.httpMethod = "POST"
        request.httpBody = sendData
        request.addValue(reqContentType, forHTTPHeaderField: "Content-Type")
        request.addValue("utf-8", forHTTPHeaderField: "charset")
        
        let (data, response) = try await URLSession.shared.data(for: request)
        
        guard let httpResponse = response as? HTTPURLResponse,
                 httpResponse.statusCode == 200 else {
               throw WebServiceError.invalidServerResponse
           }
        
        return data;
    }
    
    /// Post JSON object in [String: Any]
    ///
    /// - Parameter url: The URL
    /// - Parameter objectJSON: json to send
    /// - Returns: JSON object in [String: Any].
    class func PostJSONDictionary(to url: String, with objectJSON: [String: Any]) async throws -> [String: Any] {
        guard let bodyData = try? JSONSerialization.data(
            withJSONObject: objectJSON,
            options: []
        ) else {
            throw WebServiceError.invalidJSONEncode
        }
        
        let data = try await Post(to: url, contentType: "application/json", with: bodyData)
        guard let json = try? JSONSerialization.jsonObject(with: data, options: []) else {
            throw WebServiceError.invalidJSONDecode
        }
        
        guard let converted = json as? [String: Any] else {
            throw WebServiceError.invalidTypeConversion
        }
        return converted
    }
    
    /// Post JSON object in Encodable object
    ///
    /// - Parameter url: The URL
    /// - Parameter objectJSON: Encodable object to send
    /// - Returns: Decodable object.
    class func PostJSON<T1: Encodable, T2: Decodable>(to url: String, with objectJSON: T1) async throws -> T2 {
        let encoder = JSONEncoder()
        guard let bodyData = try? encoder.encode(objectJSON) else {
            throw WebServiceError.invalidJSONEncode
        }
        
        let data = try await Post(to: url, contentType: "application/json", with: bodyData)
        
        let decoder = JSONDecoder()
        guard let response = try? decoder.decode(T2.self, from: data) else {
            throw WebServiceError.invalidJSONDecode
        }
        return response
    }
}
