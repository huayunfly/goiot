import SwiftUI
import Charts

struct  HistoryLineDef: Identifiable {
    var id = UUID()
    var dataId: String
    var group: String
    var name: String
    var displayName: String
}

struct HistoryData: Identifiable {
    var id = UUID()
    var timestamp: Date
    var value: Double
}

struct HistoricalChartView: View {
    @EnvironmentObject var dataManager: DataManager
    
    @State private var historyData: [HistoryData] = []
    
    // 可选的下拉列表选项，对应 DataRecord 中的 keyName
    let metrics = ["temperature": "温度 (Temperature)", "humidity": "湿度 (Humidity)"]
    
    @State private var lineDefs: [HistoryLineDef] = [HistoryLineDef(dataId: "plc.1.temp1_pv", group: "goiot", name: "TC2102", displayName: "固定床预热"),
                                      HistoryLineDef(dataId: "plc.1.temp2_pv", group: "goiot", name: "TC2103",  displayName: "固定床上热"),
                                      HistoryLineDef(dataId: "plc.1.temp3_pv", group: "goiot", name: "TC2104",  displayName: "固定床中热"),
                                      HistoryLineDef(dataId: "plc.1.temp4_pv", group: "goiot", name: "TC2105",  displayName: "固定床下热"),
    ]
    
    // ✅ 抽离 Chart 容器，方便后续扩展滚动/缩放手势
    private struct ChartContainer<Content: View>: View {
        let view: Content
        var body: some View {
            view
                .chartOverlay { _ in
                    GeometryReader { geo in
                        Rectangle().fill(.clear)
                            .contentShape(Rectangle())
                    }
                }
        }
    }
    
    var body: some View {
        VStack {
            VStack {
                List(lineDefs) { line in
                    HStack {
                        Image(systemName: "item.imageName")
                        Text(line.name)
                    }
                }
                
                Button(action: {
                    Task {
                        await reloadHistory()
                    }
                }) {
                    Text("Tap me!")
                }
                
                Spacer()
                
                // ✅ 核心：必须指定明确高度，否则 Chart 会坍塌为 0
                ChartContainer(view: chartView)
                    .frame(height: 300)
                    .padding(.vertical, 4)
                
            }
            .background(Color.gray.opacity(0.05))
            .cornerRadius(12)
            .padding(.horizontal)
        }
        .task {
            // 视图出现时加载数据
            await reloadHistory()
        }
        .onChange(of: dataManager.storageWindowHours) {
            Task {
                await reloadHistory()
            }
        }
    }
    
    @ViewBuilder
    private var chartView: some View {
        if historyData.isEmpty {
            ContentUnavailableView(
                "暂无数据",
                systemImage: "chart.xyaxis.line",
                description: Text("请切换指标或等待数据同步")
            )
        } else {
            Chart(historyData) {
                data in
                // 面积图底色 (渐变)
                AreaMark(
                    x: .value("", data.timestamp),
                    y: .value("", data.value)
                )
                .foregroundStyle(
                    .linearGradient(
                        colors: [.blue.opacity(0.3), .blue.opacity(0.05)],
                        startPoint: .bottom,
                        endPoint: .top
                    )
                )
                
                // 曲线主体
                LineMark(
                    x: .value("", data.timestamp),
                    y: .value("", data.value)
                )
                .foregroundStyle(.blue)
                .interpolationMethod(.catmullRom) // 平滑曲线
                
                // 数据点 (可选，数据量大时建议隐藏)
                PointMark(
                    x: .value("", data.timestamp),
                    y: .value("", data.value)
                )
                .foregroundStyle(.blue)
                .symbolSize(10)
            }
        }
    }
    
    
    
    // 加载对应指标的历史数据
    private func reloadHistory() async {
        let newData = await dataManager.fetchHistoryFor(key: lineDefs[0].dataId, group: lineDefs[0].group, lastHours: dataManager.storageWindowHours)
            withAnimation {
                historyData = newData.map { HistoryData(timestamp: $0.timestamp, value: $0.value)
            }
        }
    }
}


struct HistoricalChartView_Previews: PreviewProvider {
    static var previews: some View {
        HistoricalChartView()
            .environmentObject(DataManager())
    }
}



