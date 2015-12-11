//
//  CoffeeAnnotation.swift
//  Coffee
//
//  Created by Geosat-RD01 on 2015/12/10.
//  Copyright © 2015年 Geosat-RD01. All rights reserved.
//

import MapKit

class CoffeeAnnotation: NSObject, MKAnnotation{
  
  let title:String?
  let subtitle:String?
  let coordinate: CLLocationCoordinate2D
  
  init(title: String?, subtitle:String?, coordinate: CLLocationCoordinate2D){
    self.title = title
    self.subtitle = subtitle
    self.coordinate = coordinate
    
    super.init()
  }
  
}
