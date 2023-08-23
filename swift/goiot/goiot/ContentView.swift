//
//  ContentView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI

struct ContentView: View {
    
    @State private var selection = 1
    
    var body: some View {
        TabView(selection: $selection) {
            MonitorTabView()
                .tabItem {
                    Image(systemName: "checkerboard.rectangle")
                    Text("监视")
                }.tag(1)
            TrendTabView()
                .tabItem {
                    Image(systemName: "chart.line.flattrend.xyaxis")
                    Text("趋势")
                }.tag(2)
            SettingTabView()
                .tabItem {
                    Image(systemName: "gear")
                    Text("设置")
                }.tag(3)
        }
        .font(.largeTitle)
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
