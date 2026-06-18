//
//  LoginView.swift
//  goiot
//
//  Created by YUN HUA on 2026/2/5.
//  updated UI style on 2026-06-16
//

import SwiftUI

struct LoginView: View {
    @EnvironmentObject var userData: UserData
    
    @State private var username = ""
    @State private var password = ""
    @State private var serviceAddress = NetworkConfig.baseURL
    
    @State private var errorMessage: String?
    @State private var isLoading = false
    @State private var isLoggedIn = false
    
    @FocusState private var focusedField: FormField?
    
    enum FormField: Hashable {
        case username
        case password
        case address
    }
    
    var body: some View {
        NavigationView {
            ScrollView {
                VStack(spacing: 28) {
                    // 1. 品牌区域
                    VStack(spacing: 8) {
                        Image(systemName: "globe.desk")
                            .font(.system(size: 60))
                            .foregroundColor(.blue)
                        Text("系统登录")
                            .font(.title.bold())
                        Text("请输入您的凭据及服务地址")
                            .font(.subheadline)
                            .foregroundColor(.secondary)
                    }
                    .padding(.top, 80)
                    
                    // 2. 🚀 表单区域 (优化留白)
                    VStack(spacing: 18) {
                        FormInputIcon(
                            text: $username,
                            placeholder: "请输入用户名",
                            icon: "person.fill",
                            isSecure: false,
                            isFocused: focusedField == .username
                        )
                        .focused($focusedField, equals: .username)
                        
                        FormInputIcon(
                            text: $password,
                            placeholder: "请输入密码",
                            icon: "lock.fill",
                            isSecure: true,
                            isFocused: focusedField == .password
                        )
                        .focused($focusedField, equals: .password)
                        
                        Divider()
                            .background(Color.gray.opacity(0.3))
                        
                        Text("网络设置")
                            .font(.caption.bold())
                            .foregroundColor(.secondary)
                            .frame(maxWidth: .infinity, alignment: .leading)
                            .padding(.horizontal, 4)
                            .padding(.bottom, 4)
                        
                        FormInputIcon(
                            text: $serviceAddress,
                            placeholder: "http://api-server-address:port",
                            icon: "network",
                            isSecure: false,
                            isFocused: focusedField == .address
                        )
                        .focused($focusedField, equals: .address)
                    }
                    // 增加 24 的内部留白，解决顶部贴合问题
                    .padding(24)
                    .background(Color(.systemBackground))
                    .cornerRadius(20)
                    .shadow(color: Color.black.opacity(0.08), radius: 12, x: 0, y: 6)
                    .padding(.horizontal, 16)
                    
                    // 登录按钮
                    VStack(spacing: 16) {
                        Button(action: handleLogin) {
                            ZStack {
                                if isLoading {
                                    // 优化：给 ProgressView 设置最小宽高，防止它在按钮中间乱跑
                                    ProgressView()
                                        .progressViewStyle(CircularProgressViewStyle(tint: .white))
                                        .scaleEffect(1.2)
                                } else {
                                    Text("立即登录")
                                        .font(.body)
                                        .fontWeight(.semibold) // 🚀 增加字重，更专业
                                        .foregroundColor(.white)
                                        .padding(.horizontal, 32) // 🚀 左右增加额外留白
                                }
                            }
                            // 🚀 核心修正：显式定义按钮的高度 (Apple 标准大按钮高度 56pt)
                            .frame(height: 56)
                            .frame(maxWidth: .infinity) // 撑满父容器宽度
                            
                            .background(Color.blue)
                            .cornerRadius(28) // 🚀 对应高度的一半，形成完美的胶囊形状
                            
                            // 🚀 增加柔和阴影，提升悬浮感
                            .shadow(color: Color.blue.opacity(0.3), radius: 10, x: 0, y: 4)
                        }
                        .disabled(isLoading || errorMessage != nil) // 加载中或有错误时禁用
                        .opacity(isLoading ? 0.7 : 1.0) // 加载时略微变灰
                    }
                    .padding(.horizontal, 16) // 按钮外部左右留出边距
                    
                    // 错误提示
                    if let error = errorMessage {
                        Text(error)
                            .font(.caption)
                            .foregroundColor(.white)
                            .padding(12)
                            .frame(maxWidth: .infinity)
                            .background(Color.red.opacity(0.85))
                            .cornerRadius(10)
                            .padding(.horizontal, 24)
                            .transition(.opacity.combined(with: .move(edge: .top)))
                    }
                }
                .padding(.top, 20)
                .padding(.bottom, 40)
            }
            .background(Color(.systemGroupedBackground))
            .ignoresSafeArea()
        }
        .overlay {
            if isLoggedIn {
                ZStack {
                    Color(.systemBackground).opacity(0.9).ignoresSafeArea()
                    VStack(spacing: 16) {
                        Image(systemName: "checkmark.seal.fill")
                            .font(.title)
                            .foregroundColor(.green)
                        Text("认证成功")
                            .font(.title2.bold())
                    }
                }
                .transition(.opacity)
                .animation(.easeInOut(duration: 0.3), value: isLoggedIn)
            }
        }
    }
    
    private func handleLogin() {
        guard !username.isEmpty && !password.isEmpty else {
            errorMessage = "账号和密码不能为空"; return
        }
        
        errorMessage = nil
        isLoading = true
        
        Task {
            do {
                try await userData.login(username: username, password: password, customURL: serviceAddress)
                isLoggedIn = true
            } catch {
                errorMessage = "登录失败: \(error)"
                try await Task.sleep(nanoseconds: 3_000_000_000)
                errorMessage = nil
            }
            isLoading = false
        }
    }
}

// MARK: - 🚀 优化后的输入框组件 (增加垂直内边距)
struct FormInputIcon: View {
    @Binding var text: String
    var placeholder: String
    var icon: String
    var isSecure: Bool
    var isFocused: Bool
    
    var body: some View {
        HStack(spacing: 12) {
            Image(systemName: icon)
                .foregroundColor(isFocused ? .blue : .gray)
                .frame(width: 20)
            
            Group {
                if isSecure {
                    SecureField(placeholder, text: $text)
                } else {
                    TextField(placeholder, text: $text)
                        .autocapitalization(.none)
                        .disableAutocorrection(true)
                        .textContentType(.username) // 提示系统输入法
                }
            }
        }
        // 核心优化：将 padding 设为 14，让输入框上下更饱满，不显局促
        .padding(14)
        .background(Color.gray.opacity(0.08))
        .cornerRadius(12)
        .overlay(
            RoundedRectangle(cornerRadius: 12)
                .stroke(isFocused ? Color.blue : Color.clear, lineWidth: 2)
        )
    }
}

struct LoginView_Previews: PreviewProvider {
    static var previews: some View {
        LoginView()
            .environmentObject(UserData())
    }
}
