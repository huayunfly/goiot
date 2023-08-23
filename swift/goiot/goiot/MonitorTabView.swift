//
//  MonitorTabView.swift
//  goiot
//
//  Created by YUN HUA on 2023/8/10.
//

import SwiftUI

struct MonitorTabView: View {
    var body: some View {
        HStack(alignment: .oneThird) {
            Rectangle()
                .foregroundColor(Color.green)
                .frame(width: 50, height: 200)
            Rectangle()
                .foregroundColor(Color.red)

                .frame(width: 50, height: 200)
            Rectangle()
                .foregroundColor(Color.blue)
                .frame(width: 50, height: 200)
            Rectangle()
                .foregroundColor(Color.orange)
                .alignmentGuide(.oneThird,
                     computeValue: { d in d[VerticalAlignment.top] })
                .frame(width: 50, height: 200)
        }
        Text("First Content View")
    }
}

extension VerticalAlignment {
    private enum OneThird : AlignmentID {
        static func defaultValue(in d: ViewDimensions) -> CGFloat {
     return d.height / 4 }   }
    static let oneThird = VerticalAlignment(OneThird.self)
}

struct MonitorTabView_Previews: PreviewProvider {
    static var previews: some View {
        MonitorTabView()
    }
}
