//
//  MonitorTabView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI

// MARK: - 样式枚举
enum ItemStyle: String, CaseIterable, Identifiable {
    case compact = "紧凑"
    case standard = "标准"
    case detailed = "详细"
    var id: String { self.rawValue }
}

// MARK: - 主视图
struct MonitorTabView: View {
    @EnvironmentObject var userData: UserData
    @EnvironmentObject var dataManager: DataManager
    
    @State private var selectedStyle: ItemStyle = .standard
    @State private var isRefreshing = false
    @State private var isExpanded = true
    
    // 安全提取数据，避免越界
    private var monitorData: [DataInfo] {
        guard let indices = dataManager.dataGroupIndexMap["goiot"]?["fp2"]?.values else { return [] }
        return indices.compactMap { index in
            dataManager.dataArray.indices.contains(index) ? dataManager.dataArray[index] : nil
        }.sorted { $0.id < $1.id }
    }
    
    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVStack(spacing: 16) {
                    
                    Button {
                        isRefreshing = true
                        dataManager.StartRefreshData(token: userData.token, withTimeInterval: 5)
                        DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
                            isRefreshing = false
                        }
                    } label: {
                        Image(systemName: "arrow.clockwise")
                            .rotationEffect(.degrees(isRefreshing ? 360 : 0))
                            .animation(.easeInOut(duration: 1.0).repeatForever(autoreverses: isRefreshing), value: isRefreshing)
                    }
                    // 数据分组卡片
                    CollapsibleSection(
                        title: "实时监测数据",
                        icon: "chart.xyaxis.line",
                        isExpanded: $isExpanded,
                        items: monitorData
                    ) { dataInfo in
                        DataInfoCard(dataInfo: dataInfo, style: selectedStyle)
                    }
                    
                    // 空状态提示
                    if monitorData.isEmpty {
                        EmptyStateView()
                    }
                }
                .padding(.horizontal)
                .padding(.bottom, 30)
            }
            .navigationTitle("设备监控中心")
            .toolbar {
                ToolbarItemGroup(placement: .topBarLeading) {
                    Button {
                        isRefreshing = true
                        dataManager.StartRefreshData(token: userData.token, withTimeInterval: 5)
                        DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
                            isRefreshing = false
                        }
                    } label: {
                        Image(systemName: "arrow.clockwise")
                            .rotationEffect(.degrees(isRefreshing ? 360 : 0))
                            .animation(.easeInOut(duration: 1.0).repeatForever(autoreverses: isRefreshing), value: isRefreshing)
                    }
                }
                
                ToolbarItem(placement: .topBarTrailing) {
                    Menu {
                        ForEach(ItemStyle.allCases) { style in
                            Button {
                                selectedStyle = style
                            } label: {
                                Label(style.rawValue, systemImage: style == selectedStyle ? "checkmark.circle.fill" : "circle")
                            }
                        }
                    } label: {
                        Image(systemName: "slider.horizontal.3")
                    }
                }
            }
        }
    }
}

// MARK: - 可折叠分组容器
struct CollapsibleSection<Content: View>: View {
    let title: String
    let icon: String
    @Binding var isExpanded: Bool
    let items: [DataInfo]
    let content: (DataInfo) -> Content
    
    var body: some View {
        VStack(spacing: 12) {
            SectionHeader(title: title, icon: icon, isExpanded: $isExpanded, count: items.count)
            
            if isExpanded {
                ForEach(items, id: \.id) { item in
                    content(item)
                }
            }
        }
        .animation(.spring(response: 0.3, dampingFraction: 0.8), value: isExpanded)
    }
}

// MARK: - 分组头部
struct SectionHeader: View {
    let title: String
    let icon: String
    @Binding var isExpanded: Bool
    let count: Int
    
    var body: some View {
        HStack {
            Image(systemName: icon)
                .font(.title3)
                .foregroundColor(.white)
                .padding(10)
                .background(Color.blue.gradient)
                .cornerRadius(10)
            
            Text(title)
                .font(.headline)
                .foregroundColor(.primary)
            
            Spacer()
            
            Text("\(count) 项")
                .font(.caption)
                .foregroundColor(.secondary)
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
                .background(Color.gray.opacity(0.1))
                .cornerRadius(8)
            
            Button(action: { isExpanded.toggle() }) {
                Image(systemName: isExpanded ? "chevron.up" : "chevron.down")
                    .font(.caption)
                    .foregroundColor(.blue)
            }
        }
        .padding(.vertical, 8)
    }
}

// MARK: - 数据卡片行视图
struct DataInfoCard: View {
    @ObservedObject var dataInfo: DataInfo
    let style: ItemStyle
    
    private var displayValue: String {
        switch dataInfo.dtype {
        case .DF: return String(format: "%.2f", dataInfo.fValue)
        case .WUB, .WB, .DUB, .DB: return String(dataInfo.intValue)
        case .BB: return String(dataInfo.byteValue)
        case .BT: return dataInfo.boolValue ? "ON" : "OFF"
        case .STR: return dataInfo.strValue
        }
    }
    
    private var statusColor: Color {
        dataInfo.result >= 0 ? .green : .red
    }
    
    private var typeDisplay: String {
        switch dataInfo.dtype {
        case .DF: return "浮点"
        case .WUB: return "无符号16位"
        case .WB: return "有符号16位"
        case .DUB: return "无符号32位"
        case .DB: return "有符号32位"
        case .BB: return "字节"
        case .BT: return "位状态"
        case .STR: return "字符串"
        }
    }
    
    private func valueText(_ value: String) -> some View {
        Text(value)
            .font(.system(.callout, design: .monospaced))
            .foregroundColor(.primary)
    }
    
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack(spacing: 12) {
                // 状态指示灯
                Circle()
                    .fill(statusColor)
                    .frame(width: 8, height: 8)
                    .shadow(color: statusColor.opacity(0.5), radius: 3)
                
                VStack(alignment: .leading, spacing: 2) {
                    Text(dataInfo.name)
                        .font(.headline)
                        .foregroundColor(.primary)
                    
                    if style != .compact {
                        Text(dataInfo.id)
                            .font(.caption2)
                            .foregroundColor(.secondary)
                    }
                }
                
                Spacer()
 
                VStack(alignment: .trailing, spacing: 2) {
                    Text(displayValue)
                        .font(style == .compact ? .title3 : .title2)
                        .fontWeight(.bold)
                        .foregroundColor(statusColor)
                    
                    if style != .compact {
                        Text(formatTimestamp(dataInfo.timestamp))
                            .font(.caption2)
                            .foregroundColor(.secondary)
                    }
                }
            }
            .padding(16)
            
            // 详细模式扩展信息
            if style == .detailed {
                HStack(spacing: 12) {
                    InfoTag(title: "类型", value: typeDisplay)
                    InfoTag(title: "地址", value: String(dataInfo.regiterAddress))
                    InfoTag(title: "读写", value: dataInfo.readWriteType == .readOnly ? "只读" : "读写")
                }
                .padding(.horizontal, 16)
                .padding(.vertical, 8)
                .background(Color(.systemGray6))
                .cornerRadius(8)
            }
        }
        .background(Color(.systemBackground))
        .cornerRadius(12)
        .shadow(color: Color.black.opacity(0.05), radius: 4, x: 0, y: 2)
        .overlay(
            RoundedRectangle(cornerRadius: 12)
                .stroke(Color.gray.opacity(0.15), lineWidth: 1)
        )
    }
}

// MARK: - 信息标签
struct InfoTag: View {
    let title: String
    let value: String
    
    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(title)
                .font(.caption2)
                .foregroundColor(.secondary)
            Text(value)
                .font(.caption)
                .fontWeight(.medium)
                .foregroundColor(.primary)
        }
        .frame(maxWidth: .infinity, alignment: .leading)
    }
}

// MARK: - 空状态视图
struct EmptyStateView: View {
    var body: some View {
        VStack(spacing: 12) {
            Image(systemName: "wrench.and.adjustments")
                .font(.largeTitle)
                .foregroundColor(.gray)
            Text("暂无监测数据")
                .font(.headline)
                .foregroundColor(.secondary)
            Text("请检查设备连接或点击左上角刷新")
                .font(.subheadline)
                .foregroundColor(.secondary)
                .multilineTextAlignment(.center)
        }
        .padding(40)
        .background(Color(.systemGray6))
        .cornerRadius(16)
    }
}

// MARK: - 辅助函数
func formatTimestamp(_ timestamp: Double) -> String {
    let date = Date(timeIntervalSince1970: timestamp)
    let formatter = DateFormatter()
    formatter.dateFormat = "HH:mm:ss"
    return formatter.string(from: date)
}

// MARK: - 预览
struct MonitorTabView_Previews: PreviewProvider {
    static var previews: some View {
        MonitorTabView()
            .environmentObject(UserData())
            .environmentObject(DataManager())
    }
}
