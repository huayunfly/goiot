//
//  TrendTabView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI

struct TrendTabView: View {
    
    @EnvironmentObject var userData: UserData
    
    var body: some View {
        Text("Second Content View \(userData.token)")
    }
}

struct TrendTabView_Previews: PreviewProvider {
    static var previews: some View {
        TrendTabView().environmentObject(UserData())
    }
}
