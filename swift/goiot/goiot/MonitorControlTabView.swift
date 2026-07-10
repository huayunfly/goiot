//
//  MonitorTabView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI

// MARK: - 分组数据模型
struct MonitorGroupSection: Identifiable {
    let id: String
    let title: String
    let items: [DataInfo]
}

// MARK: - 样式枚举
enum ControlItemStyle: String, CaseIterable, Identifiable {
    case compact = "紧凑"
    case standard = "标准"
    case detailed = "详细"
    var id: String { self.rawValue }
}

// MARK: - 主视图
struct MonitorControlTabView: View {
    @EnvironmentObject var userData: UserData
    @EnvironmentObject var dataManager: DataManager
    
    @State private var selectedStyle: ControlItemStyle = .standard
    @State private var isRefreshing = false
    //@State private var isExpanded = true
    // Manage multiple unfolding group status.
    @State private var expandedSections: Set<String> = Set()
    
    // Zone -> group -> [DataInfo]
    private var monitorSections: [MonitorGroupSection] {
        var sections: [MonitorGroupSection] = []
        print(dataManager.dataGroupIndexMap.count)
        for (zoneKey, _) in dataManager.dataGroupIndexMap {
            guard let selZone = dataManager.dataGroupIndexMap[zoneKey] else { continue }
            for (groupKey, _) in selZone {
                guard let indices = selZone[groupKey]?.values else { continue }
                
                let dataInfos = indices.compactMap { index in
                    dataManager.dataArray.indices.contains(index) ? dataManager.dataArray[index] : nil
                }.sorted { $0.id < $1.id }
                
                guard !dataInfos.isEmpty else { continue }
                let title = "\(zoneKey) - \(groupKey)"
                sections.append(MonitorGroupSection(id: "\(zoneKey).\(groupKey)", title: title, items: dataInfos))
            }
        }
        return sections
    }
    
    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVStack(spacing: 16) {
                    ForEach(monitorSections) { section in
                        // Binding：unfold the section according to the Set sign.
                        let isExpanded = Binding<Bool>(
                            get: { expandedSections.contains(section.id) },
                            set: { newIsExpanded in
                                withAnimation(.easeInOut) {
                                    if newIsExpanded {
                                        expandedSections.insert(section.id)
                                    } else {
                                        expandedSections.remove(section.id)
                                    }
                                }
                            }
                        )
                        
                        ControlCollapsibleSection(
                            title: section.title,
                            icon: "chart.xyaxis.line",
                            isExpanded: isExpanded,
                            items: section.items
                        ) { dataInfo in
                            ControlDataInfoCard(dataInfo: dataInfo, style: selectedStyle)
                        }
                    }
                    
                    if monitorSections.isEmpty {
                        ControlEmptyStateView() // 修正原代码中的 EmptyStateView 拼写
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
                        dataManager.StartRefreshData(token: userData.token, withZoneGroups: monitorSections.compactMap {section in section.id}, withTimeInterval: 5)
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
                        ForEach(ControlItemStyle.allCases) { style in
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
    } // NavigationStack
}

// MARK: - 可折叠分组容器
struct ControlCollapsibleSection<Content: View>: View {
    let title: String
    let icon: String
    @Binding var isExpanded: Bool
    let items: [DataInfo]
    let content: (DataInfo) -> Content
    
    var body: some View {
        VStack(spacing: 12) {
            ControlSectionHeader(title: title, icon: icon, isExpanded: $isExpanded, count: items.count)
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
struct ControlSectionHeader: View {
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
struct ControlDataInfoCard: View {
    @EnvironmentObject var userData: UserData
    @EnvironmentObject var dataManager: DataManager
    @ObservedObject var dataInfo: DataInfo
    let style: ControlItemStyle
    
    @State private var isEditing = false
    @State private var draftValue: String = ""
    @FocusState private var isFocused: Bool
    
    private var isWritable: Bool {
        dataInfo.readWriteType == .readWrite || dataInfo.readWriteType == .writeOnly
    }
    
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
    
    private func keyboardForType() -> UIKeyboardType {
        switch dataInfo.dtype {
        case .DF: return .decimalPad
        case .WUB, .WB, .DUB, .DB, .BB: return .numberPad
        case .BT: return .default
        case .STR: return .default
        }
    }
    
    private func startEditing() {
        draftValue = displayValue.replacingOccurrences(of: "ON", with: "1").replacingOccurrences(of: "OFF", with: "0")
        isEditing = true
        DispatchQueue.main.async { isFocused = true }
    }
    
    private func cancelEditing() {
        isEditing = false
        isFocused = false
        draftValue = ""
    }
    
    private func saveValue() async {
        var parsedValue: String? = nil
        
        switch dataInfo.dtype {
        case .DF: parsedValue = String(Double(draftValue) ?? dataInfo.fValue)
        case .WUB, .WB: parsedValue = String(Int16(draftValue).flatMap(Int64.init) ?? dataInfo.intValue)
        case .DUB, .DB: parsedValue = String(Int32(draftValue).flatMap(Int64.init) ?? dataInfo.intValue)
        case .BB: parsedValue = String(UInt8(draftValue).flatMap(UInt8.init) ?? dataInfo.byteValue)
        case .BT: parsedValue = String((Int(draftValue) ?? 0) > 0 ? 1 : 0)
        case .STR: parsedValue = draftValue.isEmpty ? dataInfo.strValue : draftValue
        }
        
        if let val = parsedValue {
            await dataManager.writeDataItem(dataInfo, token: userData.token, writeValue: val)
            isEditing = false
            isFocused = false
        } else {
            // 格式错误时保持编辑状态，方便用户修正
            print("格式错误")
            isFocused = true
        }
    }
    
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack(spacing: 12) {
                Circle()
                    .fill(statusColor)
                    .frame(width: 8, height: 8)
                    .shadow(color: statusColor.opacity(0.5), radius: 3)
                
                VStack(alignment: .leading, spacing: 2) {
                    Text(dataInfo.name)
                        .font(.headline)
                        .foregroundColor(.primary)
                    
                    if style != .compact {
                        Text(dataInfo.displayName)
                            .font(.subheadline)
                            .foregroundColor(.secondary)
                    }
                }
                
                Spacer()
 
                // 动态显示/编辑区域
                VStack(alignment: .trailing, spacing: 4) {
                    if isEditing {
                        HStack(spacing: 8) {
                            TextField("输入新值", text: $draftValue)
                                //.font(.system(.caption, design: .monospaced))
                                .keyboardType(keyboardForType())
                                .focused($isFocused)
                                .textFieldStyle(.plain)
                                .font(.system(.body, design: .monospaced)) // 调大字号，保持等宽便于数字对齐
                                .foregroundColor(.primary)
                                .frame(maxWidth: .infinity, alignment: .trailing)
                                .onSubmit { Task { await saveValue() } }
                                //.onExitCommand { cancelEditing() }
                            
                            Button {
                                Task { await saveValue() }
                            } label: {
                                Image(systemName: "checkmark.circle.fill")
                                    //.font(.title3) // 调大图标，与输入框视觉匹配
                                    .foregroundColor(.green)
                            }
                            
                            Button(role: .cancel) {
                                cancelEditing()
                            } label: {
                                Image(systemName: "xmark.circle.fill")
                                    //.font(.title3)
                                    .foregroundColor(.secondary)
                            }
                        }
                        .padding(.vertical, 2)
                    } else {
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
                    
                    if !isEditing && isWritable {
                        Button {
                            startEditing()
                        } label: {
                            Image(systemName: "pencil.circle")
                                .font(.caption)
                                .foregroundColor(.blue)
                        }
                    }
                }
            }
            .padding(16)
            
            if style == .detailed {
                HStack(spacing: 12) {
                    ControlInfoTag(title: "类型", value: typeDisplay)
                    ControlInfoTag(title: "地址", value: String(dataInfo.regiterAddress))
                    ControlInfoTag(title: "读写", value: isWritable ? "读写" : "只读")
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
        .animation(.easeInOut(duration: 0.2), value: isEditing)
    }
}

// MARK: - 信息标签
struct ControlInfoTag: View {
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
struct ControlEmptyStateView: View {
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
func controlFormatTimestamp(_ timestamp: Double) -> String {
    let date = Date(timeIntervalSince1970: timestamp)
    let formatter = DateFormatter()
    formatter.dateFormat = "HH:mm:ss"
    return formatter.string(from: date)
}

// MARK: - 预览
struct MonitorControlTabView_Previews: PreviewProvider {
    static var previews: some View {
        MonitorControlTabView()
            .environmentObject(UserData())
            .environmentObject(DataManager())
    }
}
