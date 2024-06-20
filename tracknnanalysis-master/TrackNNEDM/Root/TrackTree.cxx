#include "TrackNNEDM/TrackTree.h"

TrackTree::TrackTree(std::shared_ptr<SeedTrack> seed,  unsigned int maxSize):m_seed(seed), m_maxSize(maxSize)
{
    auto hitsList = seed->getHitsList();
    std::vector<std::shared_ptr<TrackTreeNode>> tmp_nodes;
    for (size_t i=0; i<hitsList.size(); i++)
    {
        auto hitNode = std::make_shared<TrackTreeNode> (hitsList.at(i));
        hitNode->setMaxBranches(m_maxBranches);
        tmp_nodes.push_back(hitNode);
    }
    this->addActiveHitNode(tmp_nodes.back());

    for (size_t i=0; i<tmp_nodes.size()-1; i++)
    {
        std::vector<std::shared_ptr<TrackTreeNode>> nextHits{tmp_nodes.at(i+1)};
        tmp_nodes.at(i)->addNextHits(nextHits);
    }
}

int TrackTreeNode::getDepth(int current)
{
    int depth = current;
    if(this->getPrevHit())
    {
        this->getPrevHit()->getDepth(current+1);
    }
    return depth+1;    
}

bool TrackTreeNode::addNextHit(std::shared_ptr<TrackTreeNode> hit)
{
    // return false if maxBranches has been reached and the hit was not added
    if(m_nActiveBranches == m_maxBranches)
    {
        return false;
    }

    // m_nextHits.push_back(hit); // implicit conversion from weak_ptr to shared_ptr

    hit->setPrevHit(shared_from_this()); 
    m_nActiveBranches++;
    return true;
}

bool TrackTreeNode::addNextHit(std::shared_ptr<FinalTrackTreeNode> hit)
{
    // Always add Final Hits

    hit->setPrevHit(shared_from_this()); 
    return true;
}

void TrackTreeNode::addNextHits(std::vector<std::shared_ptr<TrackTreeNode>> hits)
{ 
    // Curently unsafe due to branch limit
    for (auto& hit:hits)
    {
        if(m_nActiveBranches >= m_maxBranches)
        {
            break;
        }
        // m_nextHits.push_back(hit); // implicit conversion from weak_ptr to shared_ptr
        hit->setPrevHit(shared_from_this()); 
        m_nActiveBranches++;
    }
}

void TrackTreeNode::getHitsList(std::vector<std::shared_ptr<Hit>>& track, int nHits)
{
    // set nHits = -1 to use all hits
    if(nHits == 0 || !this->getPrevHit()) { return; }
    track.push_back(this->getHit());
    if(!this->getPrevHit()) { return; }
    nHits--;
    this->getPrevHit()->getHitsList(track, nHits);
}

// // Constructor to createa  FinalTrackTreeNode from a TrackTreeNode
// FinalTrackTreeNode::FinalTrackTreeNode(std::shared_ptr<TrackTreeNode> node)
// {

// }

int FinalTrackTreeNode::getNHitOverlap(std::shared_ptr<FinalTrackTreeNode> compNode)
{
    std::vector<std::shared_ptr<Hit>> currHitList;
    std::vector<std::shared_ptr<Hit>> compHitList;
    this->getHitsList(currHitList, -1);
    compNode->getHitsList(compHitList, -1);

    int match = 0;
    for(const auto& hit: currHitList)
    {
        for(const auto& toCompare: compHitList)
        {
            if(hit->distanceToX(toCompare) > 0.1) continue;
            if(hit->distanceToY(toCompare) > 0.1) continue;
            if(hit->distanceToZ(toCompare) > 0.1) continue;

            if(hit->distanceTo(toCompare) < 0.1)
            {
                compHitList.erase(std::remove(compHitList.begin(), compHitList.end(), toCompare), compHitList.end());
                match++;
                break;
            }
        }
    }

    return match;
}

bool FinalTrackTreeNode::isMatchedToReco(Index recoIndex, float threshold, int seedSize)
{  
    std::vector<std::shared_ptr<Hit>> hitsList;
    this->getHitsList(hitsList, -1);
    float counter = 0;
    for (size_t i=seedSize; i < hitsList.size(); i++)
    {
        auto hit = hitsList.at(i);
        Index idx = hit->getTruthTrackBarcode().at(0);
        if (idx == recoIndex) counter += 1 ;
    }
    float truthFraction  = counter / (this->getDepth() - seedSize);
    
    if (truthFraction > threshold) return true;
    else return false;
}

int TrackTree::addActiveHitNodes(std::shared_ptr<TrackTreeNode> currentNode, std::vector<std::shared_ptr<Hit>>& hits)
{
    int nHits = 0;
    for (auto& hit: hits)
    {
        if(m_currentSize >= m_maxSize) { return nHits; }
        auto hitNode = std::make_shared<TrackTreeNode> (hit);
        hitNode->setMaxBranches(m_maxBranches);
        bool t = currentNode->addNextHit(hitNode);
        if (!t){
            return nHits; // The hit was not added because maxActiveBranches has already been reached. return
        }

        this->addActiveHitNode(hitNode);
        nHits++; // Hit as been used

        m_currentSize++;
    }

    return nHits;

}

int TrackTree::addFinalHitNodes(std::shared_ptr<TrackTreeNode> currentNode, std::vector<std::shared_ptr<Hit>>& hits)
{
    int nHits = 0;
    for (auto& hit: hits)
    {
        if(m_currentSize >= m_maxSize) { return nHits; }
        auto hitNode = std::make_shared<FinalTrackTreeNode> (hit);
        
        // Ingore the output
        // bool t = currentNode->addNextHit(hitNode);
        currentNode->addNextHit(hitNode);
        
        // Always add Final hits
        this->addFinalHitNode(hitNode);
        nHits++;

        m_currentSize++;
    }

    return nHits;

}

void TrackTree::printDebug(std::shared_ptr<TrackTreeNode> node)
{
    node->getHit()->printDebug();
    if(!node->getPrevHit()) 
    {
        return;
    }
    printDebug(node->getPrevHit());
    std::cout<<std::endl;
}

void TrackTree::getTrackBackwards(std::shared_ptr<TrackTreeNode> node, std::vector<std::shared_ptr<Hit>>& track)
{
    track.push_back(node->getHit());
    if(!node->getPrevHit()) {return;}
    getTrackBackwards(node->getPrevHit(), track);
}

// Does this need to be a member of TrackTree?
void TrackTree::getHitsList(std::shared_ptr<TrackTreeNode> node, std::vector<std::shared_ptr<Hit>>& track, int nHits)
{
    // set nHits = -1 to use all hits
    if(nHits == 0 || !node) { return; }
    track.push_back(node->getHit());
    if(!node->getPrevHit()) { return; }
    nHits--;
    getHitsList(node->getPrevHit(), track, nHits);
}

void TrackTree::eraseActiveHits()
{
    // m_activeHits.erase(m_activeHits.begin(), m_activeHits.begin()+m_usedHitCounter);
    // m_usedHitCounter = 0;
    m_activeHits.pop_back();
}

void TrackTree::activeHitToFinalHits(std::shared_ptr<TrackTreeNode> node)
{
    auto Final = std::make_shared<FinalTrackTreeNode>(FinalTrackTreeNode(*node));
    m_FinalHits.push_back(Final);
}

// void TrackTree::activeHitsToFinalHits()
// {
//     auto start = std::make_move_iterator(m_activeHits.begin());
//     auto end = std::make_move_iterator(m_activeHits.begin() );
//     std::move(start, end, std::back_inserter(m_FinalHits));

// }