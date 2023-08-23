//
//  ApiHelper.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/22.
//

import Foundation

struct ApiResultBody: Decodable {
    let message: String
    let result: [String: String]
    let statusCode: String
    let error: String?
}

struct ApiRequestBody: Encodable {
    let name: String
    let token: String?
    let operation: String
    let condition: [String: String]
}

struct ApiGetDataBody {
    let name: String
    let token: String
    let operation: String
    let condition : [String: Any]
}

struct ApiSetDataBody {
    let name: String
    let token: String
    let operation: String
    let data: [String: Any]
}
