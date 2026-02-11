//
//  UserData.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/28.
//

import Foundation
import Combine

class UserData : ObservableObject {
    @Published var token: String = ""
    @Published var username: String = ""
    @Published var isLoggedIn: Bool = false
    @Published var profileData: ProfileData? = nil
    
    init() {
        
    }
    
    func login(username: String, password: String) async throws {
        // DataBus API request
        Task {
            let requestBody = ApiRequestBody(name: "tenant", operation: "Login", token: nil, condition: ["username": username, "password": password])
            let address: String = "http://192.168.2.177:6300/message"
            let response: ApiResultBody = try await WebServiceCaller.PostJSON(to: address, with: requestBody, timeoutInterval: 5)
            guard response.statusCode == "200" else {
                isLoggedIn = false
                throw WebServiceError.loginError
            }
            isLoggedIn = true
            self.username = username
            token = response.result?["token"] ?? "1-2-3-4-5-6"
        }
    }
    
    func logout() {
        isLoggedIn = false
        username = ""
        profileData = nil
    }
}

struct ProfileData: Codable {
    var name: String
    var email: String
    var avatarUrl: String
}
