#include "TrackNNAnalysis/TrackingMetricsTool.h"

std::vector<std::shared_ptr<FinalTrackTreeNode>> TrackingMetricsTool::HitWarrior(std::vector<std::shared_ptr<FinalTrackTreeNode>> /*finalHits*/, float /*nnCut*/)
{
    std::vector<std::shared_ptr<FinalTrackTreeNode>> NNOverlapTracks;
    //// Order the track by their NN score
    //sort(finalHits.begin(), finalHits.end(), 
    //[](const std::shared_ptr<FinalTrackTreeNode> & a, const std::shared_ptr<FinalTrackTreeNode> & b) -> bool
    //{ 
    //    return a->getNNScore() > b->getNNScore(); 
    //});

    //// HW algorithm
    //std::set<int> indexToRemove;

    //for(int i = 0; i < finalHits.size(); i++)
    //{
    //    // This has already been removed, don't worry about it
    //    if(indexToRemove.find(i) != indexToRemove.end()) continue;

    //    for(int j = i+1; j < finalHits.size(); j++)
    //    {

    //        auto nOverlapHits = finalHits[j]->getNHitOverlap(finalHits[i]);
    //        if(nOverlapHits >= HWThreshold)
    //        {
    //            // Since it is already sorted, just reject jth track
    //            indexToRemove.insert(j);
    //        }
    //    }
    //}

    //for(int i = 0; i < finalHits.size(); i++)
    //{
    //    if(indexToRemove.find(i) != indexToRemove.end()) continue;
    //    NNOverlapTracks.push_back (finalHits[i]);

    //}

    return NNOverlapTracks;
}

std::vector<std::shared_ptr<FinalTrackTreeNode>> TrackingMetricsTool::SelectNNTracks(std::vector<TrackTree*> /*trackTrees*/, float /*nnCut*/)
{
    // For each seed, select the track with the highest NN score and above the cut
    std::vector<std::shared_ptr<FinalTrackTreeNode>> NNSelectedTracks;
    //for(auto& tree: trackTrees)
    //{
    //    float maxScore = -1;
    //    std::shared_ptr<FinalTrackTreeNode> bestTrack;
    //    for (const auto& finalHitNode:tree->getFinalHits())
    //    {
    //        plots->Fill("AllDisc", finalHitNode->getNNScore()); // move this to efficiency function
    //        if(finalHitNode->getNNScore() < nnCut) continue; 
    //        if(finalHitNode->getNNScore() > maxScore)
    //        {
    //            maxScore = finalHitNode->getNNScore();
    //            bestTrack = finalHitNode;
    //        }
    //    }
    //    NNSelectedTracks.push_back (bestTrack);
    //}

    return NNSelectedTracks;
}

std::vector<std::shared_ptr<FinalTrackTreeNode>> TrackingMetricsTool::TDRAlgo(std::vector<TrackTree*> trackTrees, float nnCut)
{
    /////////////////////////////////////
    // This is like the TDR algo
    /////////////////////////////////////

    // For each seed, select the track with the highest NN score and above the cut
    auto NNSelectedTracks = TrackingMetricsTool::SelectNNTracks(trackTrees, nnCut);
    auto NNOverlapTracks = TrackingMetricsTool::HitWarrior(NNSelectedTracks, nnCut);
    return NNOverlapTracks;
}

void TrackingMetricsTool::efficiency(std::vector<std::shared_ptr<TrackTree*>> /*trackTrees*/, std::vector<std::shared_ptr<FinalTrackTreeNode>> /*finalHits*/)
{

    //// To init the counters
    //plots->addToCounter("nTracks", 0);
    //plots->addToCounter("nMatchedTracks", 0);
    //plots->addToCounter("nMatchedNNSelectedTracks", 0);
    //plots->addToCounter("nMatchedNNOverlapTracks", 0);
    //plots->addToCounter("nRoads", 0);

    ////calculate the efficiency.
    //// Each track tree corresponds to a seed
    //int nMatchedTracks = 0;
    //int nMatchedNNSelectedTracks = 0;
    //int nMatchedNNOverlapTracks = 0;
    //
    //for (auto& tree: trackTrees)
    //{
    //    auto trueTrack = tree->getSeed()->getBaseTrueTrack();

    //    //cut on track length
    //    if (finalHitNode->getDepth() < minHit) continue;
    //    //plots->Fill("AllDisc", track->getNNScore());
    //    plots->Fill("TruthAllZ0", trueTrack->getZ0());
    //    plots->Fill("TruthAllD0", trueTrack->getD0());
    //    plots->Fill("TruthAllPt", trueTrack->getPt());
    //    plots->Fill("TruthAllPhi", trueTrack->getPhi());
    //    plots->Fill("TruthAllEta", trueTrack->getEta());
    //    plots->Fill("TruthAllNHitsPerTrack", trueTrack->getNHits());

    //    plots->addToCounter("nTracks", 1);
    //    Index recoId = trueTrack->getBarcode();

    //    // find if any extrapolated tracks have at least 50% hits matching this track

    //    //////////////////////////////////////////////////////////////////////
    //    // Inclusive perf
    //    //////////////////////////////////////////////////////////////////////
    //    for (auto& finalHit: finalHits)
    //    {
    //        // consider only the hits after the seed
    //        std::vector<std::shared_ptr<Hit>> hitsList;

    //        if (finalHit->getDepth() < minHit) continue;
    //        bool isMatched = finalHit->isMatchedToReco(recoId, 0.5, 3);
    //        if (isMatched)
    //        {
    //            plots->addToCounter("nMatchedTracks", 1);
    //            
    //            nMatchedTracks++;
    //            plots->Fill("TruthMatchedZ0",               trueTrack->getZ0());
    //            plots->Fill("TruthMatchedD0",               trueTrack->getD0());
    //            plots->Fill("TruthMatchedPt",               trueTrack->getPt());
    //            plots->Fill("TruthMatchedPhi",              trueTrack->getPhi());
    //            plots->Fill("TruthMatchedEta",              trueTrack->getEta());
    //            plots->Fill("TruthMatchedNHitsPerTrack",    trueTrack->getNHits());
    //            break;
    //        }
    //    }

    //    //////////////////////////////////////////////////////////////////////
    //    // After NN Cut perf
    //    //////////////////////////////////////////////////////////////////////
    //    auto NNSelectedTracks = SelectNNTracks(finalHits, nnCut);
    //    for (auto& finalHit: NNSelectedTracks)
    //    {
    //        // consider only the hits after the seed
    //        std::vector<std::shared_ptr<Hit>> hitsList;

    //        if (finalHit->getDepth() < minHit) continue;
    //        bool isMatched = finalHit->isMatchedToReco(recoId, 0.5, 3);
    //        if (isMatched)
    //        {
    //            plots->addToCounter("nMatchedTracks", 1);
    //            
    //            plots->Fill("TruthMatchedNNSelectedZ0",               trueTrack->getZ0());
    //            plots->Fill("TruthMatchedNNSelectedD0",               trueTrack->getD0());
    //            plots->Fill("TruthMatchedNNSelectedPt",               trueTrack->getPt());
    //            plots->Fill("TruthMatchedNNSelectedPhi",              trueTrack->getPhi());
    //            plots->Fill("TruthMatchedNNSelectedEta",              trueTrack->getEta());
    //            plots->Fill("TruthMatchedNNSelectedNHitsPerTrack",    trueTrack->getNHits());
    //            break;
    //        }
    //    }

    //    //////////////////////////////////////////////////////////////////////
    //    // After NN Cut & HW perf
    //    //////////////////////////////////////////////////////////////////////
    //    auto NNOverlapTracks = HitWarrior(NNSelectedTracks, nnCut);
    //    for (auto& finalHit: NNOverlapTracks)
    //    {
    //        // consider only the hits after the seed
    //        std::vector<std::shared_ptr<Hit>> hitsList;

    //        if (finalHit->getDepth() < minHit) continue;
    //        bool isMatched = finalHit->isMatchedToReco(recoId, 0.5, 3);
    //        if (isMatched)
    //        {
    //            plots->addToCounter("nMatchedTracks", 1);

    //            plots->Fill("TruthMatchedNNOverlapZ0",               trueTrack->getZ0());
    //            plots->Fill("TruthMatchedNNOverlapD0",               trueTrack->getD0());
    //            plots->Fill("TruthMatchedNNOverlapPt",               trueTrack->getPt());
    //            plots->Fill("TruthMatchedNNOverlapPhi",              trueTrack->getPhi());
    //            plots->Fill("TruthMatchedNNOverlapEta",              trueTrack->getEta());
    //            plots->Fill("TruthMatchedNNOverlapNHitsPerTrack",    trueTrack->getNHits());
    //            break;
    //        }
    //    }
    //
    //}

    //plots->Fill("nTracksPerEvent", finalHits.size());
    //plots->Fill("nTracksPerEventMatched", nMatchedTracks);
    //plots->Fill("nTrueTracksPerEvent", trackTrees.size());

    //plots->addToCounter("nRoads", allCompleteTracks.size());

}
