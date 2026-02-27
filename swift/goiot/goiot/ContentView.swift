//
//  ContentView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI

struct ContentView: View {
    
    @StateObject var userData: UserData = UserData()
    
    @StateObject var dataManager: DataManager = DataManager()
    
    var body: some View {
        NavigationStack {
            if userData.isLoggedIn {
                MyTabView().environmentObject(userData).environmentObject(dataManager)
            } else {
                LoginView().environmentObject(userData)
            }
        }
    }
}

struct MyTabView: View {
    
    @State private var selection = 1
    @EnvironmentObject var userData: UserData
    @EnvironmentObject var dataManager: DataManager
    
    var body: some View {
        TabView(selection: $selection) {
            MonitorTabView()
                .tabItem {
                    Image(systemName: "checkerboard.rectangle")
                    Text("监视")
                }.tag(1).environmentObject(userData).environmentObject(dataManager)
            TrendTabView()
                .tabItem {
                    Image(systemName: "chart.line.flattrend.xyaxis")
                    Text("趋势")
                }.tag(2).environmentObject(userData)
            SettingTabView()
                .tabItem {
                    Image(systemName: "gear")
                    Text("设置")
                }.tag(3).environmentObject(userData)
        }.font(.largeTitle)
    }
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
