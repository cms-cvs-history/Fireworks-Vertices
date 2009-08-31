// -*- C++ -*-
// $Id: FWSecVertex3DProxyBuilder.cc,v 1.1 2009/08/29 21:00:18 dmytro Exp $
//

// include files
#include "Fireworks/Core/interface/FW3DSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "DataFormats/BTauReco/interface/SecondaryVertexTagInfo.h"
#include "TEvePointSet.h"
#include "TEveTrack.h"
#include "TEveBoxSet.h"
#include "TGeoSphere.h"
#include "TEveGeoNode.h"
#include "TEveTrackPropagator.h"
#include "Fireworks/Core/interface/FWEvePtr.h"
#include "Fireworks/Core/src/CmsShowMain.h"
#include "DataFormats/TrackReco/interface/Track.h"

#include "TMatrixDEigen.h"
#include "TMatrixDSym.h"  
#include "TDecompSVD.h"
#include "TVectorD.h"
#include "TEveTrans.h"
#include <vector>

class FWSecVertex3DProxyBuilder : public FW3DSimpleProxyBuilderTemplate<reco::SecondaryVertexTagInfo>  {
   
public:
   FWSecVertex3DProxyBuilder(): 
    m_propagator( new TEveTrackPropagator)
{
   m_propagator->SetMagField( - CmsShowMain::getMagneticField() );
   m_propagator->SetMaxR(123.0);
   m_propagator->SetMaxZ(300.0);
}
   virtual ~FWSecVertex3DProxyBuilder(){}
   REGISTER_PROXYBUILDER_METHODS();
 
private:
   FWSecVertex3DProxyBuilder(const FWSecVertex3DProxyBuilder&); // stop default
   const FWSecVertex3DProxyBuilder& operator=(const FWSecVertex3DProxyBuilder&); // stop default
   
   // ---------- member data --------------------------------
   FWEvePtr<TEveTrackPropagator> m_propagator;

   void build(const reco::SecondaryVertexTagInfo& iData, unsigned int iIndex, TEveElement& oItemHolder) const;
};

void 
FWSecVertex3DProxyBuilder::build(const reco::SecondaryVertexTagInfo& iData, unsigned int iIndex,TEveElement& oItemHolder) const
{
  TEveGeoManagerHolder gmgr(TEveGeoShape::GetGeoMangeur());
    TEvePointSet* pointSet = new TEvePointSet();
    pointSet->SetMainColor(item()->defaultDisplayProperties().color());
    for(unsigned int i=0;i<iData.nVertices();i++)
    {
      const reco::Vertex & v = iData.secondaryVertex(i);
      // do we need this stuff?
      TGeoSphere * sphere = new TGeoSphere(0, 0.06); //would that leak?
      TGeoTranslation position(v.x(), v.y(), v.z() );
      TEveGeoShape * shape = new TEveGeoShape();
      sphere->SetBoxDimensions(2.5,2.5,2.5);
      shape->SetShape(sphere);
      shape->SetMainColor(item()->defaultDisplayProperties().color());
      shape->SetMainTransparency(10);
 
      TEveTrans & t =   shape->RefMainTrans();
      reco::Vertex::Error e= v.error();
      TMatrixDSym m(3);
      for(int i=0;i<3;i++)
       for(int j=0;j<3;j++)
        {
          m(i,j) = e(i,j);
        }
      TMatrixDEigen eig(m);
      TDecompSVD svd(m);
      TMatrixD mm = svd.GetU();
 //   TMatrixD mm =  eig.GetEigenVectors().Print();
      for(int i=0;i<3;i++)
       for(int j=0;j<3;j++)
        {
           t(i+1,j+1) = mm(i,j);
        }   
      TVectorD vv ( eig.GetEigenValuesRe())   ; 
      t.Scale(sqrt(vv(0))*100.,sqrt(vv(1))*100.,sqrt(vv(2))*100.);
      t.SetPos(v.x(),v.y(),v.z());

      oItemHolder.AddElement(shape);

      pointSet->SetNextPoint( v.x(), v.y(), v.z() );

    //  TEveLine* line = new TEveLine();
    //  pointSet->SetMainColor(item()->defaultDisplayProperties().color());
    //  line->SetNextPoint( v.x(), v.y(), v.z() );
    //  oItemHolder.AddElement(line);
 


      for(reco::Vertex::trackRef_iterator it = v.tracks_begin() ; 
          it != v.tracks_end()  ; ++it)
      {
      const reco::Track & track = *it->get();
      TEveRecTrack t;
      t.fBeta = 1.;
      t.fV = TEveVector(track.vx(), track.vy(), track.vz());
      t.fP = TEveVector(track.px(), track.py(), track.pz());
      t.fSign = track.charge();
      TEveTrack* trk = new TEveTrack(&t,m_propagator.get());
      trk->SetMainColor(item()->defaultDisplayProperties().color());
      trk->MakeTrack();
      oItemHolder.AddElement( trk );
      }

    }
    oItemHolder.AddElement( pointSet );


}

REGISTER_FW3DDATAPROXYBUILDER(FWSecVertex3DProxyBuilder,reco::SecondaryVertexTagInfo,"SecVertex");
