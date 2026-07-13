import SwiftUI

// 图形化节点模型
struct HMIDeviceNode: Identifiable, Hashable {
    let id: UUID
    let name: String
    let title: String
    let canvasPosition: CGPoint
    let pvInfoId: String
    let svInfoId: String?
    let iconType: HMIIconModule
    let operationUIType: HMIOperationUIType
    let measurementUnit: HMIMeasurementUnit
    let textFomat: String
    let highLimit: Double?
    let lowLimit: Double?
}

enum HMIIconModule: Hashable {
    case symbol(String)
    case image(String)
}

enum HMIOperationUIType: Hashable {
    case text
    case state(UInt)
    case processValue(UInt)
    case onOff(UInt)
}

enum HMIMeasurementUnit: Hashable {
    case none
    case L
    case mL
    case barA
    case barG
    case sccm
    case C
    case mm
    case mLm
    case Lm
    case MPa
    case rpm
    case lel
    case ppm
}

// Global image cache
internal struct HMIImageCache {
    static var imageCache: [String: UIImage] = [:]
    
    private init() {}
}

// 主视图
struct HMIControlTabView: View {
    @EnvironmentObject var userData: UserData
    @EnvironmentObject var dataManager: DataManager
    
    @GestureState private var magnification: CGFloat = 1.0
    @State private var zoom: CGFloat = 1.0
    
    @State private var scale: CGFloat = 1.0
    @State private var selectedNode: HMIDeviceNode?
    @State private var livePV: [String: String] = [:]
    @State private var livePVStatus: [String: Int32] = [:]
    
    // Canvas
    @State private var canvasSize: CGSize = .zero
    
    // Zoom offset
    @State private var offset: CGSize = .zero
    
    // Canvas
    let canvasName: String
    
    // Device nodes
    let hmiNodes: [HMIDeviceNode]
    
    var body: some View {
        NavigationStack {
            ScrollView([.horizontal, .vertical]) {
                if #available(iOS 17.0, *) {
                    ZStack {
                        // 1. PID 背景图
                        Rectangle()
                            .fill(Color(.systemGray6))
                            .frame(width: canvasSize.width, height: canvasSize.height)
                            .overlay {
                                if let uiImage = UIImage.getImageCached(named: canvasName) {
                                    Image(uiImage: uiImage)
                                        .resizable()
                                        .scaledToFit()
                                        .frame(width: canvasSize.width, height: canvasSize.height)
                                        .cornerRadius(12)
                                }
                                else {
                                    Image(systemName: "table")
                                }
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
                            HMIDeviceNodeView(node: node, pvValue: livePV[node.pvInfoId] ?? "0.x", pvStatus: livePVStatus[node.pvInfoId] ?? -1, isEditing: selectedNode?.id == node.id)
                                // A view that fixes the center of this view at `position`.
                                .position(node.canvasPosition)
                                .onTapGesture {
                                    withAnimation {
                                        selectedNode = node
                                    }
                                }
                        }
                    }
                    // 设定 ZStack 的物理尺寸
                    .frame(width: canvasSize.width, height: canvasSize.height)
                    
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
        .onAppear {
            canvasSize = UIImage.getOriginalImageSize(named: canvasName, fallback: CGSize(width: 1200, height: 900))
            startPVListener()
        }
    }
    
    private func startPVListener() {
        Task {
            // 确保UI总是在主线程上更新
            let actor = await MainActor.run { self }
            while !Task.isCancelled {
                for node in actor.hmiNodes {
                    guard let dataInfo = getDataInfo(byId: node.pvInfoId, from: actor.dataManager) else {
                        actor.livePVStatus[node.pvInfoId] = -1 // Error
                        continue
                    }
                    actor.livePV[node.pvInfoId] = dataInfoValueToString(from: dataInfo, format: node.textFomat, unit: node.measurementUnit)
                    actor.livePVStatus[node.pvInfoId] = dataInfo.result
                }
                try? await Task.sleep(for: .seconds(1.0))
            }
        }
    }
    
    // Converts dataInfo value to String by the specified format and measurement unit.
    private func dataInfoValueToString(from dataInfo: DataInfo, format fmt: String?, unit dataUnit: HMIMeasurementUnit) -> String {
        let strValue: String
        switch dataInfo.dtype {
        case .DF:
            strValue = String(format: fmt ?? "%.2f", dataInfo.fValue * dataInfo.ratio)
        case .WB, .WUB, .DB, .DUB:
            if abs(dataInfo.ratio - 1.0) < 1e-6 {
                strValue = String(format: fmt ?? "%d", dataInfo.intValue)
            }
            // Rule: Integer value to float value conversion on some devices (MFC etc.)
            else {
                strValue = String(format: fmt ?? "%.2f", Double(dataInfo.intValue) * dataInfo.ratio)
            }
        case .BB:
            strValue = String(format: fmt ?? "%d", dataInfo.byteValue)
        case .BT:
            strValue = dataInfo.boolValue ? "1" : "0"
        default:
            assert(false, "Match DataType \(dataInfo.dtype) error.")
            return "0.x"
        }
        return "\(strValue) \(dataUnit)"
    }
    
    // Gets dataInfo from dataMagager by dataId.
    private func getDataInfo(byId dataId: String, from dataManager: DataManager) -> DataInfo? {
        guard !dataId.isEmpty else { return nil }
        
        // "goiot.mfc.1.pv".split() -> "goiot" "mfc.1.pv"
        let zoneNames = dataId.split(separator: ".", maxSplits: 1, omittingEmptySubsequences: true)
        if zoneNames.count < 2 { return nil }
        let zone = String(zoneNames[0])
        let dataID = String(zoneNames[1])
        // "mfc.1.pv".split() -> "mfc" "mfc.1.pv"
        let groupNames = dataID.split(separator: ".", maxSplits: 1, omittingEmptySubsequences: true)
        if groupNames.count < 2 { return nil }
        let group = String(groupNames[0])
        
        guard let dataIndex = dataManager.dataGroupIndexMap[zone]?[group]?[dataID] else { return nil }
        if dataIndex >= dataManager.dataArray.count { return nil }
        return dataManager.dataArray[dataIndex]
    }
    
    // Limits the zooming image size.
    private func clampZoom(_ value: CGFloat) -> CGFloat {
        return min(max(value, 0.4), 2.5)
    }
}

// UIImage methods to do with the cached images.
extension UIImage {
    // Get UIImage size.
    static func getOriginalImageSize(named imageName: String, fallback: CGSize) -> CGSize {
        guard let image = getImageCached(named: imageName) else {
            return fallback
        }
        
        // Points * Scale = Pixels
        let width = image.size.width * image.scale
        let height = image.size.height * image.scale
        return CGSize(width: width, height: height)
    }
    
    // Get UIImage in the cache or load an image from Assets.
    static func getImageCached(named imageName: String) -> UIImage? {
        if let foundImage =  HMIImageCache.imageCache[imageName] {
            return foundImage
        } else {
            if let uiImage = UIImage(named: imageName) {
                HMIImageCache.imageCache[imageName] = uiImage
                return uiImage
            }
            else {
                return nil
            }
        }
    }
    
    // Crop an image
    static func getCroppedImage(from uiImage: UIImage, cropArea cropRect: CGRect) -> UIImage? {
        if let croppedImage = uiImage.cgImage?.cropping(to: cropRect) {
            return UIImage(cgImage: croppedImage)
        } else {
            return nil
        }
    }
    
    // Divide an image into multiple equal pieces by width and get one.
    static func getSplitImageByWidth(from uiImage: UIImage, number numSplit: UInt, index idxSplit: UInt) -> UIImage? {
        let width = uiImage.size.width * uiImage.scale
        let height = uiImage.size.height * uiImage.scale
        if width > 0.0 && height > 0.0 && numSplit > 0 {
            let splitWidth = width / Double(numSplit)
            let rect = CGRect(x: Double(idxSplit) * splitWidth, y: 0, width: splitWidth, height: height)
            return getCroppedImage(from: uiImage, cropArea: rect)
        }
        else {
            return nil
        }
    }
}

// 设备节点视图
struct HMIDeviceNodeView: View {
    @State private var canvasSize: CGSize = .zero
    @State private var uiImage: UIImage = UIImage()
    
    @State var isNormal: Bool = false
    
    let node: HMIDeviceNode
    let pvValue: String
    let pvStatus: Int32
    let isEditing: Bool
    
    // Dynamically displaying image field
    private var imageIcon: some View {
        Group {
            switch node.iconType {
            case .symbol(let name):
                Image(systemName: name)
                    .font(.system(size: 28))
                    .foregroundColor(isEditing ? .yellow : .primary)
            case .image(let name):
                Image(uiImage: uiImage)
                    .frame(width: canvasSize.width, height: canvasSize.height)
                    .onAppear {
                        updateNodeImage(name, pvStatus)
                    }
                    // Listens pvStatus and switches the node image state
                    .onChange(of: pvStatus) { _, newState in
                        updateNodeImage(name, newState)
                    }
            }
        }
    }

    var body: some View {
        VStack(spacing: 0) {
            Text(pvValue)
                .font(.system(.caption, design: .rounded).bold())
                .foregroundColor(.primary)
                .padding(8)
                // Retain half transparent zone for PV
                .background(Color(.systemBackground).opacity(0.85))
                .cornerRadius(4)
                .shadow(radius: 2)
   
            imageIcon
                .padding(6) // Retain a few zone for the gesture click.
                .background(Color.clear)
                // 增加选中时的黄色虚线边框/光晕作为交互反馈
                .overlay(RoundedRectangle(cornerRadius: 12).stroke(isEditing ? Color.yellow : Color.clear, lineWidth: 2))
                .shadow(color: isEditing ? .orange.opacity(0.6) : .black.opacity(0.1), radius: isEditing ? 10 : 2, y: 2)
                .scaleEffect(isEditing ? 1.1 : 1.0)
                .animation(.spring(response: 0.25, dampingFraction: 0.8), value: isEditing)
            
            Label(node.name, systemImage: "table")
                .labelStyle(.titleOnly).padding(2).font(.system(.caption, design: .rounded).bold())
        }
        // 节点整体外层透明化
        .background(Color.clear)
    }
    
    // Helper function: update the dynamic image according the PV status.
    private func updateNodeImage(_ name: String, _ status: Int32) {
        if let image = UIImage.getImageCached(named: name) {
            var splitNum: UInt = 1
            
            // Get the splited part according the operation type.
            switch node.operationUIType {
            case .text:
                splitNum = 1
            case .state(let statusNum):
                splitNum = statusNum
            case .processValue(let statusNum):
                splitNum = statusNum
            case .onOff(let statusNum):
                splitNum = statusNum
            }
                                    
            // Logic:
            // IF pvStatus == 0 (device Normal state), index = 1 (Split sheet 2nd column)
            // Otherwise index = 0 (Split sheet 1st column, default Abnormal/Alarming state)
            let targetIndex: UInt = (status == 0) ? 1 : 0
            
            // Preventing array index out of bounds
            let safeIndex = min(targetIndex, splitNum - 1)
            
            // Calculate the cropped image size and update UI
            let imgSize = UIImage.getOriginalImageSize(named: name, fallback: CGSize(width: 30, height: 30))
            canvasSize = CGSize(width: imgSize.width / Double(splitNum), height: imgSize.height)
            uiImage = UIImage.getSplitImageByWidth(from: image, number: splitNum, index: safeIndex) ?? image
        } else {
            canvasSize = CGSize(width: 30, height: 30)
            uiImage = UIImage()
        }
    }
}

// SV 编辑面板
struct SVEditorSheet: View {
    let node: HMIDeviceNode
    let token: String
    @EnvironmentObject var dataManager: DataManager
    @State private var svInput: String = "0.0"
    @Environment(\.dismiss) var dismiss
    
    // 临时编辑状态，避免未确认时直接修改绑定值
    @State private var tempFloat: Double = 0
    @State private var tempInt: Int = 0
    @State private var tempBool: Bool = false
    @State private var errorMessage: String = ""
    
    private var targetInfo: DataInfo? {
        guard let svId = node.svInfoId else { return nil }
        return getDataInfo(byId: svId)
    }
    
    var body: some View {
        NavigationStack {
            Form {
                Section(header: Text("当前设定值 (SV) \(String(describing: node.measurementUnit))")) {
                    HStack {
                        Text("目标值:")
                        Spacer()
                        Text(svInput)
                            .font(.title2.monospaced())
                            .foregroundColor(.blue)
                    }
                }
                
                switch node.operationUIType {
                case .processValue: onOffEditor
                case .state: stateEditor
                case .onOff: onOffEditor
                case .text: onOffEditor
                }
                
                if !errorMessage.isEmpty {
                    Text(errorMessage)
                        .foregroundColor(.red)
                        .font(.caption)
                        .multilineTextAlignment(.center)
                }

                Section {
                    Button("写入设备 / 应用设定") {
                        if applyChanges() {
                            submitSV()
                        }
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
                    Button("返回") { dismiss() }
                }
            }
            .onAppear {
                if let dataInfo = targetInfo {
                    svInput = dataInfoValueToString(from: dataInfo, format: node.textFomat)
                }
                syncInitialValues()
            }
        }
    }
    
    @ViewBuilder
    private var processValueEditor: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Text("下限: \(formatLimit(node.lowLimit))")
                Spacer()
                Text("上限: \(formatLimit(node.highLimit))")
            }
            .font(.caption)
            .foregroundColor(.secondary)
            
            TextField("输入设定值", value: $tempFloat, format: .number)
                .textFieldStyle(.roundedBorder)
                .keyboardType(.decimalPad)
                .disableAutocorrection(true)
        }
    }
    
    @ViewBuilder
    private var stateEditor: some View {
        Picker("选择状态", selection: $tempInt) {
            let upperBound = max(0, min(Int.max, Int(node.highLimit ?? 0)))
            ForEach(0...upperBound, id: \.self) { index in
                Text("状态 \(index)")
                    .tag(index)
            }
        }
        .pickerStyle(.menu)
    }
    
    @ViewBuilder
    private var onOffEditor: some View {
        Toggle(isOn: $tempBool) {
            Text("运行状态 (ON / OFF)")
        }
        .toggleStyle(CustomOnOffToggleStyle())
    }
    
    // Gets dataInfo from dataMagager.
    private func getDataInfo(byId dataId: String) -> DataInfo? {
        guard !dataId.isEmpty else { return nil }
        
        // "goiot.mfc.1.sv".split() -> "goiot" "mfc.1.sv"
        let zoneNames = dataId.split(separator: ".", maxSplits: 1, omittingEmptySubsequences: true)
        if zoneNames.count < 2 { return nil }
        let zone = String(zoneNames[0])
        let dataID = String(zoneNames[1])
        // "mfc.1.sv".split() -> "mfc" "1.sv"
        let groupNames = dataID.split(separator: ".", maxSplits: 1, omittingEmptySubsequences: true)
        if groupNames.count < 2 { return nil }
        let group = String(groupNames[0])
        
        guard let dataIndex = dataManager.dataGroupIndexMap[zone]?[group]?[dataID] else { return nil }
        if dataIndex >= dataManager.dataArray.count { return nil }
        return dataManager.dataArray[dataIndex]
    }
    
    private func syncInitialValues() {
        tempFloat = Double(svInput) ?? 0.0
        tempInt = Int(svInput) ?? 0
        tempBool = Bool(svInput) ?? false
        errorMessage = ""
    }
    
    private func applyChanges() -> Bool {
        switch node.operationUIType {
        case .text:
            svInput = ""
        case .state:
            svInput = String(tempInt)
        case .processValue:
            guard validateRange(value: tempFloat) else { return false }
            svInput = String(tempFloat)
        case .onOff:
            svInput = tempBool ? "1" : "0"
        }
        return true
    }
    
    private func validateRange(value: Double) -> Bool {
        
        let low: Double = node.lowLimit ?? -Double.greatestFiniteMagnitude
        let high = node.highLimit ?? Double.greatestFiniteMagnitude
        
        if value < low || value > high {
            errorMessage = "⚠️ 值超出允许范围：\(low)...\(high)"
            return false
        }
        return true
    }
    
    private func formatLimit(_ limit: Double?) -> String {
        guard let limit = limit else { return "无限制" }
        return String(format: "%.2f", limit)
    }
    
    // Converts dataInfo value to String by the specified format
    private func dataInfoValueToString(from dataInfo: DataInfo, format fmt: String?) -> String {
        let strValue: String
        switch dataInfo.dtype {
        case .DF:
            strValue = String(format: fmt ?? "%.2f", dataInfo.fValue * dataInfo.ratio)
        case .WB, .WUB, .DB, .DUB:
            if abs(dataInfo.ratio - 1.0) < 1e-6 {
                strValue = String(format: fmt ?? "%d", dataInfo.intValue)
            }
            // Rule: Integer value to float value conversion on some devices (MFC etc.)
            else {
                strValue = String(format: fmt ?? "%.2f", Double(dataInfo.intValue) * dataInfo.ratio)
            }
        case .BB:
            strValue = String(format: fmt ?? "%d", dataInfo.byteValue)
        case .BT:
            strValue = dataInfo.boolValue ? "1" : "0"
        case .STR:
            strValue = dataInfo.strValue
        }
        return strValue
    }
    
    private func submitSV() {
        guard let dataInfo = targetInfo else { return }
        Task {
            do {
                //
                var parsedValue: String? = nil
                switch dataInfo.dtype {
                case .DF:
                    parsedValue = String((Double(svInput) ?? dataInfo.fValue) / dataInfo.ratio)
                case .WUB, .WB:
                    if (abs(dataInfo.ratio - 1.0) < 1e-6) {
                        parsedValue = String(Int16(svInput).flatMap(Int64.init) ?? dataInfo.intValue)
                    } else {
                        let convertValue: Double = (Double(svInput) ?? 0.0) / dataInfo.ratio
                        parsedValue = String(format: "%.0f", ceil(convertValue))
                    }
                case .DUB, .DB:
                    if (abs(dataInfo.ratio - 1.0) < 1e-6) {
                        parsedValue = String(Int32(svInput).flatMap(Int64.init) ?? dataInfo.intValue)
                    }
                    else {
                        let convertValue: Double = (Double(svInput) ?? 0.0) / dataInfo.ratio
                        parsedValue = String(format: "%.0f", ceil(convertValue))
                    }
                case .BB:
                    parsedValue = String(UInt8(svInput).flatMap(UInt8.init) ?? dataInfo.byteValue)
                case .BT:
                    parsedValue = String((Int(svInput) ?? 0) > 0 ? 1 : 0)
                case .STR:
                    parsedValue = svInput.isEmpty ? dataInfo.strValue : svInput
                }
                
                if let val = parsedValue {
                    try await dataManager.writeDataItem(dataInfo, token: token, writeValue: val)
                    print("✅ SV 写入成功: \(dataInfo.id) -> \(svInput)")
                } else {
                    // 格式错误时保持编辑状态，方便用户修正
                    print("格式错误")
                }
                dismiss()
            } catch {
                print("❌ SV 写入失败: \(error)")
            }
        }
    }
}

// MARK: - 自定义 ON/OFF 样式
extension SVEditorSheet {
    private struct CustomOnOffToggleStyle: ToggleStyle {
        func makeBody(configuration: Configuration) -> some View {
            Button {
                configuration.isOn.toggle()
            } label: {
                HStack {
                    Text(configuration.isOn ? "ON" : "OFF")
                        .fontWeight(.bold)
                        .foregroundColor(.white)
                    Spacer()
                }
                .padding()
                .frame(maxWidth: .infinity)
                .background(configuration.isOn ? Color.green : Color.red.opacity(0.7))
                .cornerRadius(10)
                .contentShape(Rectangle())
            }
            .buttonStyle(.plain)
        }
    }
}

// 预览
struct HMIControlTabView_Previews: PreviewProvider {
    static let hmiNodesDemo: [HMIDeviceNode] = [
        HMIDeviceNode(id: UUID(), name: "FICA1111", title: "固定床H2质量流量控制器", canvasPosition: CGPoint(x: 302, y: 187), pvInfoId: "goiot.mfc.1.pv", svInfoId: "goiot.mfc.1.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1121", title: "固定床CO质量流量控制器", canvasPosition: CGPoint(x: 302, y: 288), pvInfoId: "goiot.mfc.2.pv", svInfoId: "goiot.mfc.2.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1131", title: "固定床N2质量流量控制器", canvasPosition: CGPoint(x: 302, y: 389), pvInfoId: "goiot.mfc.3.pv", svInfoId: "goiot.mfc.3.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1141", title: "固定床CO2质量流量控制器", canvasPosition: CGPoint(x: 302, y: 490), pvInfoId: "goiot.mfc.4.pv", svInfoId: "goiot.mfc.4.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1151", title: "固定床C2H4质量流量控制器", canvasPosition: CGPoint(x: 302, y: 591), pvInfoId: "goiot.mfc.5.pv", svInfoId: "goiot.mfc.5.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1511", title: "釜H2质量流量控制器", canvasPosition: CGPoint(x: 812, y: 232), pvInfoId: "goiot.mfc.6.pv", svInfoId: "goiot.mfc.6.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1521", title: "釜CO质量流量控制器", canvasPosition: CGPoint(x: 812, y: 333), pvInfoId: "goiot.mfc.7.pv", svInfoId: "goiot.mfc.7.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1531", title: "釜N2质量流量控制器", canvasPosition: CGPoint(x: 812, y: 434), pvInfoId: "goiot.mfc.8.pv", svInfoId: "goiot.mfc.8.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1541", title: "釜CO2质量流量控制器", canvasPosition: CGPoint(x: 812, y: 535), pvInfoId: "goiot.mfc.9.pv", svInfoId: "goiot.mfc.9.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "FICA1551", title: "釜C2H4质量流量控制器", canvasPosition: CGPoint(x: 812, y: 636), pvInfoId: "goiot.mfc.10.pv", svInfoId: "goiot.mfc.10.sv", iconType: .image("mfc"), operationUIType: .processValue(2), measurementUnit: .sccm, textFomat: "%0.1f", highLimit: 200, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "PIA1111", title: "H2气源压力", canvasPosition: CGPoint(x: 161, y: 184), pvInfoId: "goiot.s7.1.pg_1", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .barA, textFomat: "%0.1f", highLimit: 100, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "PIA1121", title: "CO气源压力", canvasPosition: CGPoint(x: 161, y: 285), pvInfoId: "goiot.s7.1.pg_2", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .barA, textFomat: "%0.1f", highLimit: 100, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "PIA1131", title: "N2气源压力", canvasPosition: CGPoint(x: 161, y: 386), pvInfoId: "goiot.s7.1.pg_3", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .barA, textFomat: "%0.1f", highLimit: 100, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "PIA1141", title: "CO2气源压力", canvasPosition: CGPoint(x: 161, y: 487), pvInfoId: "goiot.s7.1.pg_4", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .barA, textFomat: "%0.1f", highLimit: 100, lowLimit: 0),
        HMIDeviceNode(id: UUID(), name: "PIA1151", title: "C2H4气源压力", canvasPosition: CGPoint(x: 161, y: 588), pvInfoId: "goiot.s7.1.pg_5", svInfoId: nil, iconType: .image("pressure_measure"), operationUIType: .processValue(2), measurementUnit: .barA, textFomat: "%0.1f", highLimit: 100, lowLimit: 0),
    ]
    
    static var previews: some View {
        HMIControlTabView(canvasName: "837_layout1", hmiNodes: hmiNodesDemo)
            .environmentObject(UserData())
            .environmentObject(DataManager())
    }
}

