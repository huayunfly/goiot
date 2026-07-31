//
//  HMIAlarmingTabView.swift
//  goiot
//
//  Created by YUN HUA on 2026/7/29.
//

import SwiftUI

// Alarming datainfo
struct AlarmDataInfo: Identifiable, Hashable {
    let id: UUID
    let dataId: String
    let group: String
    let name: String
    let displayName: String
    var value: Double
    let unit: HMIMeasurementUnit
    var lowerLimit: Double
    var upperLimit: Double
    var lastUpdateTimestamp: Date? = nil
    
    var isAlarm: Bool {
        return value < lowerLimit || value > upperLimit
    }
    
    var alarmDescription: String {
        if value > upperLimit { return "⚠️ 超限报警" }
        if value < lowerLimit { return "⚠️ 欠限报警" }
        return "✅ 正常"
    }
}

// MARK: - 报警通知模型
struct AlarmNotification: Identifiable, Equatable {
    let id = UUID()
    let message: String
    let timestamp: Date
}

// MARK: - 报警监控主视图
struct HMIAlarmView: View {
    @State private var dataPoints: [AlarmDataInfo] = initialTestData
    @State private var activeAlarmIds: Set<UUID> = []
    @State private var alarmLog: [AlarmNotification] = []
    @State private var currentToast: AlarmNotification?
    @State private var showAlarmHistory = false
    @State private var selectedForEdit: AlarmDataInfo?
    
    @EnvironmentObject var dataManager: DataManager
    
    private func startMonitoring() {
        Task {
            while !Task.isCancelled {
                await MainActor.run {
                    for info in dataPoints {
                        let (timestamp, value) = dataManager.getDataPointValue(info.dataId, info.group)
                        updateDataPoint(id: info.id, newValue: value, timestamp: timestamp)
                    }
                }
                try await Task.sleep(nanoseconds: 1_500_000_000) // 1.5秒轮询
            }
        }
    }
    
    // 更新单个数据点并检测阈值跃迁
    @MainActor
    private func updateDataPoint(id: UUID, newValue: Double, timestamp: Date) {
        guard let index = dataPoints.firstIndex(where: { $0.id == id }) else { return }
        let oldInfo = dataPoints[index]
        
        dataPoints[index].value = newValue
        dataPoints[index].lastUpdateTimestamp = timestamp
        
        let newInfo = dataPoints[index]
        
        // 仅在状态从正常变为报警时触发提示，避免重复提示
        if newInfo.isAlarm && !oldInfo.isAlarm {
            activeAlarmIds.insert(id)
            let msg = "\(newInfo.name): \(String(format: "%.2f", newValue)) \(newInfo.unit) \(newInfo.alarmDescription)"
            let notification = AlarmNotification(message: msg, timestamp: timestamp)
            alarmLog.insert(notification, at: 0)
            currentToast = notification
        }
        
        // 报警恢复时移除标记
        if !newInfo.isAlarm && oldInfo.isAlarm {
            activeAlarmIds.remove(id)
        }
    }
    
    private func saveEditedLimits(_ updatedInfo: AlarmDataInfo) {
        if let index = dataPoints.firstIndex(where: { $0.id == updatedInfo.id }) {
            dataPoints[index] = updatedInfo
        }
    }
    
    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVStack(spacing: 12) {
                    ForEach(dataPoints) { info in
                        AlarmDataPointCard(info: info) { selectedInfo in
                            selectedForEdit = selectedInfo
                        }
                    }
                }
                .padding()
            }
            .navigationTitle("报警监控")
            .toolbar {
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button(action: { showAlarmHistory = true }) {
                        Image(systemName: "bell.fill")
                            .accessibilityLabel("查看报警记录")
                    }
                }
            }
            .overlay(alarmToastOverlay, alignment: .top)
            .sheet(isPresented: $showAlarmHistory) {
                AlarmHistoryView(alarms: alarmLog, onClear: {
                    alarmLog.removeAll()
                    activeAlarmIds.removeAll()
                })
            }
            .sheet(item: $selectedForEdit) { selectedItem in
                LimitEditSheet(info: selectedItem, onSave: saveEditedLimits)
            }
            .task {
                startMonitoring()
            }
        }
    }
    
    // 顶部滑入 Toast 提示
    private var alarmToastOverlay: some View {
        VStack {
            if let toast = currentToast {
                ToastView(message: toast.message)
                    .onTapGesture { currentToast = nil }
                    .animation(.easeInOut, value: currentToast)
                    .padding(.top, 60)
            }
            Spacer()
        }
    }
}

// MARK: - 数据点卡片
struct AlarmDataPointCard: View {
    let info: AlarmDataInfo
    var onEditLimits: (AlarmDataInfo) -> Void
    
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Text(info.name)
                    .font(.headline)
                Spacer()
                Button(action: { onEditLimits(info) }) {
                    Image(systemName: "slider.horizontal.3")
                        .foregroundStyle(.secondary)
                        .padding(.horizontal, 6)
                        .padding(.vertical, 2)
                        .background(Color.secondary.opacity(0.1), in: Capsule())
                }
                Text("\((String(format: "%.2f", info.value))) \(String(describing: info.unit))")
                    .font(.title2.monospacedDigit())
                    .foregroundStyle(info.isAlarm ? .red : .primary)
            }
            
            HStack {
                Text("下限: \(String(format: "%.2f", info.lowerLimit))")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Spacer()
                Text(info.isAlarm ? "🔴 报警中" : "🟢 正常")
                    .font(.caption.bold())
                    .foregroundStyle(info.isAlarm ? .red : .green)
                Spacer()
                Text("上限: \(String(format: "%.2f", info.upperLimit))")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            
            if let ts = info.lastUpdateTimestamp {
                HStack {
                    Image(systemName: "clock")
                        .font(.caption2)
                        .foregroundStyle(.tertiary)
                    Text(ts, style: .time)
                        .font(.caption2)
                        .foregroundStyle(.tertiary)
                    Spacer()
                    Text("数据刷新中")
                        .font(.caption2)
                        .foregroundStyle(.tertiary)
                }
            } else {
                Text("等待数据回传...")
                    .font(.caption2)
                    .foregroundStyle(.quaternary)
                    .frame(maxWidth: .infinity, alignment: .leading)
            }
        }
        .padding()
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(info.isAlarm ? Color.red.opacity(0.15) : Color(.systemGray6))
        .cornerRadius(12)
        .overlay(
            RoundedRectangle(cornerRadius: 12)
                .stroke(info.isAlarm ? Color.red : Color.clear, lineWidth: 2)
        )
    }
}

// MARK: - Toast 提示组件
struct ToastView: View {
    var message: String
    
    var body: some View {
        Text(message)
            .font(.caption.bold())
            .foregroundColor(.white)
            .padding(.horizontal, 16)
            .padding(.vertical, 10)
            .background(Color.red.opacity(0.9))
            .cornerRadius(12)
            .shadow(radius: 4)
            .transition(.move(edge: .top).combined(with: .opacity))
    }
}

// MARK: - 报警记录弹窗
struct AlarmHistoryView: View {
    let alarms: [AlarmNotification]
    let onClear: () -> Void
    @Environment(\.dismiss) private var dismiss
    
    var body: some View {
        NavigationStack {
            List {
                if alarms.isEmpty {
                    Text("暂无报警记录")
                        .foregroundStyle(.secondary)
                        .frame(maxWidth: .infinity)
                        .listRowSeparator(.hidden)
                } else {
                    ForEach(alarms.reversed()) { alarm in
                        VStack(alignment: .leading, spacing: 4) {
                            Text(alarm.message)
                                .font(.body)
                            Text(alarm.timestamp, style: .time)
                                .font(.caption2)
                                .foregroundStyle(.tertiary)
                        }
                        .padding(.vertical, 4)
                    }
                }
                
                Section {
                    Button("清空报警记录") {
                        onClear()
                    }
                    .foregroundStyle(.red)
                }
            }
            .navigationTitle("报警记录")
            .toolbar {
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button("完成") { dismiss() }
                }
            }
        }
    }
}

// MARK: - 阈值编辑 Sheet
struct LimitEditSheet: View {
    var info: AlarmDataInfo
    var onSave: (AlarmDataInfo) -> Void
    @Environment(\.dismiss) private var dismiss
    @State private var lowerTemp: String = ""
    @State private var upperTemp: String = ""
    
    private var isValid: Bool {
        let lower = Double(lowerTemp) ?? Double.infinity
        let upper = Double(upperTemp) ?? -Double.infinity
        return lower < upper
    }
    
    var body: some View {
        NavigationStack {
            Form {
                Section("数据点: \(info.name)") {
                    Text(info.dataId)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                
                Section("报警阈值设置") {
                    TextField("下限值", text: $lowerTemp)
                        .keyboardType(.decimalPad)
                    TextField("上限值", text: $upperTemp)
                        .keyboardType(.decimalPad)
                    
                    if !isValid {
                        Text("下限必须小于上限")
                            .font(.caption)
                            .foregroundStyle(.red)
                    }
                }
            }
            .navigationTitle("编辑阈值")
            .toolbar {
                ToolbarItem(placement: .navigationBarLeading) {
                    Button("取消") { dismiss() }
                }
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button("保存") {
                        if let l = Double(lowerTemp), let u = Double(upperTemp), l < u {
                            var mutableInfo = info // Self is immutalbel, so creating a mutable local copy
                            mutableInfo.lowerLimit = l
                            mutableInfo.upperLimit = u
                            onSave(mutableInfo)
                            dismiss()
                        }
                    }
                    .disabled(!isValid)
                }
            }
            .onAppear {
                lowerTemp = String(format: "%.2f", info.lowerLimit)
                upperTemp = String(format: "%.2f", info.upperLimit)
            }
        }
    }
}

// MARK: - 测试数据
var initialTestData: [AlarmDataInfo] = [
    AlarmDataInfo(id: UUID(), dataId: "s7.1.temp1_pv", group: "goiot", name: "TC2102", displayName: "预热区", value: 22.5, unit: .C, lowerLimit: 0.0, upperLimit: 85.0),
    AlarmDataInfo(id: UUID(), dataId: "s7.1.temp2_pv", group: "goiot", name: "TC2103", displayName: "上热区", value: 0.85, unit: .C, lowerLimit: 0.0, upperLimit: 85.0),
    AlarmDataInfo(id: UUID(), dataId: "s7.1.temp3_pv", group: "goiot", name: "TC2104", displayName: "中热区", value: 480, unit: .C, lowerLimit: 0.0, upperLimit: 85.0),
    AlarmDataInfo(id: UUID(), dataId: "s7.1.temp4_pv", group: "goiot", name: "TC2105", displayName: "下热区", value: 12.3, unit: .C, lowerLimit: 0.0, upperLimit: 85.0)
]

// MARK: - Preview
struct HMIAlarmingView_Previews: PreviewProvider {
    static var previews: some View {
        HMIAlarmView()
            .environmentObject(DataManager())
    }
}
