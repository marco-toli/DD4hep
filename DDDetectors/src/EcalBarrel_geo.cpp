//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
//
// Specialized generic detector constructor
// 
//==========================================================================
#include "DD4hep/DetFactoryHelper.h"
#include "XML/Layering.h"

using namespace std;
using namespace dd4hep;
using namespace dd4hep::detail;

static Ref_t create_detector(Detector& description, xml_h e, SensitiveDetector sens)  {
  static double tolerance = 0e0;


  std::cout<<"Will Robinson: creating Barrel ECAL"<<std::endl;

  Layering      layering (e);
  xml_det_t     x_det     = e;
  Material      air       = description.air();
  int           det_id    = x_det.id(); std::cout<<"det_id="<<det_id<<std::endl;
  string        det_name  = x_det.nameStr();std::cout<<"det_name="<<det_name<<std::endl;
  xml_comp_t    x_staves  = x_det.staves();
  xml_comp_t    x_dim     = x_det.dimensions();
  int           nsides    = x_dim.numsides();std::cout<<"nsides="<<nsides<<std::endl;
  double        inner_r   = x_dim.rmin();std::cout<<"inner_r="<<inner_r<<std::endl;
  double        dphi      = (2*M_PI/nsides);std::cout<<"dphi="<<dphi<<std::endl;
  double        hphi      = dphi/2;std::cout<<"hphi="<<hphi<<std::endl;
  double        mod_z     = layering.totalThickness();std::cout<<"mod_z="<<mod_z<<std::endl;
  double        outer_r   = inner_r + mod_z;std::cout<<"outer_r="<<outer_r<<std::endl;
  double        totThick  = mod_z;std::cout<<"totThick="<<totThick<<std::endl;
  DetElement    sdet      (det_name,det_id);
  Volume        motherVol = description.pickMotherVolume(sdet);
  PolyhedraRegular hedra  (nsides,inner_r,inner_r+totThick+tolerance*2e0,x_dim.z());
  Volume        envelope  (det_name,hedra,air);
  PlacedVolume  env_phv   = motherVol.placeVolume(envelope,RotationZYX(0,0,M_PI/nsides));

  env_phv.addPhysVolID("system",det_id);
  env_phv.addPhysVolID("barrel",0);
  sdet.setPlacement(env_phv);

  DetElement    stave_det("stave0",det_id);
  double dx = 0.0; //mod_z / std::sin(dphi); // dx per layer
    
  // Compute the top and bottom face measurements.
  double trd_x2 = (2 * std::tan(hphi) * outer_r - dx)/2 - tolerance;std::cout<<"trd_x2="<<trd_x2<<std::endl;
  double trd_x1 = (2 * std::tan(hphi) * inner_r + dx)/2 - tolerance;std::cout<<"trd_x1="<<trd_x1<<std::endl;
  double trd_y1 = x_dim.z()/2 - tolerance;std::cout<<"trd_y1="<<trd_y1<<std::endl;
  double trd_y2 = trd_y1;std::cout<<"trd_y2="<<trd_y2<<std::endl;
  double trd_z  = mod_z/2 - tolerance; std::cout<<"trd_z="<<trd_z<<std::endl;
		
  // Create the trapezoid for the stave.
  Trapezoid trd(trd_x1, // Outer side, i.e. the "short" X side.
                trd_x2, // Inner side, i.e. the "long"  X side.
                trd_y1, // Corresponds to subdetector (or module) Z.
                trd_y2, //
                trd_z); // Thickness, in Y for top stave, when rotated.

  Volume mod_vol("stave",trd,air);

  sens.setType("calorimeter");
  { // =====  buildBarrelStave(description, sens, module_volume) =====
    std::cout<<"building stave"<<std::endl;
    // Parameters for computing the layer X dimension:
    double stave_z  = trd_y1;std::cout<<"stave_z="<<stave_z<<std::endl;
    double tan_hphi = std::tan(hphi);std::cout<<"tan_hphi="<<tan_hphi<<std::endl;
    double l_dim_x  = trd_x1; // Starting X dimension for the layer.
    std::cout<<"l_dim_x="<<l_dim_x<<std::endl;
    double l_pos_z  = -(layering.totalThickness() / 2);std::cout<<"l_pos_z="<<l_pos_z<<std::endl;

    // Loop over the sets of layer elements in the detector.
    int l_num = 1;
    for(xml_coll_t li(x_det,_U(layer)); li; ++li)  {
      std::cout<<"    l_num="<<l_num<<std::endl;
      xml_comp_t x_layer = li;
      int repeat = x_layer.repeat();std::cout<<"   repeat="<<repeat<<std::endl;
      // Loop over number of repeats for this layer.
      for (int j=0; j<repeat; j++)    {
	std::cout<<"      j="<<j<<std::endl;
        string l_name = _toString(l_num,"layer%d");std::cout<<"      l_name="<<l_name<<std::endl;
        double l_thickness = layering.layer(l_num-1)->thickness();  // Layer's thickness.
	std::cout<<"      l_thickness="<<l_thickness<<std::endl;

        Position   l_pos(0,0,l_pos_z+l_thickness/2);      // Position of the layer.
	std::cout<<"      l_pos=("<<l_pos.x()<<","<<l_pos.y()<<","<<l_pos.z()<<std::endl;
        Box        l_box(l_dim_x-tolerance,stave_z-tolerance,l_thickness / 2-tolerance);
        Volume     l_vol(l_name,l_box,air);
        DetElement layer(stave_det, l_name, det_id);

        // Loop over the sublayers or slices for this layer.
        int s_num = 1;
        double s_pos_z = -(l_thickness / 2);
        for(xml_coll_t si(x_layer,_U(slice)); si; ++si)  {
	  std::cout<<"         s_num="<<s_num<<std::endl;
          xml_comp_t x_slice = si;
          string     s_name  = _toString(s_num,"slice%d");
          double     s_thick = x_slice.thickness();
          Box        s_box(l_dim_x-tolerance,stave_z-tolerance,s_thick / 2-tolerance);
          Volume     s_vol(s_name,s_box,description.material(x_slice.materialStr()));
          DetElement slice(layer,s_name,det_id);

          if ( x_slice.isSensitive() ) {
            s_vol.setSensitiveDetector(sens);
          }
          slice.setAttributes(description,s_vol,x_slice.regionStr(),x_slice.limitsStr(),x_slice.visStr());

          // Slice placement.
          PlacedVolume slice_phv = l_vol.placeVolume(s_vol,Position(0,0,s_pos_z+s_thick/2));
          slice_phv.addPhysVolID("slice", s_num);
          slice.setPlacement(slice_phv);
          // Increment Z position of slice.
          s_pos_z += s_thick;
	  std::cout<<"         s_pos_z="<<s_pos_z<<std::endl;
                                        
          // Increment slice number.
          ++s_num;
        }        

        // Set region, limitset, and vis of layer.
        layer.setAttributes(description,l_vol,x_layer.regionStr(),x_layer.limitsStr(),x_layer.visStr());

        PlacedVolume layer_phv = mod_vol.placeVolume(l_vol,l_pos);
        layer_phv.addPhysVolID("layer", l_num);
        layer.setPlacement(layer_phv);
        // Increment to next layer Z position.
        double xcut = l_thickness * tan_hphi;
        l_dim_x += xcut;
        l_pos_z += l_thickness;          
        ++l_num;
      }
    }
  }

  // Set stave visualization.
  if ( x_staves )   {
    mod_vol.setVisAttributes(description.visAttributes(x_staves.visStr()));
  }
  // Phi start for a stave.
  double phi = M_PI / nsides;
  double mod_x_off = dx / 2;             // Stave X offset, derived from the dx.
  double mod_y_off = inner_r + mod_z/2;  // Stave Y offset

  std::cout<<" placing staves"<<std::endl;

  // Create nsides staves.
  for (int i = 0; i < nsides; i++, phi -= dphi)      { // i is module number
    std::cout<<"phi is "<<phi<<" i ="<<i<<std::endl;
    // Compute the stave position
    double m_pos_x = mod_x_off * std::cos(phi) - mod_y_off * std::sin(phi);std::cout<<"m_pos_x="<<m_pos_x<<std::endl;
    double m_pos_y = mod_x_off * std::sin(phi) + mod_y_off * std::cos(phi);std::cout<<"m_pos_y="<<m_pos_y<<std::endl;
    Transform3D tr(RotationZYX(0,phi,M_PI*0.5),Translation3D(-m_pos_x,-m_pos_y,0));
    PlacedVolume pv = envelope.placeVolume(mod_vol,tr);
    pv.addPhysVolID("system",det_id);
    pv.addPhysVolID("barrel",0);
    pv.addPhysVolID("module",i+1);
    DetElement sd = i==0 ? stave_det : stave_det.clone(_toString(i,"stave%d"));
    std::cout<<"DetEl id is "<<sd.id()<<std::endl;
    sd.setPlacement(pv);
    sdet.add(sd);
  }

  // Set envelope volume attributes.
  envelope.setAttributes(description,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());
  return sdet;
}

DECLARE_DETELEMENT(DD4hep_EcalBarrel,create_detector)
DECLARE_DEPRECATED_DETELEMENT(EcalBarrel,create_detector)
