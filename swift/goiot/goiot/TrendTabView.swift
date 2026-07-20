//
//  TrendTabView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI

struct TrendTabView: View {
    @EnvironmentObject var dataManager: DataManager
    
    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVStack(spacing: 16) {
                    Text("趋势面板")
                        .font(.headline)
                        .padding(.top, 8)
                }
                .padding(.horizontal)
                .padding(.bottom, 30)
            }
            .navigationTitle("趋势")
        }
    }
}

struct TrendTabView_Previews: PreviewProvider {
    static var previews: some View {
        TrendTabView().environmentObject(DataManager())
    }
}
