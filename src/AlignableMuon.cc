/** \file
 *
 *  $Date: 2007/03/02 18:39:17 $
 *  $Revision: 1.15 $
 *  \author Andre Sznajder - UERJ(Brazil)
 */
 

// Framework
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Alignment/MuonAlignment/interface/AlignableMuon.h"
#include "Geometry/DTGeometry/interface/DTChamber.h" 
#include "Geometry/CSCGeometry/interface/CSCGeometry.h" 
#include <Geometry/DTGeometry/interface/DTLayer.h> 
#include "CondFormats/Alignment/interface/Alignments.h" 
#include "CondFormats/Alignment/interface/AlignmentErrors.h" 
#include "CondFormats/Alignment/interface/AlignmentSorter.h" 

// Muon  components
#include "Alignment/MuonAlignment/interface/AlignableDTChamber.h"
#include "Alignment/MuonAlignment/interface/AlignableCSCChamber.h"
#include "Alignment/MuonAlignment/interface/AlignableDTStation.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h" 
#include "Alignment/MuonAlignment/interface/AlignableCSCStation.h"
#include "Alignment/MuonAlignment/interface/AlignableDTWheel.h"
#include "Alignment/MuonAlignment/interface/AlignableDTBarrel.h"
#include "Alignment/MuonAlignment/interface/AlignableCSCEndcap.h"

//--------------------------------------------------------------------------------------------------
AlignableMuon::AlignableMuon( const DTGeometry* dtGeometry , const CSCGeometry* cscGeometry )
{

  // Build the muon barrel
  buildDTBarrel( dtGeometry );

  // Build the muon end caps
  buildCSCEndcap( cscGeometry );

  // Set links to mothers recursively
  recursiveSetMothers( this );

  edm::LogInfo("AlignableMuon") << "Constructing alignable muon objects DONE";


}

//--------------------------------------------------------------------------------------------------
AlignableMuon::~AlignableMuon() 
{

  for ( std::vector<Alignable*>::iterator iter=theMuonComponents.begin();
		iter != theMuonComponents.end(); iter++){
    delete *iter;
  }
      

}


//--------------------------------------------------------------------------------------------------
void AlignableMuon::buildDTBarrel( const DTGeometry* pDT  )
{
  
 LogDebug("Position") << "Constructing AlignableDTBarrel"; 

  // Temporary container for chambers in a given station and stations in a given wheel
  std::vector<AlignableDTChamber*>   tmpDTChambersInStation;
  std::vector<AlignableDTStation*>   tmpDTStationsInWheel;


  // Loop over wheels ( -2..2 )
  for( int iwh = -2 ; iwh < 3 ; iwh++ ){

    // Loop over stations ( 1..4 )
    for( int ist = 1 ; ist < 5 ; ist++ ){
  
      // Loop over geom DT Chambers
      std::vector<GeomDet*> theSLs;
      for( std::vector<DTChamber*>::const_iterator det = pDT->chambers().begin(); 
		   det != pDT->chambers().end(); ++det ){
        // Get the chamber ID
        DTChamberId chamberId = (*det)->id(); 
		
        // Get wheel,station and sector of the chamber
        int wh = chamberId.wheel();
        int st = chamberId.station();
        //int se = chamberId.sector();

        // Select the chambers in a given wheel in a given station
        if ( iwh == wh && ist == st ){

          // Create the alignable DT chamber
          AlignableDTChamber* tmpDTChamber  = new AlignableDTChamber( *det );
 
          // Store the DT chambers in a given DT Station and Wheel
		  tmpDTChambersInStation.push_back( tmpDTChamber );

		  // End chamber selection
		}

		// End loop over chambers
      }  
	  
      // Store the DT chambers 
      theDTChambers.insert( theDTChambers.end(), tmpDTChambersInStation.begin(),
                            tmpDTChambersInStation.end() );

      // Create the alignable DT station with chambers in a given station and wheel 
      AlignableDTStation* tmpDTStation  = new AlignableDTStation( tmpDTChambersInStation );
     
      // Store the DT stations in a given wheel  
      tmpDTStationsInWheel.push_back( tmpDTStation );

      // Clear the temporary vector of chambers in a station
      tmpDTChambersInStation.clear();

    // End loop over stations
    }

    // Store The DT stations
	theDTStations.insert( theDTStations.end(), tmpDTStationsInWheel.begin(),
						  tmpDTStationsInWheel.end() );

    // Create the alignable DT wheel
    AlignableDTWheel* tmpWheel  = new AlignableDTWheel( tmpDTStationsInWheel );
     

    // Store the DT wheels  
    theDTWheels.push_back( tmpWheel );

    // Clear temporary vector of stations in a wheel
    tmpDTStationsInWheel.clear();


  // End loop over wheels   
  }    
          
  // Create the alignable Muon Barrel
  AlignableDTBarrel* tmpDTBarrel  = new AlignableDTBarrel( theDTWheels );  
  
  // Store the barrel
  theDTBarrel.push_back( tmpDTBarrel );

  // Store the barrel in the muon 
  theMuonComponents.push_back( tmpDTBarrel );


}



//--------------------------------------------------------------------------------------------------


void AlignableMuon::buildCSCEndcap( const CSCGeometry* pCSC  )
{
  
 LogDebug("Position") << "Constructing AlignableCSCBarrel"; 

  // Temporary container for chambers in a given station
  std::vector<AlignableCSCStation*>  tmpCSCStationsInEndcap;

  // Loop over endcaps ( 1..2 )
  for( int iec = 1 ; iec < 3 ; iec++ ){

    // Temporary container for chambers in a given station
    std::vector<AlignableCSCChamber*>   tmpCSCChambersInStation;

    // Loop over stations ( 1..4 )
    for( int ist = 1 ; ist < 5 ; ist++ ){
  
      // Loop over geom CSC Chambers
      std::vector<CSCChamber*> vc = pCSC->chambers();
      for( std::vector<CSCChamber*>::const_iterator det = vc.begin();  
                                                   det != vc.end(); ++det ){

        // Get the CSCDet ID
        CSCDetId cscId = (*det)->id();

        // Get chamber, station, ring, layer and endcap labels of the CSC chamber
        int ec = cscId.endcap();
        //int ch = cscId.chamber();
        int st = cscId.station();

        // Select the chambers in a given endcap in a given station
        if ( iec == ec && ist == st ) {

          // Create the alignable CSC chamber 
          AlignableCSCChamber* tmpCSCChamber  = new AlignableCSCChamber( *det );
  
          // Store the alignable CSC chambers
          tmpCSCChambersInStation.push_back( tmpCSCChamber );    

        // End If chamber selection
        }

      // End loop over geom CSC chambers
      }

      // Store the alignable CSC chambers
      theCSCChambers.insert(  theCSCChambers.end(), tmpCSCChambersInStation.begin(), tmpCSCChambersInStation.end() );    

      // Create the alignable CSC station with chambers in a given station 
      AlignableCSCStation* tmpCSCStation  = new AlignableCSCStation( tmpCSCChambersInStation );
     
      // Store the CSC stations in a given endcap  
      tmpCSCStationsInEndcap.push_back( tmpCSCStation );

      // Clear the temporary vector of chambers in station
      tmpCSCChambersInStation.clear();

    // End loop over stations
    }

    // Create the alignable CSC endcap 
    AlignableCSCEndcap* tmpEndcap  = new AlignableCSCEndcap( tmpCSCStationsInEndcap );
     
    // Store the alignable CSC stations 
      theCSCStations.insert(  theCSCStations.end(), tmpCSCStationsInEndcap.begin(), tmpCSCStationsInEndcap.end() );

    // Store the alignable CSC endcaps
    theCSCEndcaps.push_back( tmpEndcap );

    // Clear the temporary vector of stations in endcap
    tmpCSCStationsInEndcap.clear();

  // End loop over endcaps  
  }

  // Store the encaps in the muon components  
  theMuonComponents.insert(  theMuonComponents.end(), theCSCEndcaps.begin(), theCSCEndcaps.end() );    

    
}

//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::DTSuperLayers()
{
  std::vector<Alignable*> result;

  std::vector<Alignable*> chambers = DTChambers();
  for (std::vector<Alignable*>::const_iterator chamberIter = chambers.begin();  chamberIter != chambers.end();  ++chamberIter) {
     std::vector<Alignable*> superlayers = (*chamberIter)->components();
     for (std::vector<Alignable*>::const_iterator superlayerIter = superlayers.begin();  superlayerIter != superlayers.end();  ++superlayerIter) {
	result.push_back(*superlayerIter);
     }
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::DTChambers()
{
  std::vector<Alignable*> result;
  copy( theDTChambers.begin(), theDTChambers.end(), back_inserter(result) );
  return result;
}

//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::DTStations()
{
  std::vector<Alignable*> result;
  copy( theDTStations.begin(), theDTStations.end(), back_inserter(result) );
  return result;
}


//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::DTWheels()
{
  std::vector<Alignable*> result;
  copy( theDTWheels.begin(), theDTWheels.end(), back_inserter(result) );
  return result;
}

//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::DTBarrel()
{
  std::vector<Alignable*> result ;
  copy( theDTBarrel.begin(), theDTBarrel.end(), back_inserter(result) );
  return result;
}

//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::CSCLayers()
{
  std::vector<Alignable*> result;

  std::vector<Alignable*> chambers = CSCChambers();
  for (std::vector<Alignable*>::const_iterator chamberIter = chambers.begin();  chamberIter != chambers.end();  ++chamberIter) {
     std::vector<Alignable*> layers = (*chamberIter)->components();
     for (std::vector<Alignable*>::const_iterator layerIter = layers.begin();  layerIter != layers.end();  ++layerIter) {
	result.push_back(*layerIter);
     }
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::CSCChambers()
{
  std::vector<Alignable*> result;
  copy( theCSCChambers.begin(), theCSCChambers.end(), back_inserter(result) );
  return result;
}

//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::CSCStations()
{
  std::vector<Alignable*> result;
  copy( theCSCStations.begin(), theCSCStations.end(), back_inserter(result) );
  return result;
}

//--------------------------------------------------------------------------------------------------
std::vector<Alignable*> AlignableMuon::CSCEndcaps()
{
  std::vector<Alignable*> result;
  copy( theCSCEndcaps.begin(), theCSCEndcaps.end(), back_inserter(result) );
  return result;
}


//__________________________________________________________________________________________________
void AlignableMuon::recursiveSetMothers( Alignable* alignable )
{
  
  std::vector<Alignable*> components = alignable->components();
  for ( std::vector<Alignable*>::iterator iter = components.begin();
		iter != components.end(); iter++ )
	{
	  (*iter)->setMother( alignable );
	  recursiveSetMothers( *iter );
	}

}
//__________________________________________________________________________________________________
Alignments* AlignableMuon::alignments( void ) const
{

  std::vector<Alignable*> comp = this->components();
  Alignments* m_alignments = new Alignments();
  // Add components recursively
  for ( std::vector<Alignable*>::iterator i=comp.begin(); i!=comp.end(); i++ )
    {
      Alignments* tmpAlignments = (*i)->alignments();
      std::copy( tmpAlignments->m_align.begin(), tmpAlignments->m_align.end(), 
				 std::back_inserter(m_alignments->m_align) );
	  delete tmpAlignments;
    }

  std::sort( m_alignments->m_align.begin(), m_alignments->m_align.end(), 
			 lessAlignmentDetId<AlignTransform>() );

  return m_alignments;

}
//__________________________________________________________________________________________________
AlignmentErrors* AlignableMuon::alignmentErrors( void ) const
{

  std::vector<Alignable*> comp = this->components();
  AlignmentErrors* m_alignmentErrors = new AlignmentErrors();

  // Add components recursively
  for ( std::vector<Alignable*>::iterator i=comp.begin(); i!=comp.end(); i++ )
    {
	  AlignmentErrors* tmpAlignmentErrors = (*i)->alignmentErrors();
      std::copy( tmpAlignmentErrors->m_alignError.begin(), tmpAlignmentErrors->m_alignError.end(), 
				 std::back_inserter(m_alignmentErrors->m_alignError) );
	  delete tmpAlignmentErrors;
    }

  std::sort( m_alignmentErrors->m_alignError.begin(), m_alignmentErrors->m_alignError.end(), 
			 lessAlignmentDetId<AlignTransformError>() );

  return m_alignmentErrors;

}
//__________________________________________________________________________________________________
Alignments* AlignableMuon::dtAlignments( void )
{
  // Retrieve muon barrel alignments
  Alignments* tmpAlignments = this->DTBarrel().front()->alignments();
  
  return tmpAlignments;

}
//__________________________________________________________________________________________________
AlignmentErrors* AlignableMuon::dtAlignmentErrors( void )
{
  // Retrieve muon barrel alignment errors
  AlignmentErrors* tmpAlignmentErrors = this->DTBarrel().front()->alignmentErrors();

  return tmpAlignmentErrors;
  
}
//__________________________________________________________________________________________________
Alignments* AlignableMuon::cscAlignments( void )
{

  // Retrieve muon endcaps alignments
  Alignments* cscEndCap1    = this->CSCEndcaps().front()->alignments();
  Alignments* cscEndCap2    = this->CSCEndcaps().back()->alignments();
  Alignments* tmpAlignments = new Alignments();

  std::copy( cscEndCap1->m_align.begin(), cscEndCap1->m_align.end(), back_inserter( tmpAlignments->m_align ) );
  std::copy( cscEndCap2->m_align.begin(), cscEndCap2->m_align.end(), back_inserter( tmpAlignments->m_align ) );
  
  return tmpAlignments;

}
//__________________________________________________________________________________________________
AlignmentErrors* AlignableMuon::cscAlignmentErrors( void )
{

  // Retrieve muon endcaps alignment errors
   AlignmentErrors* cscEndCap1Errors = this->CSCEndcaps().front()->alignmentErrors();
   AlignmentErrors* cscEndCap2Errors = this->CSCEndcaps().back()->alignmentErrors();
   AlignmentErrors* tmpAlignmentErrors    = new AlignmentErrors();

  std::copy(cscEndCap1Errors->m_alignError.begin(), cscEndCap1Errors->m_alignError.end(), back_inserter(tmpAlignmentErrors->m_alignError) );
  std::copy(cscEndCap2Errors->m_alignError.begin(), cscEndCap2Errors->m_alignError.end(), back_inserter(tmpAlignmentErrors->m_alignError) );
  
  return tmpAlignmentErrors;
  
}
