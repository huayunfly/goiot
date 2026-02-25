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


struct DataInfo: Identifiable, Codable {
    let id: String
    let name: String
    let fValue: Double
    let intValue: Int32
    let byteValue: UInt8
    let boolValue: Bool
    let strValue: String
    let dtype: DataType
    let readWriteType: DataReadWriteType
    let result: Int32
    let timestamp: Double
    let ratio: Double
    let dataZone: DataZone
    let regiterAddress: Int32
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
    let group_name: String
    let table: [String: [String]]
    let total: Int
}

struct ResultModelBody: Decodable {
    let data: DataModelBody
}

struct ApiResponse: Decodable {
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
    @Published var items: [String: DataInfo] = [:]
    
    init() {
        let now = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd HH:mm:ss"
        
        for i in 1...20 {
            let item = DataInfo(
                id: String(i),
                name: "数据 \(i)",
                fValue: Double.random(in: 0.1...100.0),
                intValue: 0,
                byteValue: 0,
                boolValue: false,
                strValue: "",
                dtype: .DF,
                readWriteType: .readOnly,
                result: 0,
                timestamp: now.addingTimeInterval(-Double(i) * 86400).timeIntervalSince1970,
                ratio: 1.0,
                dataZone: .OUTPUT_REGISTER,
                regiterAddress: 0
            )
            items[String(i)] = item
        }
        Task {
            await loadJSONConfig(fromFile: "drivers")
        }
    }
    
    func loadJSONConfig(fromFile named: String) async
    {
        guard let config: DriverConfig = await JSONLoader.shared.loadData(fromFile: named) else {
            return
        }
        
        //let name = config.name
        
        for driver in config.drivers {
            let driverID = driver.id
            let driverName = driver.name
            for node in driver.nodes {
                let nodeID = node.address
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
                    items[dataID] = DataInfo (
                        id: dataID,
                        name: dataItem.name,
                        fValue: 0.0,
                        intValue: 0,
                        byteValue: 0,
                        boolValue: false,
                        strValue: "",
                        dtype: dataType,
                        readWriteType: readWriteType,
                        result: -1,
                        timestamp: Date().timeIntervalSince1970,
                        ratio: dataRatio,
                        dataZone: dataZone,
                        regiterAddress: registerAddress
                    )
                }
            }
        }
    }
}
