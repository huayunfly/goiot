import SwiftUI
import Charts

/// 1. 定义曲线配置模型
struct HistoryLineDef: Identifiable, Equatable {
    var id = UUID()
    var dataId: String
    var group: String
    var name: String
    var displayName: String
    // 增加默认选中状态
    var isSelected: Bool = true
    
    static func == (lhs: HistoryLineDef, rhs: HistoryLineDef) -> Bool {
        return lhs.id == rhs.id
    }
}

/// 2. 定义绘图数据点 (增加 seriesId 用于分组和自动 Legend)
struct ChartData: Identifiable, Comparable {
    var id = UUID()
    var timestamp: Date
    var value: Double
    // 核心属性：记录该点属于哪条线
    var seriesId: String
    var seriesName: String
    var seriesColor: Color
    
    // 用于排序
    static func < (lhs: ChartData, rhs: ChartData) -> Bool {
        return lhs.timestamp < rhs.timestamp
    }
}

/// 简单的颜色生成工具
extension Array where Element == Color {
    static let chartPalette: [Color] = [
        .red, .blue, .green, .orange, .purple, .cyan, .yellow, .pink
    ]
}

struct HistoricalChartView: View {
    @EnvironmentObject var dataManager: DataManager
    
    // 绘图数据总集
    @State private var allChartData: [ChartData] = []
    // 控制列表：允许用户修改选中状态
    @State private var lineDefs: [HistoryLineDef] = initialLineDefs
    
    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVStack(spacing: 16) {
                    // --- 顶部配置区域 ---
                    VStack(alignment: .leading, spacing: 10) {
                        Text("曲线选择")
                            .font(.headline)
                        
                        ForEach($lineDefs) { $def in
                            HStack {
                                // 颜色指示器 (基于其在数组内的索引分配颜色)
                                Circle()
                                    .fill(defColor(for: def))
                                    .frame(width: 12, height: 12)
                                
                                Text(def.displayName)
                                    .font(.subheadline)
                                
                                Spacer()
                                
                                // 状态同步
                                Image(
                                    systemName: def.isSelected ? "checkmark.circle.fill" : "circle"
                                )
                                .foregroundColor(def.isSelected ? .accentColor : .gray)
                                
                                // 点击切换选中状态
                                Button {
                                    def.isSelected.toggle()
                                } label: {}
                            }
                            .contentShape(Rectangle())
                        }
                        
                        // 刷新按钮
                        Button("刷新所有选中曲线") {
                            Task { await reloadHistory() }
                        }
                        .frame(maxWidth: .infinity)
                    }
                    .padding()
                    .background(Color(.systemGray6))
                    .cornerRadius(10)
                    
                    // --- 绘图区域 ---
                    VStack {
                        if allChartData.isEmpty {
                            ContentUnavailableView(
                                "暂无数据",
                                systemImage: "chart.xyaxis.line",
                                description: Text("请勾选上方曲线并点击刷新")
                            )
                            .frame(height: 300)
                        } else {
                            chartView
                                .frame(height: 300)
                                .padding(.vertical, 4)
                        }
                    }
                    .padding()
                    .background(Color(.systemBackground))
                    .cornerRadius(10)
                    .shadow(radius: 2)
                }
                .padding(.bottom, 30)
                
                Spacer(minLength: 0) // 让底部空间自适应
            }
            .navigationTitle("历史回放")
        }
        .task {
            await reloadHistory()
        }
        .onChange(of: dataManager.storageWindowHours) {
            Task {
                await reloadHistory()
            }
        }
    }
    
    // MARK: - 绘图逻辑
    @ViewBuilder
    private var chartView: some View {
        Chart(allChartData) { data in
            // 1. 面积图
            AreaMark(
                x: .value("", data.timestamp),
                y: .value("", data.value)
            )
            // 自动根据 seriesId 分配渐变颜色
            .foregroundStyle(
                .linearGradient(
                    colors: [
                        data.seriesColor.opacity(0.3), 
                        data.seriesColor.opacity(0.01)
                    ],
                    startPoint: .bottom, 
                    endPoint: .top
                )
            )
            
            // 2. 折线图
            LineMark(
                x: .value("", data.timestamp),
                y: .value("", data.value)
            )
            // 核心：.foregroundStyle(by:) 会根据 seriesId 自动生成不同颜色并渲染 Legend
            .foregroundStyle(by: .value("Series", data.seriesId))
            .interpolationMethod(.catmullRom)
            
            // 3. 数据点 (仅在数据极少时显示，防止图表拥挤)
            if allChartData.count < 100 {
                PointMark(x: .value("", data.timestamp), y: .value("", data.value))
                    .foregroundStyle(by: .value("Series", data.seriesId))
                    .symbolSize(6)
            }
        }
        .chartXAxis {
            AxisMarks(values: .stride(by: .minute, count: 5)) { _ in
                AxisValueLabel(format: .dateTime.hour())
            }
        }
    }
    
    // MARK: - 数据加载逻辑
    private func reloadHistory() async {
        let selectedDefs = lineDefs.filter { $0.isSelected }
        guard !selectedDefs.isEmpty else { return }
        
        allChartData.removeAll()
        
        // 并发获取所有选中曲线的数据
        await withTaskGroup(of: [ChartData].self) { group in
            for (index, def) in selectedDefs.enumerated() {
                // 分配颜色
                let color = Array.chartPalette[index % Array.chartPalette.count]
                
                group.addTask {
                    // 1. 调用 DataManager
                    let records = await dataManager.fetchHistoryFor(
                        key: def.dataId,
                        group: def.group,
                        lastHours: dataManager.storageWindowHours
                    )
                    
                    // 2. 映射为 ChartData，注入 seriesId 和 color
                    return records.map { record in
                        ChartData(
                            timestamp: record.timestamp,
                            value: record.value,
                            seriesId: def.dataId,
                            seriesName: def.displayName,
                            seriesColor: color
                        )
                    }
                }
            }
            
            // 合并所有数据
            for await result in group {
                allChartData.append(contentsOf: result)
            }
        }
        
        // 统一排序，保证 X 轴连贯性
        allChartData.sort()
    }
    
    // 辅助函数：根据 ID 返回对应颜色，确保图例颜色与 UI 一致
    private func defColor(for def: HistoryLineDef) -> Color {
        if let index = lineDefs.firstIndex(where: { $0.id == def.id }) {
            return Array.chartPalette[index % Array.chartPalette.count]
        }
        return .gray
    }
}

// MARK: - 模拟初始数据
var initialLineDefs: [HistoryLineDef] = [
    HistoryLineDef(dataId: "s7.1.temp1_pv", group: "goiot", name: "TC2102", displayName: "固定床预热"),
    HistoryLineDef(dataId: "s7.1.temp2_pv", group: "goiot", name: "TC2103", displayName: "固定床上热"),
    HistoryLineDef(dataId: "s7.1.temp3_pv", group: "goiot", name: "TC2104", displayName: "固定床中热"),
    HistoryLineDef(dataId: "s7.1.temp4_pv", group: "goiot", name: "TC2105", displayName: "固定床下热"),
]

// MARK: - 预览
struct HistoricalChartView_Previews: PreviewProvider {
    static var previews: some View {
        HistoricalChartView()
            .environmentObject(DataManager())
    }
}
