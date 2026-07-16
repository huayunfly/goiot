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
                    Text("历史趋势面板")
                        .font(.headline)
                        .padding(.top, 8)
                    HistoricalChartView()
                        .padding(.horizontal)
                }
                .padding(.bottom)
            }
            .navigationTitle("历史曲线")
        }
    }
}

struct TrendTabView_Previews: PreviewProvider {
    static var previews: some View {
        TrendTabView().environmentObject(DataManager())
    }
}
