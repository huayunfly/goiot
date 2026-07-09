import SwiftUI

// MARK: - 页面配置模型
struct HMIPageConfig: Identifiable, Hashable {
    let id: UUID
    let title: String
    let description: String
    // UI
    let themeColor: Color
    let gradientColors: [Color]
    // HMI PID diagram name, device nodes
    let canvasName: String
    let hmiNodes: [HMIDeviceNode]
}


// MARK: - 复合主视图 (HMICompoundTabView)
struct HMICompoundTabView: View {
    @State private var selectedPage: HMIPageConfig?
    // Use @Namespace for the animation of "fold/unfold" views (like the system Photos)
    @Namespace private var layoutAnimation
    
    private let hmiNodes1: [HMIDeviceNode] = [
        HMIDeviceNode(id: UUID(), name: "FICA1111", title: "固定床H2质量流量控制器", canvasPosition: CGPoint(x: 302, y: 187), pvInfoId: "goiot.mfc.1.pv", svInfoId: "goiot.mfc.1.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1121", title: "固定床CO质量流量控制器", canvasPosition: CGPoint(x: 302, y: 288), pvInfoId: "goiot.mfc.2.pv", svInfoId: "goiot.mfc.2.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1131", title: "固定床N2质量流量控制器", canvasPosition: CGPoint(x: 302, y: 389), pvInfoId: "goiot.mfc.3.pv", svInfoId: "goiot.mfc.3.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1141", title: "固定床CO2质量流量控制器", canvasPosition: CGPoint(x: 302, y: 490), pvInfoId: "goiot.mfc.4.pv", svInfoId: "goiot.mfc.4.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1151", title: "固定床C2H4质量流量控制器", canvasPosition: CGPoint(x: 302, y: 591), pvInfoId: "goiot.mfc.5.pv", svInfoId: "goiot.mfc.5.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1511", title: "釜H2质量流量控制器", canvasPosition: CGPoint(x: 812, y: 232), pvInfoId: "goiot.mfc.6.pv", svInfoId: "goiot.mfc.6.pv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1521", title: "釜CO质量流量控制器", canvasPosition: CGPoint(x: 812, y: 333), pvInfoId: "goiot.mfc.7.pv", svInfoId: "goiot.mfc.7.pv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1531", title: "釜N2质量流量控制器", canvasPosition: CGPoint(x: 812, y: 434), pvInfoId: "goiot.mfc.8.pv", svInfoId: "goiot.mfc.8.pv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1541", title: "釜CO2质量流量控制器", canvasPosition: CGPoint(x: 812, y: 535), pvInfoId: "goiot.mfc.9.pv", svInfoId: "goiot.mfc.9.pv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "FICA1551", title: "釜C2H4质量流量控制器", canvasPosition: CGPoint(x: 812, y: 636), pvInfoId: "goiot.mfc.10.pv", svInfoId: "goiot.mfc.10.pv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .SCCM),
        HMIDeviceNode(id: UUID(), name: "PIA1111", title: "H2气源压力", canvasPosition: CGPoint(x: 161, y: 184), pvInfoId: "goiot.s7.1.pg_1", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .BARA),
        HMIDeviceNode(id: UUID(), name: "PIA1121", title: "CO气源压力", canvasPosition: CGPoint(x: 161, y: 285), pvInfoId: "goiot.s7.1.pg_2", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .BARA),
        HMIDeviceNode(id: UUID(), name: "PIA1131", title: "N2气源压力", canvasPosition: CGPoint(x: 161, y: 386), pvInfoId: "goiot.s7.1.pg_3", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .BARA),
        HMIDeviceNode(id: UUID(), name: "PIA1141", title: "CO2气源压力", canvasPosition: CGPoint(x: 161, y: 487), pvInfoId: "goiot.s7.1.pg_4", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .BARA),
        HMIDeviceNode(id: UUID(), name: "PIA1151", title: "C2H4气源压力", canvasPosition: CGPoint(x: 161, y: 588), pvInfoId: "goiot.s7.1.pg_5", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .BARA),
    ]
    
    private let hmiNodes2: [HMIDeviceNode] = [
    ]
    
    private let hmiNodes3: [HMIDeviceNode] = [
    ]
    
    // 模拟HMI 监控页面配置
    private var pages: [HMIPageConfig] {
        [
            HMIPageConfig(id: UUID(), title: "系统进料", description: "固定床和釜输入", themeColor: .blue, gradientColors: [.blue, .cyan], canvasName: "837_layout1", hmiNodes: hmiNodes1),
            HMIPageConfig(id: UUID(), title: "反应过程", description: "固定床和釜过程控制", themeColor: .green, gradientColors: [.green, .mint], canvasName: "837_layout2", hmiNodes: hmiNodes2),
            HMIPageConfig(id: UUID(), title: "物质分析", description: "在线气体分析", themeColor: .orange, gradientColors: [.orange, .yellow], canvasName: "837_layout3", hmiNodes: hmiNodes3)
        ]
    }
    
    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVGrid(columns: [GridItem(.flexible()), GridItem(.flexible())], spacing: 16) {
                    ForEach(pages) { page in
                        // 图框卡片视图
                        HMILibraryCardView(page: page, isExpanded: selectedPage?.id == page.id)
                            // 点击图框：全屏放大
                            .onTapGesture {
                                withAnimation(.spring(response: 0.35, dampingFraction: 0.8)) {
                                    selectedPage = page
                                }
                            }
                            // 动画锚点, 两个或多个视图共享一个几何形状
                            .matchedGeometryEffect(id: page.id, in: layoutAnimation)
                    }
                }
                .padding()
            }
            .navigationTitle("HMI 工艺总览")
            .toolbar { ToolbarItem(placement: .principal) { Text("工艺监控").font(.headline) } }
            // 全屏覆写层：展示放大的 HMIControlTabView
            .fullScreenCover(item: $selectedPage) { page in
                // 全屏容器：负责提供导航栏和返回按钮，内部嵌入具体的视图
                HMIFullscreenContainer(currentPage: page) {
                    selectedPage = nil
                }
            }
        }
    }
}

// 全屏容器视图
struct HMIFullscreenContainer: View {
    let currentPage: HMIPageConfig
    let onDismiss: () -> Void
    @Namespace var layoutAnimation // 与父视图共享的动画命名空间
    
    var body: some View {
        NavigationStack {
            // 这里实例化具体的 HMIControlTabView
            // 根据 currentPage.id 传入不同的 pid_diagram 文件名或节点数据
            HMIControlTabView(canvasName: currentPage.canvasName, hmiNodes: currentPage.hmiNodes)

            // 全屏专属 Toolbar：返回按钮
            .toolbar {
                ToolbarItem(placement: .navigationBarLeading) {
                    Button {
                        withAnimation(.spring(response: 0.35)) {
                            onDismiss()
                        }
                    } label: {
                        Label("返回概览", systemImage: "text.magnifyingglass")
                    }
                }
            }
            .toolbarBackground(Color(UIColor.systemBackground), for: .navigationBar)
        }
        // 动画锚点 (与父视图匹配，实现平滑过渡)
        .matchedGeometryEffect(id: currentPage.id, in: layoutAnimation)
    }
}

// 网格图框卡片 (模拟不同的图框内容)
struct HMILibraryCardView: View {
    let page: HMIPageConfig
    let isExpanded: Bool
    
    var body: some View {
        VStack(spacing: 12) {
            // 图框封面区域
            ZStack {
                LinearGradient(
                    colors: page.gradientColors,
                    startPoint: .topLeading,
                    endPoint: .bottomTrailing
                )
                .cornerRadius(16)
                
                // 模拟图框内的缩略图图标
                Image(systemName: isExpanded ? "arrow.up.right" : "wrench.and.screwdriver")
                    .font(.system(size: 35))
                    .foregroundStyle(.white.opacity(0.9))
                    .animation(.smooth, value: isExpanded)
            }
            .frame(height: 140)
            
            VStack(spacing: 4) {
                Text(page.title)
                    .font(.system(.subheadline, design: .rounded).bold())
                    .foregroundColor(.primary)
                Text(page.description)
                    .font(.caption)
                    .foregroundColor(.secondary)
            }
        }
        .padding(.vertical, 8)
        .background(Color(.systemBackground))
        .cornerRadius(16)
        .shadow(color: isExpanded ? page.themeColor.opacity(0.4) : .black.opacity(0.08), radius: isExpanded ? 10 : 4, y: 4)
        .animation(.bouncy(duration: 0.3), value: isExpanded)
    }
}

// 预览
struct HMICompoundTabView_Previews: PreviewProvider {
    static var previews: some View {
        HMICompoundTabView()
            .environmentObject(UserData())
            .environmentObject(DataManager())
    }
}
