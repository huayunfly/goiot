import SwiftUI

// MARK: - 图形化节点模型
struct HMIDeviceNode: Identifiable {
    let id: UUID
    let title: String
    let canvasPosition: CGPoint
    let pvInfoId: String
    let svInfoId: String?
    let iconType: HMIIconModule
}

enum HMIIconModule {
    case symbol(String)
    case image(String)
}

// MARK: - 主视图
struct HMIControlTabView: View {
    @EnvironmentObject var userData: UserData
    @EnvironmentObject var dataManager: DataManager
    
    @GestureState private var magnification: CGFloat = 1.0
    @State private var zoom: CGFloat = 1.0
    
    @State private var scale: CGFloat = 1.0
    @State private var selectedNode: HMIDeviceNode?
    @State private var livePV: [String: Double] = [:]
    
    // 画布物理尺寸
    private let canvasWidth: CGFloat = 1279
    private let canvasHeight: CGFloat = 1022
    
    // 用于管理缩放时的位置偏移，防止位置乱跳
    @State private var offset: CGSize = .zero
    
    // 缩放限幅
    private func clampZoom(_ value: CGFloat) -> CGFloat {
        return min(max(value, 0.4), 2.5)
    }
    
    private var hmiNodes: [HMIDeviceNode] {
        [
            HMIDeviceNode(id: UUID(), title: "反应釜 T-101", canvasPosition: CGPoint(x: 180, y: 600), pvInfoId: "T101_PV", svInfoId: "T101_SV", iconType: .image("temperature_controller")),
            HMIDeviceNode(id: UUID(), title: "进料泵 F-202", canvasPosition: CGPoint(x: 280, y: 600), pvInfoId: "F202_PV", svInfoId: "F202_SV", iconType: .image("flow_controller")),
            HMIDeviceNode(id: UUID(), title: "排气阀 V-303", canvasPosition: CGPoint(x: 380, y: 600), pvInfoId: "V303_PV", svInfoId: "V303_SV", iconType: .image("flow_controller")),
        ]
    }
    
    var body: some View {
        NavigationStack {
            ScrollView([.horizontal, .vertical]) {
                if #available(iOS 17.0, *) {
                    ZStack {
                        // 1. PID 背景图
                        Rectangle()
                            .fill(Color(.systemGray6))
                            .frame(width: canvasWidth, height: canvasHeight)
                            .overlay {
                                Image("pid_diagram")
                                    .resizable()
                                    .scaledToFit()
                                    .frame(width: canvasWidth, height: canvasHeight)
                                    .cornerRadius(12)
                            }
                            // 双击重置
                            .simultaneousGesture(
                                TapGesture(count: 2).onEnded {
                                    withAnimation(.spring(response: 0.35)) {
                                        self.scale = 1.0
                                        self.offset = .zero
                                    }
                                }
                            )
                        
                        // 2. 设备节点层
                        ForEach(hmiNodes) { (node: HMIDeviceNode) in
                            HMIDeviceNodeView(node: node, pvValue: livePV[node.pvInfoId] ?? 0.0, isEditing: selectedNode?.id == node.id)
                                .position(node.canvasPosition)
                                .onTapGesture {
                                    withAnimation {
                                        selectedNode = node
                                    }
                                }
                        }
                    }
                    // 设定 ZStack 的物理尺寸
                    .frame(width: canvasWidth, height: canvasHeight)
                    
                    // 应用偏移和缩放
                    .offset(offset)
                    .scaleEffect(scale)
                    
                    // 性能优化
                    .drawingGroup()
                    .animation(nil, value: scale) // 关闭系统默认动画，实现跟手缩放
                    
                    .simultaneousGesture(MagnificationGesture()
                        .updating($magnification) { value, state, transaction in
                            state = value
                        }
                        .onEnded { value in
                            zoom = clampZoom(zoom * value)
                            scale = zoom * magnification
                        }
                    )
                } else {
                    // Fallback on earlier versions
                }
            }
            // 占满全屏
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .ignoresSafeArea()
            .navigationTitle("图形化监控")
            .toolbar {
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button("重置") {
                        withAnimation(.easeOut(duration: 0.3)) {
                            self.scale = 1.0;
                            self.offset = .zero;
                        }
                    }
                }
            }
            .sheet(item: $selectedNode) { node in
                SVEditorSheet(node: node, token: userData.token).environmentObject(dataManager)
            }
        }
        .onAppear { startPVListener() }
    }
    
    private func startPVListener() {
        Task {
            // 确保UI总是在主线程上更新
            let actor = await MainActor.run { self }
            while !Task.isCancelled {
                for node in actor.hmiNodes {
                    if let dataInfo = actor.dataManager.dataArray.first(where: { $0.id == node.pvInfoId || $0.name == node.pvInfoId }) {
                        actor.livePV[node.pvInfoId] = dataInfo.fValue
                    }
                }
                try? await Task.sleep(for: .seconds(1.0))
            }
        }
    }
}

/// MARK: - 设备节点视图 (去背景透明版)
struct HMIDeviceNodeView: View {
    let node: HMIDeviceNode
    let pvValue: Double
    let isEditing: Bool
    
    private var imageIcon: some View {
        Group {
            switch node.iconType {
            case .symbol(let name):
                Image(systemName: name)
                    .font(.system(size: 28))
                    // 系统图标保留点击变色
                    .foregroundColor(isEditing ? .yellow : .primary)
            case .image(let name):
                Image(name)
                    .resizable()
                    .scaledToFit()
                    .frame(width: 36, height: 36)
                    // 自定义位图保持原色不变
            }
        }
    }
    
    var body: some View {
        VStack(spacing: 6) {
            imageIcon
                .padding(6) // 保留少量透明点击区域，方便手指点击
                .background(Color.clear)
                // 增加选中时的黄色虚线边框/光晕作为交互反馈
                .overlay(RoundedRectangle(cornerRadius: 12).stroke(isEditing ? Color.yellow : Color.clear, lineWidth: 2))
                .shadow(color: isEditing ? .orange.opacity(0.6) : .black.opacity(0.1), radius: isEditing ? 10 : 2, y: 2)
                .scaleEffect(isEditing ? 1.1 : 1.0)
                .animation(.spring(response: 0.25, dampingFraction: 0.8), value: isEditing)
           
            Label(node.title, systemImage: "house").labelStyle(.titleOnly).padding(2)       .font(.system(.caption, design: .rounded).bold())
            
            Text(String(format: "%.1f", pvValue))
                .font(.system(.caption, design: .rounded).bold())
                .foregroundColor(.primary)
                .padding(8)
                // PV 数值牌保留半透明灰底，确保在复杂 PID 背景图上依然清晰可读
                .background(Color(.systemBackground).opacity(0.85))
                .cornerRadius(4)
                .shadow(radius: 2)
        }
        // 节点整体外层透明化
        .background(Color.clear)
    }
}

// MARK: - SV 编辑面板
struct SVEditorSheet: View {
    let node: HMIDeviceNode
    let token: String
    @EnvironmentObject var dataManager: DataManager
    @State private var svInput: String = "0.00"
    @Environment(\.dismiss) var dismiss
    
    private var targetInfo: DataInfo? {
        guard let sid = node.svInfoId else { return nil }
        return dataManager.dataArray.first(where: { $0.id == sid || $0.name == sid })
    }
    
    var body: some View {
        NavigationStack {
            Form {
                Section(header: Text("当前设定值 (SV)")) {
                    HStack {
                        Text("目标值:")
                        Spacer()
                        Text(svInput)
                            .font(.title2.monospaced())
                            .foregroundColor(.blue)
                    }
                }
                Section {
                    TextField("输入新设定值", text: $svInput)
                        .keyboardType(.decimalPad)
                        .textFieldStyle(.roundedBorder)
                }
                Section {
                    Button("写入 PLC / 应用设定") {
                        submitSV()
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(.green)
                    .disabled(svInput.isEmpty || targetInfo == nil)
                }
            }
            .navigationTitle("\(node.title)")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("完成") { dismiss() }
                }
            }
            .onAppear {
                if let info = targetInfo {
                    svInput = String(format: "%.2f", info.fValue)
                }
            }
        }
    }
    
    private func submitSV() {
        guard let target = targetInfo else { return }
        Task {
            do {
                try await dataManager.writeDataItem(target, token: token, writeValue: svInput)
                print("✅ SV 写入成功: \(target.id) -> \(svInput)")
                dismiss()
            } catch {
                print("❌ SV 写入失败: \(error)")
            }
        }
    }
}

// MARK: - 预览
struct HMIControlTabView_Previews: PreviewProvider {
    static var previews: some View {
        HMIControlTabView()
            .environmentObject(UserData())
            .environmentObject(DataManager())
    }
}

