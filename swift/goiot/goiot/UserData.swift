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
    @Published var address: String = ""
    @Published var logon: Bool = false
    
    init() {
        
    }
}
