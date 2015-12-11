//
//  Venue.swift
//  Coffee
//
//  Created by Geosat-RD01 on 2015/12/10.
//  Copyright © 2015年 Geosat-RD01. All rights reserved.
//

import RealmSwift
import MapKit

class Venue: Object{
  /***  Use dynamic cause:
  
  The dynamic property ensures that the property can be accessed via the Objective-C runtime.
  Before Swift 2.0 all Swift code ran in the Objective-C runtime, but now Swift’s got its own runtime. By marking a property as dynamic, the Objective-C runtime can access it, which is in turn needed because Realm relies on it internally.
  
  ***/
  
  dynamic var id:String = ""
  dynamic var name:String = ""
  
  dynamic var latitude:Float = 0
  dynamic var longitude:Float = 0
  
  dynamic var address:String = ""
  
  var coordinate:CLLocation {
    return CLLocation(latitude: Double(latitude), longitude: Double(longitude));
  }
  
  override static func primaryKey() -> String?{
  return "id";
  }
  
}
