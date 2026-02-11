//
//  LoginView.swift
//  goiot
//
//  Created by YUN HUA on 2026/2/5.
//

import SwiftUI
import Combine

struct LoginView: View {
    // 登录状态和数据模型
    @State private var username = ""
    @State private var password = ""
    @State private var error: String?
    @State private var isLoading = false
    @State private var isLoggedIn = false
    
    // 表单验证状态
    @State private var isUsernameValid = true
    @State private var isPasswordValid = true
    
    @EnvironmentObject var userData: UserData
    
    // 用于取消订阅的发布者
    private var cancellables = Set<AnyCancellable>()
    
    var body: some View {
        VStack(spacing: 20) {
            // 标题
            Image(systemName: "person.crop.circle")
                .font(.title)
            Text("欢迎登录")
                .font(.title)
                .foregroundColor(.primary)
                .padding(.top, 10)
            
            // 表单
            VStack(alignment: .leading, spacing: 15) {
                // 用户名输入框
                Group {
                    HStack {
                        Image(systemName: "person")
                            .foregroundColor(.blue)
                            .frame(width: 24)
                        TextField("用户名", text: $username)
                            .padding(.vertical, 8)
                    }
                    .padding(.horizontal, 16)
                    .background(Color(.systemGray6))
                    .cornerRadius(8)
                    .overlay(alignment: .trailing) {
                        if !isUsernameValid {
                            Image(systemName: "exclamationmark.circle")
                                .foregroundColor(.red)
                                .padding(8)
                        }
                    }
                    .onChange(of: username) { newValue in
                        validateUsername(newValue)
                    }
                }
                
                // 密码输入框
                Group {
                    HStack {
                        Image(systemName: "lock")
                            .foregroundColor(.blue)
                            .frame(width: 24)
                        SecureField("密码", text: $password)
                            .padding(.vertical, 10)
                    }
                    .padding(.horizontal, 16)
                    .background(Color(.systemGray6))
                    .cornerRadius(8)
                    .overlay(alignment: .trailing) {
                        if !isPasswordValid {
                            Image(systemName: "exclamationmark.circle")
                                .foregroundColor(.red)
                                .padding(8)
                        }
                    }
                    .onChange(of: password) { newValue in
                        validatePassword(newValue)
                    }
                }
            }
            .padding()
            
            // 登录按钮
            Button {
                handleLogin()
            } label: {
                ZStack {
                    if isLoading {
                        ProgressView()
                            .tint(.white)
                    } else {
                        Text("登录")
                            .font(.title2)
                    }
                }
                .frame(width: 300, height: 50)
                .background(Color.blue)
                .foregroundColor(.white)
                .cornerRadius(12)
                .padding()
            }
            .disabled(!isFormValid || isLoading)
            .opacity(isFormValid ? 1 : 0.7)
            
            // 错误提示
            if let error = error {
                Text(error)
                    .foregroundColor(.red)
                    .font(.body)
            }
            
            // 登录成功后的界面
            if isLoggedIn {
                SuccessView()
            }
        }
        .padding()
        .frame(width: 350, height: 380)
        .background(Color(.systemBackground))
        .cornerRadius(20)
        .shadow(radius: 10)
    }
    
    private var isFormValid: Bool {
        isUsernameValid && isPasswordValid
    }
    
    private func validateUsername(_ username: String) {
        let isValid = !username.isEmpty && username.count >= 3
        isUsernameValid = isValid
    }
    
    private func validatePassword(_ password: String) {
        let isValid = !password.isEmpty && password.count >= 6
        isPasswordValid = isValid
    }
    
    private func handleLogin() {
        error = nil
        isLoading = true
        
        // 模拟网络请求延迟
//        DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
//            // 模拟登录成功
//            if username == "test" && password == "password" {
//                isLoggedIn = true
//            } else {
//                self.error = "用户名或密码错误"
//                isLoading = false
//            }
//        }
        // web login()
        loginToWebService()
    }
}

struct SuccessView: View {
    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "checkmark.circle")
                .font(.title)
                .foregroundColor(.green)
            Text("登录成功！")
                .font(.title)
            Text("正在跳转...")
                .font(.body)
                .foregroundColor(.secondary)
        }
        .padding()
    }
}

struct LoginView_Previews: PreviewProvider {
    static var previews: some View {
        LoginView()
    }
}

extension LoginView {
    private func loginToWebService() {
        guard !username.isEmpty && !password.isEmpty else {
            self.error = "请输入用户名和密码"
            self.isLoading = false;
            return
        }

        // DataBus API request
        Task {
            do {
                try await userData.login(username: "goiot", password: "abc123")
                self.isLoading = false
                isLoggedIn = userData.isLoggedIn
            
            } catch let ex {
                self.error = "错误：\(ex)"
                self.isLoading = false
            }
        }
    }
}
