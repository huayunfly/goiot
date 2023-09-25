//
//  SettingTabView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI

struct SettingItem : Identifiable {
    var id = UUID()
    var task: String
    var imageName: String
}

struct DataInput: View {
    var title: String
    var imageName: String
    @Binding var userInput: String
    var body: some View {
        HStack() {
            Image(systemName: imageName)
            Text(title)
                .font(.headline)
            TextField("输入 \(title)", text: $userInput)
                .foregroundColor(.gray)
                .font(.custom("HelveticaNeue", size: 15))
        }
    }
}

struct LogonStatus: View {
    var username : String
    @Binding var status: Bool
    var body: some View {
        HStack() {
            Toggle(isOn: $status)
            {
                Text(username)
            }
        }
    }
}

struct LogonButton: View {
    @Binding var isConnected: Bool
    
    // Constants
    static private let COLOR_LOGON = Color(red: 88 / 255, green: 224 / 255, blue: 133 / 255)
    static private let COLOR_LOGOFF = Color(red: 255 / 255, green: 99 / 255, blue: 71 / 255)
    static private let TXT_LOGON = "登录"
    static private let TXT_LOGOFF = "退出登录"
    
    // Action
    var logonAction: () -> Void
    
    var body: some View {
        HStack() {
            Button(action: logonAction) {
                Text(isConnected ? LogonButton.TXT_LOGOFF : LogonButton.TXT_LOGON)
                    .font(.system(size: 16))
                    .frame(minWidth: 0, maxWidth: .infinity)
                    .padding(10)
                    .foregroundColor(.white).background(isConnected ? LogonButton.COLOR_LOGOFF : LogonButton.COLOR_LOGON).cornerRadius(5)
                    .padding(.horizontal, 20)
            }
        }
    }
}

struct SettingTabView: View {
    
    @State private var address: String = "http://192.168.2.107:6301/api"
    @State private var username: String = "yun_hua@yashentech.com"
    @State private var password: String = "agaeq545b3nbtg"
    @State private var refreshInSecond: String = "5"
    @State private var isConnected : Bool = false
    @State private var txtLogon = TXT_LOGON
    @State private var colorLogon = COLOR_LOGON
    
    @EnvironmentObject var userData: UserData
    
    // Constants
    static private let COLOR_LOGON = Color(red: 88 / 255, green: 224 / 255, blue: 133 / 255)
    static private let COLOR_LOGOFF = Color(red: 255 / 255, green: 99 / 255, blue: 71 / 255)
    static private let TXT_LOGON = "登录"
    static private let TXT_LOGOFF = "退出登录"
    
    var body: some View {
        List {
            Section(header: Text("服务")) {
                LogonStatus(username: username, status: $isConnected)
                DataInput(title: "地址", imageName: "globe.asia.australia.fill", userInput: $address)
                DataInput(title: "用户", imageName: "person.2.fill", userInput: $username)
                DataInput(title: "密码", imageName: "character.textbox", userInput: $password)
                LogonButton(isConnected: $isConnected, logonAction: logon)
            }
            Section(header: Text("设置")) {
                HStack {
                    DataInput(title: "刷新（秒）", imageName: "arrow.clockwise", userInput: $refreshInSecond)
                }
            }
        }
    }
    
    func logon()
    {
        Task {
            let requestBody = ApiRequestBody(name: "tenant", token: nil, operation: "Login", condition: ["username": username, "password": password])
            
            do {
                if !isConnected {
                    let response: ApiResultBody = try await WebServiceCaller.PostJSON(to: address, with: requestBody, timeoutInterval: 5)
                    guard response.statusCode == "200" else {
                        print("Login return status error.")
                        throw WebServiceError.statusError
                    }
                    userData.username = username
                    userData.address = address
                    userData.logon = true
                    userData.token = response.result["token"] ?? "1-2-3-4-5-6"
                    isConnected = true
                    txtLogon = SettingTabView.TXT_LOGOFF
                    colorLogon = SettingTabView.COLOR_LOGOFF
                    print(userData.token)
                }
                else {
                    userData.logon = false
                    userData.token = ""
                    isConnected = false;
                    txtLogon = SettingTabView.TXT_LOGON
                    colorLogon = SettingTabView.COLOR_LOGON
                    print("Logoff")
                }
            } catch let ex {
                print(ex)
            }
        }
    }
}

struct SettingTabView_Previews: PreviewProvider {
    static var previews: some View {
        SettingTabView()
    }
}
