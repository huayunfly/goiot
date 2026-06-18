//
//  WebServiceError.swift
//  goiot
//
//  Created by YUN HUA on 2026/6/15.
//


import Foundation
import os.log

// 统一网络日志管理器
private let networkLogger = Logger(subsystem: Bundle.main.bundleIdentifier ?? "com.goiot", category: "Network")

// 保持您原有的枚举定义，避免 redeclaration 报错
enum WebServiceError: Error {
    case invalidServerResponse
    case invalidJSONEncode
    case invalidJSONDecode
    case invalidTypeConversion
    case invalidURL
    case statusError
    case loginError
    case GetDataError
}

final class WebServiceCaller {
    
    /// 基础 POST 请求
    static func Post(to url: String, contentType reqContentType: String, with sendData: Data, timeoutInterval timeout: TimeInterval) async throws -> Data {
        guard let urlObj = URL(string: url) else {
            networkLogger.error("无效 URL: \(url)")
            throw WebServiceError.invalidURL
        }
        
        var request = URLRequest(url: urlObj)
        request.httpMethod = "POST"
        request.httpBody = sendData
        request.setValue(reqContentType, forHTTPHeaderField: "Content-Type")
        request.timeoutInterval = timeout
        
        let (data, response) = try await URLSession.shared.data(for: request)
        
        guard let httpResponse = response as? HTTPURLResponse else {
            networkLogger.error("非 HTTP 响应类型: \(String(describing: response))")
            throw WebServiceError.invalidServerResponse
        }
        
        guard (200..<300).contains(httpResponse.statusCode) else {
            networkLogger.error("HTTP 状态码异常: \(httpResponse.statusCode)")
            throw WebServiceError.statusError
        }
        
        return data
    }
    
    /// 字典型 JSON 请求
    static func PostJSONDictionary(to url: String, with objectJSON: [String: Any], timeoutInterval timeout: TimeInterval) async throws -> [String: Any] {
        let bodyData = try JSONSerialization.data(withJSONObject: objectJSON, options: [])
        let responseData = try await Post(to: url, contentType: "application/json", with: bodyData, timeoutInterval: timeout)
        
        let jsonObject = try JSONSerialization.jsonObject(with: responseData, options: [])
        guard let dict = jsonObject as? [String: Any] else {
            throw WebServiceError.invalidTypeConversion
        }
        return dict
    }
    
    /// 泛型 Codable 请求
    static func PostJSON<T1: Encodable, T2: Decodable>(to url: String, with objectJSON: T1, timeoutInterval timeout: TimeInterval) async throws -> T2 {
        let bodyData = try JSONEncoder().encode(objectJSON)
        let responseData = try await Post(to: url, contentType: "application/json", with: bodyData, timeoutInterval: timeout)
        
        // 专业日志：仅 Debug 输出完整响应，Release 自动禁用，兼顾调试与性能
        #if DEBUG
        if let logStr = String(data: responseData, encoding: .utf8) {
            networkLogger.debug("[API Response] \(logStr)")
        }
        #endif
        
        let decoder = JSONDecoder()
        return try decoder.decode(T2.self, from: responseData)
    }
}

