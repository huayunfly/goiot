//
//  TrendTabView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI
import Charts

// MARK: - 图表专用数据模型
struct TrendData: Identifiable, Comparable {
    var id = UUID()
    let timestamp: Date
    let value: Double
    let seriesKey: String
    let seriesName: String
    let color: Color
    
    static func < (lhs: Self, rhs: Self) -> Bool { lhs.timestamp < rhs.timestamp }
}

struct ChartLineDef: Identifiable, Equatable {
    var id = UUID()
    var dataId: String
    var group: String
    var name: String
    var displayName: String
    var isSelected: Bool = true
    
    static func == (lhs: ChartLineDef, rhs: ChartLineDef) -> Bool { lhs.id == rhs.id }
}

struct TrendChartGroup: Identifiable {
    var id = UUID()
    var title: String
    var lines: [ChartLineDef]
    var measurementUnit: HMIMeasurementUnit
}

struct TrendTabView: View {
    @EnvironmentObject var dataManager: DataManager
    @State private var chartGroups: [TrendChartGroup] = initialTrendChartGroups
    @State private var groupData: [UUID: [ChartData]] = [:]
    @State private var lastRefresh: Date = .now
    
    var body: some View {
        // ✅ TimelineView 每 5 秒提供一次新的 context
        TimelineView(.periodic(from: .now, by: 5)) { context in
            VStack(spacing: 16) {
                // 顶部状态栏
                HStack {
                    VStack(alignment: .leading) {
                        Text("实时趋势")
                            .font(.headline)
                        Text("数据窗口: 近2小时 | FIFO队列")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                    Spacer()
                    HStack(spacing: 12) {
                        Label("最近刷新", systemImage: "clock.badge.dot")
                        Text(lastRefresh, format: .dateTime.hour())
                            .monospacedDigit()
                    }
                    .font(.caption)
                    .padding(8)
                    .background(Color.gray.opacity(0.1))
                    .cornerRadius(8)
                }
                .padding(.horizontal)
                
                // 多图表滚动容器
                ScrollView {
                    LazyVStack(spacing: 20) {
                        if chartGroups.isEmpty {
                            ContentUnavailableView(
                                "等待数据注入",
                                systemImage: "arrow.down.to.line",
                                description: Text("请确保设备已连接并推送实时数据流")
                            )
                            .padding()
                        } else {
                            ForEach(chartGroups) { group in
                                TrendChartPanel(group: group, data: groupData[group.id] ?? [])
                            }
                        }
                    }
                    .padding(.horizontal)
                    .padding(.bottom, 40)
                }
            }
            .padding(.top)
            // 驱动数据预处理与 UI 刷新
            .task(id: context.date) {
                await refreshDataSource()
                lastRefresh = context.date
            }
        }
        .navigationTitle("实时趋势")
    }
    
    // MARK: - 数据源同步逻辑
    private func refreshDataSource() async {
        var newData: [UUID: [ChartData]] = [:]
        let colors = Array.chartPalette
        
        for group in chartGroups {
            var lineData: [ChartData] = []
            for (idx, line) in group.lines.enumerated() {
                let color = colors[idx % colors.count]
                let realTimeData = dataManager.computeRealtimeData(line.dataId, line.group)
                lineData.append(contentsOf: realTimeData.map { rtData in
                    ChartData(
                        timestamp: rtData.timestamp,
                        value: rtData.value,
                        seriesId: rtData.dataId,
                        seriesName: line.group,
                        seriesColor: color)
                })
            }
            newData[group.id] = lineData
        }
        // Uses animation to smooth the chart updating.
        withAnimation(.easeInOut(duration: 0.3)) {
            groupData = newData
        }
    }
}

// MARK: - 独立图表面板组件
private struct TrendChartPanel: View {
    let group: TrendChartGroup
    let data: [ChartData]
    
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Text(group.title)
                    .font(.subheadline.bold())
                    .foregroundStyle(.primary)
                Spacer()
                Text("\(group.lines.count) 条曲线")
                    .font(.caption.bold())
                    .foregroundStyle(.secondary)
            }
            
            // ✅ 核心图表区域
            Chart(data) { point in
                AreaMark(
                    x: .value("时间", point.timestamp),
                    y: .value("数值", point.value)
                )
                .foregroundStyle(
                    .linearGradient(
                        colors: [point.seriesColor.opacity(0.25), .clear],
                        startPoint: .bottom,
                        endPoint: .top
                    )
                )
                .interpolationMethod(.catmullRom)
                
                LineMark(
                    x: .value("时间", point.timestamp),
                    y: .value("数值", point.value)
                )
                .foregroundStyle(by: .value("Series", point.seriesId))
                .interpolationMethod(.catmullRom)
                .lineStyle(StrokeStyle(lineWidth: 2))
                
                // 仅显示最新 3 个点，避免大数据量卡顿
                PointMark(
                    x: .value("时间", point.timestamp),
                    y: .value("数值", point.value)
                )
                .foregroundStyle(by: .value("Series", point.seriesId))
                .symbolSize(6)
                //.filter { _ in data.suffix(3).contains(where: { $0.timestamp == point.timestamp }) }
            }
            .frame(height: 220)
            .chartXAxis {
                AxisMarks(values: .stride(by: .minute, count: 10)) { val in
                    AxisValueLabel(format: .dateTime.hour())
                }
            }
            .chartYAxis {
                AxisMarks { value in
                    AxisValueLabel(format: Decimal.FormatStyle.number.precision(.fractionLength(1)))
                    // Background grid line
                    AxisGridLine(stroke: StrokeStyle(lineWidth: 0.5)).foregroundStyle(Color.gray.opacity(0.3))
                }
            }
            .chartOverlay {_ in 
                GeometryReader { _ in
                    Rectangle().fill(.clear).contentShape(Rectangle())
                }
            }
        }
        .padding()
        .background(Color(.systemBackground))
        .cornerRadius(12)
        .shadow(color: .black.opacity(0.05), radius: 4, x: 0, y: 2)
    }
}

// MARK: - 初始数据配置
var initialTrendChartGroups: [TrendChartGroup] = [
    TrendChartGroup(
        title: "固定床温度监测",
        lines: [
            ChartLineDef(dataId: "s7.1.temp1_pv", group: "goiot", name: "TC2102", displayName: "预热区"),
            ChartLineDef(dataId: "s7.1.temp2_pv", group: "goiot", name: "TC2103", displayName: "上热区"),
            ChartLineDef(dataId: "s7.1.temp3_pv", group: "goiot", name: "TC2104", displayName: "中热区"),
            ChartLineDef(dataId: "s7.1.temp4_pv", group: "goiot", name: "TC2105", displayName: "下热区"),
        ], measurementUnit: .C
    ),
    TrendChartGroup(
        title: "固定床压力分布",
        lines: [
            ChartLineDef(dataId: "s7.1.pg_1", group: "goiot", name: "PIA1111", displayName: "顶部压力"),
            ChartLineDef(dataId: "s7.1.pg_2", group: "goiot", name: "PIA1121", displayName: "中部压力"),
            ChartLineDef(dataId: "s7.1.pg_3", group: "goiot", name: "PIA1131", displayName: "底部压力"),
        ], measurementUnit: .barA
    ),
    TrendChartGroup(
        title: "流体流量监控",
        lines: [
            ChartLineDef(dataId: "mfc.1.pv", group: "goiot", name: "FICA1111", displayName: "进料H2"),
            ChartLineDef(dataId: "mfc.2.pv", group: "goiot", name: "FICA1121", displayName: "进料CO"),
            ChartLineDef(dataId: "mfc.3.pv", group: "goiot", name: "FICA1131", displayName: "进料N2"),
        ], measurementUnit: .sccm
    )
]

struct TrendTabView_Previews: PreviewProvider {
    static var previews: some View {
        TrendTabView().environmentObject(DataManager())
    }
}
