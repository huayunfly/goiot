//
//  UserData.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/28.
//

import Foundation
import SwiftUI
import os.log

private let userDataLogger = Logger(subsystem: Bundle.main.bundleIdentifier ?? "com.goiot", category: "UserData")

/// 集中管理 API 地址，彻底解决硬编码问题
struct NetworkConfig {
    #if DEBUG
    static let baseURL = "http://192.168.2.177:6300/message"
    #else
    static let baseURL = "https://api.your-production-domain.com/message" // 替换为线上域名
    #endif
}

/// @MainActor 确保 @Published 属性更新始终在主线程，避免 SwiftUI 警告与 UI 卡死
@MainActor
final class UserData: ObservableObject {
    @Published var token: String = ""
    @Published var username: String = ""
    @Published var isLoggedIn: Bool = false
    @Published var profileData: ProfileData? = nil
    
    func login(username: String, password: String) async throws {
        let request = ApiGeneralRequest(
            name: "tenant",
            operation: "login",
            token: nil,
            condition: ["username": username, "password": password]
        )
        
        do {
            let response: ApiGeneralResponse = try await WebServiceCaller.PostJSON(
                to: NetworkConfig.baseURL,
                with: request,
                timeoutInterval: 5.0
            )
            
            isLoggedIn = true
            self.username = username
            
            // 安全提取 token，移除危险的硬编码 fallback
            if let validToken = response.result?["token"] as? String, !validToken.isEmpty {
                self.token = validToken
                userDataLogger.info("用户 \(username) 登录成功")
            } else {
                userDataLogger.error("登录响应缺失有效 token")
                throw WebServiceError.loginError
            }
        } catch {
            // 记录真实错误堆栈，状态置空后向上抛出，便于 UI 层统一提示
            userDataLogger.error("登录失败: \(error.localizedDescription)")
            isLoggedIn = false
            self.token = ""
            throw error
        }
    }
    
    func logout() {
        isLoggedIn = false
        username = ""
        token = ""
        profileData = nil
        userDataLogger.info("用户已退出登录")
    }
}

struct ProfileData: Codable {
    var name: String
    var email: String
    var avatarUrl: String
}

