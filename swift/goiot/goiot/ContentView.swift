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
        Group {
            if userData.isLoggedIn {
                MyTabView().environmentObject(userData).environmentObject(dataManager)
            } else {
                LoginView().environmentObject(userData)
            }
        }//在顶层注入一次 EnvironmentObject，所有子视图都会自动继承，
        // 无需在下面每个 TabItem 重复注入。
        .environmentObject(userData)
        .environmentObject(dataManager)
    }
}

struct MyTabView: View {
    
    @State private var selection = 1
    @EnvironmentObject var userData: UserData
    @EnvironmentObject var dataManager: DataManager
    
    var body: some View {
        TabView(selection: $selection) {
            MonitorControlTabView()
                .tabItem {
                    AppIcon.monitor.tabIconStyle()
                    Text("监视")
                }.tag(1)
            TrendTabView()
                .tabItem {
                    AppIcon.trend.tabIconStyle()
                    Text("趋势")
                }.tag(2)
            HMIControlTabView()
                .tabItem {
                    Image(systemName: "rectangle.grid.1x2")
                    Text("图形控制")
                }
                .tag(3)
            SettingTabView()
                .tabItem {
                    AppIcon.settings.tabIconStyle()
                    Text("设置")
                }.tag(4)
        }//FIX：移除 .font(.largeTitle) 修饰符是继承（Cascading）的。"大字体" 设置泄漏到了SettingTabView 的所有子组件中，导致排版崩溃。
    }
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
