//
//  ViewController.swift
//  Coffee
//
//  Created by Geosat-RD01 on 2015/11/30.
//  Copyright © 2015年 Geosat-RD01. All rights reserved.
//

import UIKit
import MapKit
import RealmSwift


class ViewController: UIViewController{
  var lastLocation : CLLocation?
  var venues:[Venue]?
  
  @IBOutlet var mapView:MKMapView?
  @IBOutlet var tableView:UITableView?
  
  var locationManager : CLLocationManager?
  let distanceSpan : Double = 500
  
  override func viewDidLoad() {
    super.viewDidLoad()
    NSNotificationCenter.defaultCenter().addObserver(self, selector: Selector("onVenuesUpdated:"), name: API.notifications.venuesUpdated, object: nil)
    
  }
  
  override func viewWillAppear(animated: Bool) {
    super.viewWillAppear(true)
    if let mapView = self.mapView{
      mapView.delegate = self
    }
    if locationManager == nil {
      locationManager = CLLocationManager()
      
      locationManager!.delegate = self
      //This method will cause a popup in the app asking for permission to use the GPS location data.
      locationManager!.desiredAccuracy = kCLLocationAccuracyBestForNavigation
      locationManager!.requestAlwaysAuthorization()
      // Don't send location updates with a distance smaller than 50 meters between them
      locationManager!.distanceFilter = 50
      locationManager!.startUpdatingLocation()
    }
    
    if let tableView = self.tableView{
      tableView.delegate = self
      tableView.dataSource = self
    }
  }
  
  override func didReceiveMemoryWarning() {
    super.didReceiveMemoryWarning()
    
  }
  
  /***  Function area  ***/
  
  func onVenuesUpdated(notification:NSNotification){
    refreshVenues(nil)
  }
  
  func refreshVenues(location: CLLocation?, getDataFromFoursquare:Bool = false){
    if location != nil{
      lastLocation = location
    }
    
    if let location = lastLocation{
      if getDataFromFoursquare == true{
        CoffeeAPI.sharedInstance.getCoffeeShopsWithLocation(location)
      }
      
      let (start, stop) = calculateCoordinatesWithRegion(location)
      
      let predicate = NSPredicate(format: "latitude < %f AND latitude > %f AND longitude > %f AND longitude < %f", start.latitude, stop.latitude, start.longitude, stop.longitude)
      
      let realm = try! Realm()
      
      venues = realm.objects(Venue).filter(predicate).sort {
        //The $0 and $1 are shorthands for the two unsorted objects.
        location.distanceFromLocation($0.coordinate) < location.distanceFromLocation($1.coordinate)
      }
      
      for venue in venues!{
        let annotation = CoffeeAnnotation(title: venue.name, subtitle: venue.address, coordinate: CLLocationCoordinate2D(latitude: Double(venue.latitude), longitude: Double(venue.longitude)))
        
        mapView?.addAnnotation(annotation)
      }
      tableView?.reloadData()
    }
  }
  
  func calculateCoordinatesWithRegion(location:CLLocation) -> (CLLocationCoordinate2D, CLLocationCoordinate2D){
    let region = MKCoordinateRegionMakeWithDistance(location.coordinate, distanceSpan, distanceSpan)
    
    var start:CLLocationCoordinate2D = CLLocationCoordinate2D()
    var stop:CLLocationCoordinate2D = CLLocationCoordinate2D()
    
    start.latitude  = region.center.latitude  + (region.span.latitudeDelta  / 2.0)
    start.longitude = region.center.longitude - (region.span.longitudeDelta / 2.0)
    stop.latitude   = region.center.latitude  - (region.span.latitudeDelta  / 2.0)
    stop.longitude  = region.center.longitude + (region.span.longitudeDelta / 2.0)
    
    return (start, stop)
  }
  
}

// MARK: - MKMapViewDelegate
extension ViewController : MKMapViewDelegate{
  func mapView(mapView: MKMapView, viewForAnnotation annotation: MKAnnotation) -> MKAnnotationView?{
    
    if annotation.isKindOfClass(MKUserLocation){
      return nil
    }
    
    var view = mapView.dequeueReusableAnnotationViewWithIdentifier("annotationIdentifier")
    
    if view == nil{
      view = MKPinAnnotationView(annotation: annotation, reuseIdentifier: "annotationIdentifier")
    }
    
    view?.canShowCallout = true
    return view
  }
}

// MARK: - CLLocationManagerDelegate
extension ViewController : CLLocationManagerDelegate{
  func locationManager(manager: CLLocationManager, didUpdateToLocation newLocation: CLLocation, fromLocation oldLocation: CLLocation) {
    if let mapView = self.mapView {
      let region = MKCoordinateRegionMakeWithDistance(newLocation.coordinate, distanceSpan, distanceSpan)
      mapView.setRegion(region, animated: true)
      refreshVenues(newLocation, getDataFromFoursquare: true)
    }
  }
}

// MARK: - UITableViewDelegate
extension ViewController : UITableViewDelegate{
  func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath){
    if let venue = venues?[indexPath.row]{
      let region = MKCoordinateRegionMakeWithDistance(CLLocationCoordinate2D(latitude: Double(venue.latitude), longitude: Double(venue.longitude)), distanceSpan, distanceSpan)
      mapView?.setRegion(region, animated: true)
    }
  }
}

// MARK: - UITableViewDataSource
extension ViewController : UITableViewDataSource{
  
  func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int{
    //Note that the ?? is called a nil-coalescing operator. It basically says: when venues is nil, use 0 as a default value.
    return venues?.count ?? 0
  }
  
  func numberOfSectionsInTableView(tableView: UITableView) -> Int{
    return 1
  }
  
  func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell{
    var cell = tableView.dequeueReusableCellWithIdentifier("cellIdentifier");
    
    if cell == nil{
      cell = UITableViewCell(style: UITableViewCellStyle.Subtitle, reuseIdentifier: "cellIdentifier")
    }
    
    if let venue = venues?[indexPath.row]{
      cell!.textLabel?.text = venue.name
      cell!.detailTextLabel?.text = venue.address
    }
    
    return cell!
  }
}

