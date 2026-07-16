//
//  ApiHelper.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/22.
//

import Foundation
import CoreData

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
    let displayName: String
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
    
    init(id: String, name: String, displayName: String, ratio: Double, dtype: DataType, readWriteType: DataReadWriteType, dataZone: DataZone, regiterAddress: Int32, fValue: Double, intValue: Int64, byteValue: UInt8, boolValue: Bool, strValue: String, result: Int32, timestamp: Double) {
        self.id = id
        self.name = name
        self.displayName = displayName
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
    let displayName: String?
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

// ApiGeneralRequest
struct ApiGeneralRequest: Encodable {
    let name: String
    let operation: String
    let token: String?
    let condition: [String: String]
}

struct ApiGeneralResponse: Decodable {
    let message: String
    let result: [String: String]?
}

// ApiGetDataRequest
struct ApiGetDataRequest: Encodable {
    let name: String
    let operation: String
    let token: String
    let condition: GetDataRequestCondition
}

struct GetDataRequestCondition: Codable {
    let groupName: String
    let idList: [String]
    let timeRange: [Double]
    let properties: [String]
    let batchSize: Int32
    let batchNum: Int32
    
    enum CodingKeys: String, CodingKey {
        case groupName = "groupName"
        case idList = "idList"
        case timeRange = "timeRange"
        case properties
        case batchSize = "batchSize"
        case batchNum = "batchNum"
    }
}

// ApiGetDataResponse
struct ApiGetDataResponse: Decodable {
    let message: String
    let result: GetDataResult
    
    enum CodingKeys: String, CodingKey {
        case message
        case result
    }
}

struct GetDataResult: Decodable {
    let groupName: String
    let table: [String: [String]]
    
    enum CodingKeys: String, CodingKey {
        case groupName = "groupName"
        case table
    }
}

// ApiSetDataRequest
struct ApiSetDataRequest: Encodable {
    let name: String
    let operation: String
    let token: String
    let condition: SetDataRequestCondition
}

struct SetDataRequestCondition: Codable {
    let groupName: String
    let table: [String: [String]]
    
    enum CodingKeys: String, CodingKey {
        case groupName = "groupName"
        case table = "table"
    }
}

// ApiSetDataResponse
struct ApiSetDataResponse: Decodable {
    let message: String
    let result: [String: String]
    
    enum CodingKeys: String, CodingKey {
        case message
        case result
    }
}

struct SetDataResult: Decodable {
    let updated: Int32
    
    enum CodingKeys: String, CodingKey {
        case updated
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
