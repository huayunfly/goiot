import Foundation
import CoreData
import os.log

private let dataManagerLogger = Logger(subsystem: Bundle.main.bundleIdentifier ?? "com.goiot", category: "DataManager")

class DataManager: ObservableObject {
    var dataGroupIndexMap: [String: [String: [String: Int]]] = [:]
    @Published var dataArray: [DataInfo] = []
    
    private var timer: Timer?
    let persistenceController = PersistenceController.shared
    
    
    // 设置历史存储窗口（单位：小时），默认 3 小时
    var storageWindowHours: Int = 3 {
        didSet {
            // 一旦调整时间窗口，立即触发一次清理检查
            purgeOldData(hours: storageWindowHours)
        }
    }
    
    init() {
        // Appends a demo group
        DemoDataInfoGroup()
        
        Task {
            await loadJSONConfig(fromFile: "drivers")
        }
    }
    
    /// MARK: - 模拟接收数据并存储 (调用此函数更新你的业务)
    func handleDataUpdate(_ newInfo: DataInfo) {
        // 1. 更新内存中的实时数据 (原有逻辑)
        dataArray.append(newInfo)
        if dataArray.count > 10 { dataArray.removeFirst() } // 假设仅保留最近10条用于实时显示
        
        // 2. 保存到 Core Data
        saveToHistory(dataValue: [(dataId: "mfc.1.pv", value: 9)], group: "goiot")
        
        // 3. 检查并删除过期数据
        purgeOldData(hours: storageWindowHours)
    }
    
    // MARK: - 存储逻辑
    private func saveToHistory(dataValue: [(dataId: String, value: Double)] , group: String) {
        let backgroundContext = persistenceController.container.newBackgroundContext()
        
        for (dataId, value) in dataValue {
            let record = DataRecord(context: backgroundContext)
            record.id = dataId
            record.group = group
            record.value = value
            record.timestamp = Date()
        }
        do {
            try backgroundContext.save()
        } catch {
            dataManagerLogger.error("❌Save error: \(error)")
        }
    }
    
    // MARK: - 数据清理 (滑动窗口逻辑)
    func purgeOldData(hours: Int) {
        let backgroundContext = persistenceController.container.newBackgroundContext()
        let cutoffDate = Date().addingTimeInterval(-Double(hours * 3600))
        
        backgroundContext.perform {
            do {
                let fetchRequest: NSFetchRequest<DataRecord> = DataRecord.fetchRequest()
                fetchRequest.entity = DataRecord.entity()
                fetchRequest.predicate = NSPredicate(format: "timestamp < %@", cutoffDate as CVarArg)
                
                let oldData = try backgroundContext.fetch(fetchRequest)
                for record in oldData {
                    backgroundContext.delete(record)
                }
                try backgroundContext.save()
            } catch {
                print("Purge error: \(error)")
            }
        }
    }
    
    // MARK: - 提供给视图层读取数据的方法
    // Gets the history data with the given time range ( past 3 hours etc. )
    func fetchHistoryFor(key: String, group: String, lastHours: Int = 3) async -> [(timestamp: Date, value: Double)] {
        let backgroundContext: NSManagedObjectContext = persistenceController.container.newBackgroundContext()
        let cutoffDate = Date().addingTimeInterval(-Double(lastHours * 3600))
        var resultData: [(timestamp: Date, value: Double)] = []
        
        await backgroundContext.perform {
            let fetchRequest: NSFetchRequest<DataRecord> = DataRecord.fetchRequest()
            fetchRequest.predicate = NSPredicate(format: "id == %@ AND group == %@ AND timestamp >= %@", key, group, cutoffDate as CVarArg)
            fetchRequest.sortDescriptors = [NSSortDescriptor(key: "timestamp", ascending: true)]
            
            do {
                let records = try fetchRequest.execute()
                resultData = records.map { (timestamp: $0.timestamp ?? cutoffDate, value: $0.value) }
                dataManagerLogger.debug("✅ fetched \(resultData.count) records")
            } catch {
                dataManagerLogger.error("❌ Core Data fetch failed: \(error.localizedDescription)")
            }
        }
        return resultData
        
        
        //        do {
        //            let resultData = try await backgroundContext.perform { () throws -> [(timestamp: Date, value: Double)] in
        //                let fetchRequest: NSFetchRequest<DataRecord> = DataRecord.fetchRequest()
        //                fetchRequest.predicate = NSPredicate(format: "id == %@ AND group == %@ AND timestamp >= %@", key, group, cutoffDate as CVarArg)
        //                fetchRequest.sortDescriptors = [NSSortDescriptor(key: "timestamp", ascending: true)]
        //
        //                let records = try fetchRequest.execute()
        //                return records.map { (timestamp: $0.timestamp ?? cutoffDate, value: $0.value) }
        //            }
        //            dataManagerLogger.debug("✅ fetched \(resultData.count) records")
        //            return resultData
        //        } catch {
        //            dataManagerLogger.error("❌ Core Data fetch failed: \(error.localizedDescription)")
        //            return []
        //        }
    }

    // Loads JSON file
    func loadJSONConfig(fromFile named: String) async
    {
        guard let config: DriverConfig = await JSONLoader.shared.loadData(fromFile: named) else {
            return
        }
        
        let groupName = config.name
        if dataGroupIndexMap[groupName] != nil {
            return
        }
        
        var newDataGroupIndex: [String: [String: Int]] = [:]
        var startIndex = dataArray.count
        
        for driver in config.drivers {
            let driverID = driver.id
            var driverGroupIndex: [String: Int] = [:]
            for node in driver.nodes {
                for dataItem in node.data {
                    guard dataItem.register.count > 5 && !dataItem.id.isEmpty else {
                        continue
                    }
                    let dataID = [driver.id, node.address, dataItem.id].joined(separator: ".")
                    let dataRatio = dataItem.ratio ?? 1.0
                    
                    // Parse data type
                    let readWriteType: DataReadWriteType
                    var start = dataItem.register.startIndex
                    var end = dataItem.register.index(start, offsetBy: 2)
                    var str = dataItem.register[start..<end]
                    
                    if str.contains("R") && str.contains("W") {
                        readWriteType = DataReadWriteType.readWrite
                        start = dataItem.register.index(start, offsetBy: 2)
                    }
                    else if str.contains("R") {
                        readWriteType = DataReadWriteType.readOnly
                        start = dataItem.register.index(start, offsetBy: 1)
                    }
                    else if str.contains("W") {
                        readWriteType = DataReadWriteType.writeOnly
                        start = dataItem.register.index(start, offsetBy: 1)
                    }
                    else {
                        continue
                    }
                    
                    end = dataItem.register.index(start, offsetBy: 1)
                    str = dataItem.register[start..<end]
                    guard let intZone = Int32(str) else {
                        continue
                    }
                    let dataZone: DataZone
                    switch (intZone) {
                    case 0:
                        dataZone = .OUTPUT_RELAY
                    case 1:
                        dataZone = .INPUT_RELAY
                    case 3:
                        dataZone = .INPUT_REGISTER
                    case 4:
                        dataZone = .OUTPUT_REGISTER
                    case 5:
                        dataZone = .PLC_DB
                    case 6:
                        dataZone = .DB
                    default:
                        assert(false, "Parse DataZone \(intZone) error.")
                        continue
                    }
                    
                    var dataType: DataType
                    if dataZone == .INPUT_REGISTER || dataZone == .OUTPUT_REGISTER ||
                        dataZone == .INPUT_REGISTER || dataZone == .OUTPUT_REGISTER ||
                        dataZone == .PLC_DB || dataZone == .DB {
                        start = dataItem.register.index(start, offsetBy: 1)
                        end = dataItem.register.index(start, offsetBy: 3)
                        str = dataItem.register[start..<end]
                        if str.contains("DF") {
                            dataType = DataType.DF
                            start = dataItem.register.index(start, offsetBy: 2)
                        }
                        else if str.contains("WUB") {
                            dataType = DataType.WUB
                            start = dataItem.register.index(start, offsetBy: 3)
                        }
                        else if str.contains("WB") {
                            dataType = DataType.WB
                            start = dataItem.register.index(start, offsetBy: 2)
                        }
                        else if str.contains("DUB") {
                            dataType = DataType.DUB
                            start = dataItem.register.index(start, offsetBy: 3)
                        }
                        else if str.contains("DB") {
                            dataType = DataType.DB
                            start = dataItem.register.index(start, offsetBy: 2)
                        }
                        else if str.contains("BT") {
                            dataType = DataType.BT
                            start = dataItem.register.index(start, offsetBy: 2)
                        }
                        else if str.contains("BB") {
                            dataType = DataType.BB
                            start = dataItem.register.index(start, offsetBy: 2)
                        }
                        else if str.contains("STR") {
                            dataType = DataType.STR
                            start = dataItem.register.index(start, offsetBy: 3)
                        }
                        else {
                            assert(false, "Parse DataType \(str) error.")
                            continue
                        }
                    }
                    else
                    {
                        dataType = DataType.DF
                        start = dataItem.register.index(start, offsetBy: 2)
                    }
                    guard let registerAddress = Int32(dataItem.register[start..<dataItem.register.endIndex]) else {
                        continue
                    }
                    driverGroupIndex[dataID] = startIndex
                    dataArray.append(DataInfo (
                        id: dataID,
                        name: dataItem.name,
                        displayName: dataItem.displayName ?? "NULL",
                        ratio: dataRatio,
                        dtype: dataType,
                        readWriteType: readWriteType,
                        dataZone: dataZone,
                        regiterAddress: registerAddress,
                        fValue: 0.0,
                        intValue: 0,
                        byteValue: 0,
                        boolValue: false,
                        strValue: "",
                        result: -1,
                        timestamp: Date().timeIntervalSince1970
                    ))
                    startIndex+=1
                }
            }
            newDataGroupIndex[driverID] = driverGroupIndex
        }
        dataGroupIndexMap[groupName] = newDataGroupIndex
    }
    
    // Starts a timer to refresh data with 5 seconds interval.
    func StartRefreshData(token tokenID: String, withGroupSubGroup grpSubGroups: [String], withTimeInterval timeInterval: Double) {
        timer = Timer.scheduledTimer(withTimeInterval: timeInterval, repeats: true) { _ in
            DispatchQueue.global().async {
                Task {
                    do {
                        try await self.RefreshData(token: tokenID, withGroupSubGroup: grpSubGroups)
                    } catch {
                        print("Error in RefreshData().")
                    }
                }
            }
        }
    }
    
    // Stops a timer.
    func StopRefreshData() {
        timer?.invalidate()
        timer = nil
    }
    
    // Appends a demo group
    func DemoDataInfoGroup() {
        let now = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd HH:mm:ss"
        dataGroupIndexMap["Test"] = [:]
        var uiDemoGroup: [String: Int] = [:]
        for i in 1...20 {
            let item = DataInfo(
                id: String(i),
                name: "数据 \(i)",
                displayName: "显示数据 \(i)",
                ratio: 1.0,
                dtype: .DF,
                readWriteType: .readOnly,
                dataZone: .OUTPUT_REGISTER,
                regiterAddress: 0,
                fValue: Double.random(in: 0.1...100.0),
                intValue: 0,
                byteValue: 0,
                boolValue: false,
                strValue: "",
                result: 0,
                timestamp: now.addingTimeInterval(-Double(i) * 86400).timeIntervalSince1970
            )
            uiDemoGroup[String(i)] = i - 1
            dataArray.append(item)
        }
        dataGroupIndexMap["Test"] = ["UIDemo": uiDemoGroup]
    }
    
    func RefreshData(token tokenID: String, withGroupSubGroup grpSubGroup: [String]) async throws {
        for components in grpSubGroup {
            let words = components.split(separator: ".", maxSplits: 2, omittingEmptySubsequences: true)
            if words.count < 2 { continue }
            let groupName = words[0]
            let subGroupName = words[1]
            
            guard let dataGroup: [String: [String: Int]] = dataGroupIndexMap[String(groupName)] else {
                return
            }
            guard let nodeGroup: [String: Int] = dataGroup[String(subGroupName)] else {
                return
            }
            let dataIDList = Array(nodeGroup.keys)
            let endTime = Date().timeIntervalSince1970
            let startTime = endTime - 15.0
            let getDataCondition = GetDataRequestCondition(groupName: String(groupName),
                                                           idList: dataIDList,
                                                           timeRange: [startTime, endTime],
                                                           properties: ["value", "result", "time"],
                                                           batchSize: 128, batchNum: 1)
            let getDataRequest = ApiGetDataRequest(name: "service_name",
                                                   operation: "GETDATAR",
                                                   token: tokenID,
                                                   condition: getDataCondition)
            
            let address: String = NetworkConfig.baseURL
            let response: ApiGetDataResponse = try await WebServiceCaller.PostJSON(to: address, with: getDataRequest, timeoutInterval: 5)
            let resultData = response.result
            
            guard let dataGroup = self.dataGroupIndexMap[resultData.groupName] else { return }
            guard let idList = resultData.table["id"] else { return }
            guard let valueList = resultData.table["value"] else { return }
            guard let resultList = resultData.table["result"] else { return }
            guard let timeList = resultData.table["time"] else { return }
            
            let counts = [idList.count, valueList.count, resultList.count, timeList.count]
            if (counts.min() != counts.max()) {
                return
            }
            
            //所有 UI 更新必须通过 DispatchQueue.main.async 回到主线程。
            DispatchQueue.main.async {
                for (index, dataID) in idList.enumerated() {
                    if let dotIndex = dataID.firstIndex(of: ".") {
                        let firstPart = String(dataID[..<dotIndex])
                        if let dataIndex = dataGroup[firstPart]?[dataID] {
                            let dataItem = self.dataArray[dataIndex]
                            switch (dataItem.dtype) {
                            case .DF:
                                dataItem.fValue = Double(valueList[index]) ?? 0.0
                            case .WB, .WUB, .DB, .DUB:
                                dataItem.intValue = Int64(valueList[index]) ?? 0
                            case .BB:
                                dataItem.byteValue = UInt8(valueList[index]) ?? 0
                            case .BT:
                                dataItem.boolValue = (Int32(valueList[index]) ?? 0) > 0 ? true : false
                            case .STR:
                                dataItem.strValue = valueList[index]
                            }
                            dataItem.result = Int32(resultList[index]) ?? -1
                            dataItem.timestamp = Double(timeList[index]) ?? Date().timeIntervalSince1970
                        }
                    }
                }
            }
            
            // Saves to history
            var dataValue: [(dataId: String, value: Double)] = []
            for (index, dataID) in idList.enumerated() {
                if let dotIndex = dataID.firstIndex(of: ".") {
                    let firstPart = String(dataID[..<dotIndex])
                    if let dataIndex = dataGroup[firstPart]?[dataID] {
                        // Checks the result.
                        let result = Int32(resultList[index]) ?? -1
                        if result == 0 {
                            let dataItem = self.dataArray[dataIndex]
                            switch (dataItem.dtype) {
                            case .DF:
                                dataValue.append((dataId: dataID, value: Double(valueList[index]) ?? 0.0))
                            case .WB, .WUB, .DB, .DUB:
                                dataValue.append((dataId: dataID, value: Double(valueList[index]) ?? 0.0))
                            case .BB:
                                continue
                            case .BT:
                                continue
                            case .STR:
                                continue
                            }
                        }
                    }
                }
            }
            if dataValue.count > 0 {
                saveToHistory(dataValue: dataValue, group: String(groupName))
            }
        }
    }
    
    // MARK: - 数据写入支持
    func writeDataItem(_ dataInfo: DataInfo, token: String, writeValue: String) async {
        let payloadValue: String
        switch dataInfo.dtype {
        case .DF:
            // Double 直接转 String，也可以使用 String(format: "%.0f", dataInfo.fValue) 去除不必要的零
            payloadValue = String(dataInfo.fValue)
        case .WUB, .WB, .DUB, .DB:
            payloadValue = String(dataInfo.intValue)
        case .BB:
            payloadValue = String(dataInfo.byteValue)
        case .BT:
            // 布尔值通常转换为 1 或 0
            payloadValue = String(dataInfo.boolValue ? 1 : 0)
        case .STR:
            payloadValue = dataInfo.strValue
        }
        
        let timestamp = Date().timeIntervalSince1970
        let setDataCondition = SetDataRequestCondition(groupName: "goiot",
                                                       table: ["id": [String(dataInfo.id)], "value": [writeValue], "result": [String(0)], "time": [String(timestamp)]])
        
        let body = ApiSetDataRequest(
            name: "service_name",
            operation: "SETDATAP", // 根据实际 API 文档确认操作码
            token: token,
            condition: setDataCondition
        )
        
        do {
            let address = NetworkConfig.baseURL
            let response: ApiSetDataResponse = try await WebServiceCaller.PostJSON(to: address, with: body, timeoutInterval: 5)
            
            DispatchQueue.main.async {
                dataInfo.result = 0 // 标记写操作成功
                dataInfo.timestamp = timestamp
            }
        } catch {
            print("⚠️ 写入数据失败 \(dataInfo.id): \(error)")
            DispatchQueue.main.async {
                dataInfo.result = -1
            }
        }
    }
}
