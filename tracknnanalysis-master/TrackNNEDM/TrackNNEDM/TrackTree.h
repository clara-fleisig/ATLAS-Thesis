#ifndef _TRACKTREE_H
#define _TRACKTREE_H

#include "TrackNNEDM/Hit.h"
#include "TrackNNEDM/SeedTrack.h"
#include <limits.h>

//forward class decl.
class Hit;
class FinalTrackTreeNode;

// Store Index of hit in global hit container
// Using Hit pointers could result in hits next/prev being overwritten by different trees
// Index prevents copying hit information across multiple trees

class TrackTreeNode
    :public std::enable_shared_from_this<TrackTreeNode>
{
    friend class FinalTrackTreeNode;

    public:
        enum class HitStatus
        {
            active,
            Final,
            initial
        };
    private:
        const std::shared_ptr<Hit> m_hit;
        // std::vector<std::weak_ptr<TrackTreeNode>> m_nextHits; // weak_ptr to avoid circular reference
        std::shared_ptr<TrackTreeNode> m_prevHit = nullptr; 
        unsigned int m_maxBranches = 10;
        unsigned int m_nActiveBranches=0; // number of current branches

        // Change this to be an enum type
        HitStatus m_hitStatus = HitStatus::initial;
    
    public:
        TrackTreeNode(){};
        TrackTreeNode(std::shared_ptr<Hit> hit):m_hit(hit) {};
        virtual ~TrackTreeNode(){};

        std::shared_ptr<Hit> getHit(){ return m_hit; }
        // std::vector<std::weak_ptr<TrackTreeNode>> getNextHits(){ return m_nextHits; }
        std::shared_ptr<TrackTreeNode> getPrevHit(){ return m_prevHit; }

        void setPrevHit(std::shared_ptr<TrackTreeNode> hit){ m_prevHit = hit; } 
        void addNextHits(std::vector<std::shared_ptr<TrackTreeNode>> hits);
        bool addNextHit(std::shared_ptr<TrackTreeNode> hit);
        bool addNextHit(std::shared_ptr<FinalTrackTreeNode> hit);
        void setFinal(){ m_hitStatus=HitStatus::Final; }
        void setActive(){ m_hitStatus=HitStatus::active; }
        void getHitsList(std::vector<std::shared_ptr<Hit>>& track, int nHits=-1);
        int getDepth(int current=0);


        void setMaxBranches(unsigned int maxBranches){m_maxBranches = maxBranches;}
};

class FinalTrackTreeNode: public TrackTreeNode
{

    private:
            float m_NNEvalScore = -1;
            int trackLength = 0;
    
    // Final Hit in a track can store the NN Classification score of that track
    public:
        FinalTrackTreeNode(){};
        FinalTrackTreeNode(const TrackTreeNode& node):TrackTreeNode(node){};
        FinalTrackTreeNode(std::shared_ptr<Hit> hit):TrackTreeNode(hit) {} ;
        // FinalTrackTreeNode(std::shared_ptr<TrackTreeNode> node);
        ~FinalTrackTreeNode(){};

        public:
            float getNNScore() { return m_NNEvalScore; }
            void  getNNScore(float score) { m_NNEvalScore = score; }
            int getNHitOverlap(std::shared_ptr<FinalTrackTreeNode> compNode);
            bool isMatchedToReco(Index recoIndex, float threshold, int seedSize);

};


class TrackTree
{

    private:
        std::shared_ptr<SeedTrack> m_seed;
        // std::vector<std::shared_ptr<TrackTreeNode>> m_treeNodeSeed;
        unsigned int m_maxSize;
        unsigned int m_currentSize = 0;
        int m_usedHitCounter = 0;

    public:
        
        TrackTree(std::shared_ptr<SeedTrack> seed, unsigned int maxSize=UINT_MAX);
        ~TrackTree(){}
        std::shared_ptr<SeedTrack> getSeed(){ return m_seed; }
        void setSeed(std::shared_ptr<SeedTrack> seed){ m_seed=seed; }
    
    private:
        std::vector<std::shared_ptr<TrackTreeNode>> m_activeHits;
        std::vector<std::shared_ptr<FinalTrackTreeNode>> m_FinalHits;
        unsigned int m_maxBranches = 10;

    public:
        void setMaxBranches(unsigned int maxBranches){ m_maxBranches = maxBranches; }
        // return the number of used hits
        int addActiveHitNodes( std::shared_ptr<TrackTreeNode> currentNode, std::vector<std::shared_ptr<Hit>>& hits);
        int addFinalHitNodes( std::shared_ptr<TrackTreeNode> currentNode, std::vector<std::shared_ptr<Hit>>& hits);
        void addFinalHitNode(std::shared_ptr<FinalTrackTreeNode> hitNode) 
        { 
            hitNode->setFinal();
            m_FinalHits.push_back(hitNode); 
        }
        void addActiveHitNode(std::shared_ptr<TrackTreeNode> hitNode) 
        { 
            hitNode->setActive();
            m_activeHits.push_back(hitNode); 
        }
        void addHit(std::shared_ptr<Hit> hitNode);

        std::vector<std::shared_ptr<TrackTreeNode>> getActiveHits(){ return m_activeHits; }
        std::vector<std::shared_ptr<FinalTrackTreeNode>> getFinalHits(){ return m_FinalHits; }
        void eraseActiveHits();
        void activeHitToFinalHits(std::shared_ptr<TrackTreeNode> node);
        void printDebug(std::shared_ptr<TrackTreeNode> node);

        void getTrackBackwards(std::shared_ptr<TrackTreeNode> node, std::vector<std::shared_ptr<Hit>>& track);
        void getHitsList(std::shared_ptr<TrackTreeNode> node, std::vector<std::shared_ptr<Hit>>& track, int nHits=-1);

        void incrementUsedCounter(int val){
            m_usedHitCounter+=val;
        }



};

#endif