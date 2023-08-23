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

struct LoginStatus: View {
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

struct SettingTabView: View {
    @State var address: String = "http://192.168.2.107:6301/api"
    @State var username: String = "yun_hua@yashentech.com"
    @State var password: String = "agaeq545b3nbtg"
    @State var refreshInSecond: String = "5"
    @State var isConnected : Bool = true
    @State var token : String = ""
    
    var body: some View {
        List {
            Section(header: Text("服务")) {
                LoginStatus(username: username, status: $isConnected)
                DataInput(title: "地址", imageName: "globe.asia.australia.fill", userInput: $address)
                DataInput(title: "用户", imageName: "person.2.fill", userInput: $username)
                DataInput(title: "密码", imageName: "character.textbox", userInput: $password)
                Button(action: logon) {
                    Text("登录")
                        .font(.system(size: 16))
                        .frame(minWidth: 0, maxWidth: .infinity)
                        .padding(10)
                        .foregroundColor(.white).background(Color(red: 88 / 255, green: 224 / 255, blue: 133 / 255)).cornerRadius(5)
                        .padding(.horizontal, 20)
                }
            }
            Section(header: Text("设置")) {
                HStack {
                    Image(systemName: "arrow.clockwise")
                    Text("刷新（秒）")
                    TextEditor(text: $refreshInSecond)
                        .foregroundColor(.gray)
                        .font(.custom("HelveticaNeue", size: 15))
                        .lineSpacing(5).padding(1)
                }
            }
        }
    }
    
    func logon()
    {
        Task {
            let requestBody = ApiRequestBody(name: "tenant", token: nil, operation: "Login", condition: ["username": username, "password": password])
            
            do {
                let response: ApiResultBody = try await WebServiceCaller.PostJSON(to: address, with: requestBody)
                guard response.statusCode == "200" else {
                    print("Login return status error.")
                    throw WebServiceError.statusError
                }
                token = response.result["token"] ?? "1-2-3-4-5-6"
                print(token)
            } catch let ex {
                print(ex)
            }
        }

        
//        let url = URL(string: address)!
//        var request = URLRequest(url: url)
//        let body: [String: Any] = ["name": "tenant", "operation": "Login", "condition": ["username": username, "password": password]]
//        let bodyData = try? JSONSerialization.data(
//            withJSONObject: body,
//            options: []
//        )

//        // Change the URLRequest to a POST request
//        request.httpMethod = "POST"
//        request.httpBody = bodyData
//        request.addValue("application/json", forHTTPHeaderField: "Content-Type")
//        request.addValue("utf-8", forHTTPHeaderField: "charset")
//
//        // Create the HTTP request
//        let session = URLSession.shared
//        let task = session.dataTask(with: request) { (data, response, error) in
//            if let error = error {
//                print(error)
//            }
//            else if let data {
//                let decoder = JSONDecoder()
//                let response = try? decoder.decode(ApiResultBody.self, from: data)
//                if let response, response.statusCode == "200" {
//                    token = response.result["token"] ?? "1-2-3-4-5-6"
//                    print(token)
//                }
//                else {
//                    print("JSON decode failed.")
//                }
////                // JSON parse example directively.
////                let json = try? JSONSerialization.jsonObject(with: data, options: [])
////                if let response = json as? [String: Any]
////                {
////                    if let status = response["statusCode"] as? String
////                    {
////                        if status == "200", let result = response["result"] as? [String: Any]
////                        {
////                            token = result["token"] as? String ?? "1-2-3-4-5-6"
////                            print(token)
////                            isConnected = !isConnected
////                        }
////                    }
////                }
//            }
//            else
//            {
//                print("Unexpected error.")
//            }
//        }
//        task.resume()
    }
}

struct SettingTabView_Previews: PreviewProvider {
    static var previews: some View {
        SettingTabView()
    }
}
