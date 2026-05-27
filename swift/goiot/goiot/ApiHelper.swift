//
//  ApiHelper.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/22.
//

import Foundation

struct ApiResultBody: Decodable {
    let message: String
    let result: [String: String]?
    let statusCode: String
}

struct ApiRequestBody: Encodable {
    let name: String
    let operation: String
    let token: String?
    let condition: [String: String]
}

struct ApiGetDataBody {
    let name: String
    let operation: String
    let token: String
    let condition : [String: Any]
}

struct ApiSetDataBody {
    let name: String
    let operation: String
    let token: String
    let data: [String: Any]
}

// register: driver-specified, DF (float), WUB (16bits unsigned byte), WB (16bits signed byte), DUB (32bits unsigned byte), DB (32bits signed byte)
// BB(byte), BT (bit), STR(string)
enum DataType: Codable {
    case DF
    case WUB
    case WB
    case DUB
    case DB
    case BB
    case BT
    case STR
}

enum DataReadWriteType: Codable {
    case readOnly
    case writeOnly
    case readWrite
}

enum DataZone: Codable
{
    case OUTPUT_RELAY
    case INPUT_RELAY
    case INPUT_REGISTER
    case OUTPUT_REGISTER
    case PLC_DB
    case DB
}

class DataInfo: Identifiable, ObservableObject {
    let id: String
    let name: String
    let ratio: Double
    let dtype: DataType
    let readWriteType: DataReadWriteType
    let dataZone: DataZone
    let regiterAddress: Int32
    @Published var fValue: Double
    @Published var intValue: Int64
    @Published var byteValue: UInt8
    @Published var boolValue: Bool
    @Published var strValue: String
    @Published var result: Int32
    @Published var timestamp: Double
    
    init(id: String, name: String, ratio: Double, dtype: DataType, readWriteType: DataReadWriteType, dataZone: DataZone, regiterAddress: Int32, fValue: Double, intValue: Int64, byteValue: UInt8, boolValue: Bool, strValue: String, result: Int32, timestamp: Double) {
        self.id = id
        self.name = name
        self.ratio = ratio
        self.dtype = dtype
        self.readWriteType = readWriteType
        self.dataZone = dataZone
        self.regiterAddress = regiterAddress
        self.fValue = fValue
        self.intValue = intValue
        self.byteValue = byteValue
        self.boolValue = boolValue
        self.strValue = strValue
        self.result = result
        self.timestamp = timestamp
    }
}

// Driver data configuration structure
struct DriverDataItem: Decodable {
    let id: String
    let name: String
    let register: String
    let ratio: Double?
}

struct DriverNode: Decodable {
    let address: String
    let data: [DriverDataItem]
}

struct DriverBody: Decodable {
    let id: String
    let name: String
    let type: String
    let port: String
    let response_timeout_msec: Int32
    let refresh_interval_msec: Int32
    let nodes: [DriverNode]
}

struct DriverConfig: Decodable {
    let name: String
    let webapi: String
    let redis: String
    let db: String
    let tenant: String?
    let drivers: [DriverBody]
}

// Json request structure
struct ApiGeneralRequest: Encodable {
    let name: String
    let operation: String
    let token: String
    let condition: RequestConditionBody
}


struct RequestConditionBody: Codable {
    let groupName: String
    let idList: [String]
    let timeRange: [Double]
    let properties: [String]
    let batchSize: Int32
    let batchNum: Int32
    
    enum CodingKeys: String, CodingKey {
        case groupName = "group_name"
        case idList = "id_list"
        case timeRange = "time_range"
        case properties
        case batchSize = "batch_size"
        case batchNum = "batch_num"
    }
}

// JSON response structure
struct DataModelBody: Decodable {
    let groupName: String
    let table: [String: [String]]
    let total: Int32
    
    enum CodingKeys: String, CodingKey {
        case groupName = "group_name"
        case table
        case total
    }
}

struct ResultModelBody: Decodable {
    let data: DataModelBody?
    let error: String?
}

struct ApiGetDataResponse: Decodable {
    let message: String
    let result: ResultModelBody
    let statusCode: String
    
    enum CodingKeys: String, CodingKey {
        case message
        case result
        case statusCode = "statusCode"
    }
}

// JSON loader factory
class JSONLoader {
    static let shared = JSONLoader()
    
    private init() {}
    
    func loadData<T: Decodable>(fromFile named: String) async -> T? {
        //let url = URL(fileURLWithPath: named)
        guard let url = Bundle.main.url(forResource: named, withExtension: "json") else {
            print("File not found: \(named)")
            return nil
        }
        
        do {
            let data = try await loadFile(at: url)
            let decoder = JSONDecoder()
            return try decoder.decode(T.self, from: data)
        } catch {
            print("Error loading JSON: \(error)")
            return nil
        }
    }
    
    private func loadFile(at url: URL) async throws -> Data {
        if #available(iOS 14.0, *) {
            let (data, _) = try await URLSession.shared.data(from: url)
            return data
        } else {
            return try Data(contentsOf: url)
        }
    }
}


class DataManager: ObservableObject {
    //@Published var dataItems: [String: DataInfo] = [:]
    //@Published var dataGroups: [String: [String: DataInfo]] = [:]
    @Published var dataGroupIndexMap: [String: [String: [String: Int]]] = [:]
    @Published var dataArray: [DataInfo] = []
    
    private var timer: Timer?
    @Published var count = 0
    
    init() {
        let now = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd HH:mm:ss"
        dataGroupIndexMap["Test"] = [:]
        var uiDemoGroup: [String: Int] = [:]
        for i in 1...20 {
            let item = DataInfo(
                id: String(i),
                name: "数据 \(i)",
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
        Task {
            await loadJSONConfig(fromFile: "drivers")
        }
        print("init datamanager()")
    }

    // Load JSON file
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
        //var items: [String: DataInfo] = [:]
        

        print("driver count: \(config.drivers.count)")
        
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
    
    // Start a timer to refresh data with 5 seconds interval.
    @MainActor
    func StartRefreshData(token tokenID: String, withTimeInterval timeInterval: Double) {
        timer = Timer.scheduledTimer(withTimeInterval: timeInterval, repeats: true) { _ in
            DispatchQueue.global().async {
                Task {
                    do {
                        try await self.RefreshData(token: tokenID)
                        DispatchQueue.main.async { //所有 UI 更新必须通过 DispatchQueue.main.async 回到主线程。
                            self.count += 1
                        }
                    } catch {
                        print("Error in RefreshData().")
                    }
                }
            }
        }
    }

    // Stop a timer.
    func StopRefreshData() {
        timer?.invalidate()
        timer = nil
    }
    
    func RefreshData(token tokenID: String) async throws {
        guard let dataGroup: [String: [String: Int]] = dataGroupIndexMap["goiot"] else {
            return
        }
        guard let nodeGroup: [String: Int] = dataGroup["fp2"] else {
            return
        }
        let dataIDList = Array(nodeGroup.keys)
        //let dataIDList = ["mfcpfc.1.pv", "mfcpfc.2.pv","fp2.1.tc801_pv"]
        let requestCondition = RequestConditionBody(groupName: "goiot",
                                                    idList: dataIDList,
                                                    timeRange: [1677154221, 2677154223],
                                                    properties: ["value", "result", "time"],
                                                    batchSize: 128, batchNum: 1)
        let getDataRequest = ApiGeneralRequest(name: "service_name",
                                               operation: "GetDataR",
                                               token: tokenID,
                                               condition: requestCondition)
        
        let address: String = "http://192.168.2.177:6300/message"
        let response: ApiGetDataResponse = try await WebServiceCaller.PostJSON(to: address, with: getDataRequest, timeoutInterval: 5)
        guard response.statusCode == "200" else {
            print("GetData error: \(response.result.error ?? "unknown error")")
            throw WebServiceError.GetDataError
        }
        guard let resultData = response.result.data else {
            print("GetData error: resultData nil")
            throw WebServiceError.GetDataError
        }
        if resultData.total < 1 { return }
        guard let dataGroup = dataGroupIndexMap[resultData.groupName] else { return }
        guard let idList = resultData.table["id"] else { return }
        guard let valueList = resultData.table["value"] else { return }
        guard let resultList = resultData.table["result"] else { return }
        guard let timeList = resultData.table["time"] else { return }
        
        let counts = [idList.count, valueList.count, resultList.count, timeList.count]
        if (counts.min() != counts.max()) {
            return
        }
        var num = 0
        for dataID in idList {
            if let dotIndex = dataID.firstIndex(of: ".") {
                let firstPart = String(dataID[..<dotIndex])
                if let dataIndex = dataGroup[firstPart]?[dataID] {
                    var dataItem = dataArray[dataIndex]
                    switch (dataItem.dtype) {
                    case .DF:
                        dataItem.fValue = Double(valueList[num]) ?? 0.0
                    case .WB, .WUB, .DB, .DUB:
                        dataItem.intValue = Int64(valueList[num]) ?? 0
                    case .BB:
                        dataItem.byteValue = UInt8(valueList[num]) ?? 0
                    case .BT:
                        dataItem.boolValue = (Int32(valueList[num]) ?? 0) > 0 ? true : false
                    default:
                        assert(false, "Match DataType \(dataItem.dtype) error.")
                        continue
                    }
                    dataItem.result = Int32(resultList[num]) ?? -1
                    dataItem.timestamp = Double(timeList[num]) ?? Date().timeIntervalSince1970
                }
            }
            num+=1
        }
    }
}
