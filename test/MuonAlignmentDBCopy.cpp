#include <string>
#include <map>
#include <vector>

// Framework
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/MakerMacros.h"

// Database
#include "FWCore/Framework/interface/ESHandle.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

// Alignment
#include "CondFormats/Alignment/interface/Alignments.h"
#include "CondFormats/Alignment/interface/AlignmentErrors.h"
#include "CondFormats/AlignmentRecord/interface/DTAlignmentRcd.h"
#include "CondFormats/AlignmentRecord/interface/DTAlignmentErrorRcd.h"
#include "CondFormats/AlignmentRecord/interface/CSCAlignmentRcd.h"
#include "CondFormats/AlignmentRecord/interface/CSCAlignmentErrorRcd.h"


class  MuonAlignmentDBCopy : public edm::EDAnalyzer {
 public:

  explicit  MuonAlignmentDBCopy(const edm::ParameterSet& iConfig ):
  theDTAlignRecordName( "DTAlignments" ),
  theDTErrorRecordName( "DTAlignmentErrors" ),
  theCSCAlignRecordName( "CSCAlignments" ),
  theCSCErrorRecordName( "CSCAlignmentErrors" )
  {};

  ~MuonAlignmentDBCopy() {}

  virtual void analyze( const edm::Event& evt, const edm::EventSetup& evtSetup);

 private:

  std::string theDTAlignRecordName, theDTErrorRecordName;
  std::string theCSCAlignRecordName, theCSCErrorRecordName;
 
  Alignments* myDTAlignments;
  AlignmentErrors* myDTAlignmentErrors;
  Alignments* myCSCAlignments;
  AlignmentErrors* myCSCAlignmentErrors;

};

  


void MuonAlignmentDBCopy::analyze( const edm::Event& evt, const edm::EventSetup& iSetup)
{

  // 1. Retrieve DT alignment[Error]s from DBase
  edm::ESHandle<Alignments> dtAlignments;
  iSetup.get<DTAlignmentRcd>().get( dtAlignments );
  edm::ESHandle<AlignmentErrors> dtAlignmentErrors;
  iSetup.get<DTAlignmentErrorRcd>().get( dtAlignmentErrors );

  //   Create new objects we own
  myDTAlignments = new Alignments( *dtAlignments );
  myDTAlignmentErrors = new AlignmentErrors( *dtAlignmentErrors );

  // 2. Retrieve CSC alignment[Error]s from DBase
  edm::ESHandle<Alignments> cscAlignments;
  iSetup.get<CSCAlignmentRcd>().get( cscAlignments );
  edm::ESHandle<AlignmentErrors> cscAlignmentErrors;
  iSetup.get<CSCAlignmentErrorRcd>().get( cscAlignmentErrors );

  //   Create new objects we own
  myCSCAlignments = new Alignments( *cscAlignments );
  myCSCAlignmentErrors = new AlignmentErrors( *cscAlignmentErrors );

  // 2. Store alignment[Error]s to DB
  edm::Service<cond::service::PoolDBOutputService> poolDbService;

  // Call service
  if( !poolDbService.isAvailable() ) // Die if not available
	throw cms::Exception("NotAvailable") << "PoolDBOutputService not available";
	  	  
  // Store DT alignments and errors
  poolDbService->writeOne<Alignments>( &(*myDTAlignments), poolDbService->currentTime(), theDTAlignRecordName);
  poolDbService->writeOne<AlignmentErrors>( &(*myDTAlignmentErrors), poolDbService->currentTime(), theDTErrorRecordName);
							  
  // Store CSC alignments and errors
  poolDbService->writeOne<Alignments>( &(*myCSCAlignments), poolDbService->currentTime(), theCSCAlignRecordName);
  poolDbService->writeOne<AlignmentErrors>( &(*myCSCAlignmentErrors), poolDbService->currentTime(), theCSCErrorRecordName);

}

//define this as a plug-in
DEFINE_FWK_MODULE(MuonAlignmentDBCopy);


