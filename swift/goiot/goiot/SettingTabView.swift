////
////  SettingTabView.swift
////  goiot
////
////  Created by YUN HUA on 2023/8/10.
////

import SwiftUI
import os.log

private let settingsLogger = Logger(subsystem: Bundle.main.bundleIdentifier ?? "com.goiot", category: "Settings")

struct SettingTabView: View {
    @EnvironmentObject var userData: UserData
    
    @State private var serverAddress = NetworkConfig.baseURL
    @State private var inputUsername = ""
    @State private var inputPassword = ""
    @State private var refreshInterval = 5
    @State private var isShowPassword = false
    @State private var isAuthenticating = false
    
    @FocusState private var focusedField: SettingsField?
    enum SettingsField: Hashable { case address, username, password, interval }
    
    var body: some View {
        NavigationStack {
            Form {
                // 1. 认证状态面板
                Section {
                    HStack {
                        Circle()
                            .fill(userData.isLoggedIn ? Color.green : Color.red)
                            .frame(width: 8, height: 8)
                        Text(userData.isLoggedIn ? "已连接服务端" : "未连接")
                        Spacer()
                        Text(userData.username.isEmpty ? "未登录" : userData.username)
                            .foregroundColor(.secondary)
                            .font(.subheadline)
                    }
                    .padding(.vertical, 4)
                    
                    authenticationButton
                } header: {
                    Label("认证状态", systemImage: "lock.shield")
                }
                
                // 2. 服务地址配置
                Section {
                    TextField("API 基础路径", text: $serverAddress)
                        .textInputAutocapitalization(.never)
                        .keyboardType(.URL)
                        .focused($focusedField, equals: .address)
                    Text("请输入完整的服务器地址，如 http://192.168.x.x:port" )
                        .font(.caption)
                        .foregroundColor(.secondary)
                        .padding(.top, 8)
                    Button("保存并同步至全局配置") {
                        saveServerAddress()
                    }
                } header: {
                    Label("网络设置", systemImage: "globe.asia.australia.fill")
                }
                
                // 3. 登录凭据管理
                Section {
                    TextField("用户名", text: $inputUsername)
                        .focused($focusedField, equals: .username)
                    
                    HStack {
                        if isShowPassword {
                            TextField("密码", text: $inputPassword)
                                .focused($focusedField, equals: .password)
                        } else {
                            SecureField("密码", text: $inputPassword)
                                .focused($focusedField, equals: .password)
                        }
                        Button {
                            isShowPassword.toggle()
                        } label: {
                            Image(systemName: isShowPassword ? "eye.slash.fill" : "eye.fill")
                                .foregroundColor(.gray)
                        }
                    }
                } header: {
                    Label("凭据管理", systemImage: "key.fill")
                }
                
                // 4. 性能与同步策略
                Section {
                    Stepper("数据刷新间隔: \(refreshInterval) 秒", value: $refreshInterval, in: 1...60, step: 1)
                } header: {
                    Label("同步策略", systemImage: "arrow.clockwise")
                }
            }
            .listStyle(.insetGrouped) // 推荐使用 InsetGrouped 样式更美观
            .navigationTitle("用户设置")
            .toolbar {
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button {
                        syncFromUserData()
                    } label: {
                        Image(systemName: "arrow.triangle.2.circlepath")
                    }
                }
            }
            .onAppear {
                syncFromUserData()
            }
        }
    }
    
    // ... (以下的 handleAuthChange 等逻辑保持不变)
    
    private var authenticationButton: some View {
        Button {
            handleAuthChange()
        } label: {
            HStack {
                Image(systemName: isAuthenticating ? "arrow.triangle.rotate.90.right" : (userData.isLoggedIn ? "rectangle.portrait.and.arrow.right" : "lock.open"))
                Text(isAuthenticating ? "处理中..." : (userData.isLoggedIn ? "退出登录" : "立即登录"))
                    .fontWeight(.semibold)
            }
            .frame(maxWidth: .infinity)
            .padding(.vertical, 14)
            .background(userData.isLoggedIn ? Color(.systemRed) : Color.blue)
            .foregroundColor(.white)
            .cornerRadius(12)
        }
        .disabled(isAuthenticating)
        .opacity(isAuthenticating ? 0.7 : 1.0)
    }
    
    private func handleAuthChange() {
        isAuthenticating = true
        Task {
            do {
                if userData.isLoggedIn {
                    await userData.logout()
                    settingsLogger.info("设置页退出登录")
                } else {
                    guard !inputUsername.isEmpty && !inputPassword.isEmpty else {
                        throw WebServiceError.loginError
                    }
                    try await userData.login(username: inputUsername, password: inputPassword, customURL: serverAddress)
                    inputPassword = ""
                    settingsLogger.info("设置页登录成功")
                }
            } catch {
                settingsLogger.error("认证失败: \(error)")
            }
            await MainActor.run {
                isAuthenticating = false
            }
        }
    }
    
    private func saveServerAddress() {
        UserDefaults.standard.set(serverAddress, forKey: "app_global_server_url")
        settingsLogger.info("已持久化地址")
    }
    
    private func syncFromUserData() {
        inputUsername = userData.username
        if let saved = UserDefaults.standard.string(forKey: "app_global_server_url") {
            serverAddress = saved
        }
    }
}

struct SettingTabView_Previews: PreviewProvider {
    static var previews: some View {
        SettingTabView().environmentObject(UserData())
    }
}

