import SwiftUI
import Charts

// MARK: - 1. 数据模型
struct HistoryLineDef: Identifiable, Equatable {
    var id = UUID()
    var dataId: String
    var group: String
    var name: String
    var displayName: String
    var isSelected: Bool = true
    
    static func == (lhs: HistoryLineDef, rhs: HistoryLineDef) -> Bool { lhs.id == rhs.id }
}

struct HistoryChartGroup: Identifiable, Hashable {
    var id = UUID()
    var title: String
    var lines: [HistoryLineDef]
    var measurementUnit: HMIMeasurementUnit
    var isVisible: Bool = true
    
    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }
}

// 绘图数据点 (携带分组标识与颜色)
struct ChartData: Identifiable, Comparable {
    var id = UUID()
    var timestamp: Date
    var value: Double
    var seriesId: String
    var seriesName: String
    var seriesColor: Color
    
    static func < (lhs: ChartData, rhs: ChartData) -> Bool { lhs.timestamp < rhs.timestamp }
}

extension Array where Element == Color {
    static let chartPalette: [Color] = [
        .red, .blue, .green, .orange, .purple, .cyan, .yellow, .pink
    ]
}

// MARK: - 布局模式
enum ChartLayoutMode: String, CaseIterable, Identifiable {
    case vertical = "垂直列表"
    case grid = "仪表盘网格"
    var id: String { rawValue }
}

// MARK: - 2. 主视图
struct HistoricalChartView: View {
    @EnvironmentObject var dataManager: DataManager
    
    // 图表组配置
    @State private var chartGroups: [HistoryChartGroup] = initialChartGroups
    // 存储各组的绘图数据: GroupID -> [ChartData]
    @State private var groupData: [UUID: [ChartData]] = [:]
    
    // UI 状态
    @State private var layoutMode = ChartLayoutMode.grid
    @State private var isRefreshing = false
    
    var visibleGroups: [HistoryChartGroup] { chartGroups.filter(\.isVisible) }
    
    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVGrid(columns: gridColumns, spacing: 16) {
                    ForEach(chartGroups) { group in
                        GroupChartCard(
                            group: group,
                            data: groupData[group.id] ?? [],
                            onRefresh: { refreshGroup(group) }
                        )
                    }
                }
                .padding()
            }
            .navigationTitle("历史回放")
            .toolbar {
                ToolbarItemGroup(placement: .topBarTrailing) {
                    Picker("布局", selection: $layoutMode) {
                        ForEach(ChartLayoutMode.allCases) { mode in
                            Text(mode.rawValue).tag(mode)
                        }
                    }
                    .scaleEffect(0.8)
                    
                    Button {
                        Task { await refreshAll() }
                    } label: {
                        Image(systemName: isRefreshing ? "circle.dotted" : "arrow.clockwise")
                            .rotationEffect(Angle(degrees: isRefreshing ? 360 : 0), anchor: .center)
                    }
                }
            }
            .task { await refreshAll() }
            .onChange(of: dataManager.storageWindowHours) {
                Task { await refreshAll() }
            }
        }
    }
    
    // MARK: - 动态列配置
    private var gridColumns: [GridItem] {
        switch layoutMode {
        case .vertical:
            return [GridItem(.flexible(), spacing: 16)]
        case .grid:
            return [GridItem(.flexible(), spacing: 16),
                    GridItem(.flexible(), spacing: 16)]
        }
    }
    
    // MARK: - 数据加载
    private func refreshAll() async {
        isRefreshing = true
        defer { isRefreshing = false }
        
        var newData = [UUID: [ChartData]]()
        await withTaskGroup(of: (UUID, [ChartData]).self) { group in
            for g in visibleGroups {
                group.addTask {
                    var lines = [ChartData]()
                    let colors = Array.chartPalette
                    for (idx, line) in g.lines.enumerated() {
                        let color = colors[idx % colors.count]
                        let records = await dataManager.fetchHistoryFor(
                            key: line.dataId,
                            group: line.group,
                            lastHours: dataManager.storageWindowHours
                        )
                        let mapped = records.map {
                            ChartData(
                                timestamp: $0.timestamp,
                                value: $0.value,
                                seriesId: line.dataId,
                                seriesName: line.displayName,
                                seriesColor: color
                            )
                        }
                        lines.append(contentsOf: mapped)
                    }
                    lines.sort()
                    return (g.id, lines)
                }
            }
            for await (id, data) in group {
                newData[id] = data
            }
        }
        // Isolates the @state refresh, avoiding SwiftUI crash by the background thread upating.
        await MainActor.run { groupData = newData }
    }
    
    private func refreshGroup(_ group: HistoryChartGroup) {
        Task {
            isRefreshing = true
            defer { isRefreshing = false }
            
            var lines = [ChartData]()
            let colors = Array.chartPalette
            for (idx, line) in group.lines.enumerated() {
                let color = colors[idx % colors.count]
                let records = await dataManager.fetchHistoryFor(
                    key: line.dataId,
                    group: line.group,
                    lastHours: dataManager.storageWindowHours
                )
                let mapped = records.map {
                    ChartData(
                        timestamp: $0.timestamp,
                        value: $0.value,
                        seriesId: line.dataId,
                        seriesName: line.displayName,
                        seriesColor: color
                    )
                }
                lines.append(contentsOf: mapped)
            }
            lines.sort()
            await MainActor.run {
                groupData[group.id] = lines
            }
        }
    }
}

// MARK: - 3. 分组图表卡片组件
private struct GroupChartCard: View {
    let group: HistoryChartGroup
    let data: [ChartData]
    let onRefresh: () -> Void
    
    @State private var isExpanded = true
    
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            // 头部控制栏
            HStack {
                Text("\(group.title) 单位：\(String(describing: group.measurementUnit))")
                    .font(.headline)
                    .fontWeight(.semibold)
                
                Spacer()
                
                Button { onRefresh() } label: {
                    Image(systemName: "arrow.clockwise")
                        .symbolVariant(.fill)
                }
                .buttonStyle(.plain)
                
                // 展开/收起动画切换
                Image(systemName: isExpanded ? "chevron.up" : "chevron.down")
                    .symbolEffect(.bounce.down)
            }
            .padding(.horizontal, 12)
            .padding(.vertical, 8)
            .background(Color(.systemGray6))
            .cornerRadius(12)
            .contentShape(Rectangle())
            .onTapGesture { withAnimation(.smooth) { isExpanded.toggle() } }
            
            // 图表区域 (带平滑展开/收起)
            if isExpanded {
                ChartContainer(data: data)
                    .frame(height: 240)
                    .padding(.horizontal)
                    .padding(.bottom, 12)
            }
        }
        .background(Color(.systemBackground))
        .cornerRadius(12)
        .shadow(color: .black.opacity(0.08), radius: 4, x: 0, y: 2)
    }
}

// MARK: - 4. 图表渲染容器
private struct ChartContainer: View {
    let data: [ChartData]
    
    var body: some View {
        if data.isEmpty {
            ContentUnavailableView(
                "暂无数据",
                systemImage: "chart.xyaxis.line",
                description: Text("请检查数据源或点击刷新按钮")
            )
        } else {
            Chart(data) { point in
                // Draws mark point to further differentiate the data categories.
                AreaMark(
                    x: .value("", point.timestamp),
                    y: .value("", point.value)
                )
                .foregroundStyle(
                    .linearGradient(
                        colors: [
                            point.seriesColor.opacity(0.35),
                            point.seriesColor.opacity(0.02)
                        ],
                        startPoint: .bottom,
                        endPoint: .top
                    )
                )
                .interpolationMethod(.catmullRom)
                
                // Draws line by filtering data into multiple graph lines.
                LineMark(
                    x: .value("", point.timestamp),
                    y: .value("", point.value)
                )
                .foregroundStyle(by: .value("Series", point.seriesId))
                .interpolationMethod(.catmullRom)
                .lineStyle(StrokeStyle(lineWidth: 2))
                
                // 智能显示数据点（数据量适中时开启）
                if data.count < 150 {
                    PointMark(
                        x: .value("", point.timestamp),
                        y: .value("", point.value)
                    )
                    .foregroundStyle(by: .value("Series", point.seriesId))
                    .symbol(by: .value("Series", point.seriesId))
                    .symbolSize(6)
                }
            }
            .chartXAxis {
                AxisMarks(values: .stride(by: .minute, count: 10)) { _ in
                    AxisValueLabel(format: .dateTime.hour())
                }
            }
            .chartYAxis {
                AxisMarks { value in
                    AxisValueLabel(format: Decimal.FormatStyle.number.precision(.fractionLength(1)))
                    // Background grid line
                    AxisGridLine(stroke: StrokeStyle(lineWidth: 0.5))
                        .foregroundStyle(Color.gray.opacity(0.3))
                }
            }
        }
    }
}

// MARK: - 初始数据配置
var initialChartGroups: [HistoryChartGroup] = [
    HistoryChartGroup(
        title: "固定床温度监测",
        lines: [
            HistoryLineDef(dataId: "s7.1.temp1_pv", group: "goiot", name: "TC2102", displayName: "预热区"),
            HistoryLineDef(dataId: "s7.1.temp2_pv", group: "goiot", name: "TC2103", displayName: "上热区"),
            HistoryLineDef(dataId: "s7.1.temp3_pv", group: "goiot", name: "TC2104", displayName: "中热区"),
            HistoryLineDef(dataId: "s7.1.temp4_pv", group: "goiot", name: "TC2105", displayName: "下热区"),
        ], measurementUnit: .C
    ),
    HistoryChartGroup(
        title: "固定床压力分布",
        lines: [
            HistoryLineDef(dataId: "s7.1.pg_1", group: "goiot", name: "PIA1111", displayName: "顶部压力"),
            HistoryLineDef(dataId: "s7.1.pg_2", group: "goiot", name: "PIA1121", displayName: "中部压力"),
            HistoryLineDef(dataId: "s7.1.pg_3", group: "goiot", name: "PIA1131", displayName: "底部压力"),
        ], measurementUnit: .barA
    ),
    HistoryChartGroup(
        title: "流体流量监控",
        lines: [
            HistoryLineDef(dataId: "mfc.1.pv", group: "goiot", name: "FICA1111", displayName: "进料H2"),
            HistoryLineDef(dataId: "mfc.2.pv", group: "goiot", name: "FICA1121", displayName: "进料CO"),
            HistoryLineDef(dataId: "mfc.3.pv", group: "goiot", name: "FICA1131", displayName: "进料N2"),
        ], measurementUnit: .sccm
    )
]

// MARK: - 预览

struct HistoricalChartView_Previews: PreviewProvider {
    static var previews: some View {
        HistoricalChartView()
            .environmentObject(DataManager())
    }
}
