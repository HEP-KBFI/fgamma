#ifndef CutoffStackingAction_h
#define CutoffStackingAction_h

#include <G4UserStackingAction.hh>

#include <fstream>

class CutoffStackingAction : public G4UserStackingAction {
	std::ostream &trackstream;

	public:
		CutoffStackingAction(std::ostream &trackstream);
		G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* tr);
		//void statistics();
};

#endif
