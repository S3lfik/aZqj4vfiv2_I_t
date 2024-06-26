// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IT_Grid.h"
#include "IlluviumTask/IlluviumTask.h"

struct FGrid;

namespace Path
{
	struct FConnection;
	struct FNodeRecord;

	struct FNode
	{
		FIntPoint XY;
		bool bIsReachable = false;

		bool operator==(const FNode& InNode) const
		{
			return (this->XY == InNode.XY);
		}
	};

	struct FConnection
	{
		FNode NodeFrom;
		FNode NodeTo;
		float ConnectionCost = 0.f;

		FNodeRecord* From = nullptr;
		FNodeRecord* To = nullptr;
	};

	struct FNodeRecord
	{
		FNodeRecord()
		{
		};

		FNodeRecord(const FNode& InNode)
		{
			Node = InNode;
		}

		FConnection ParentConnection;
		float HeuristicValue = 0.f;
		float CostSoFar = 0.f;
		float EstimatedTotalCost = 0.f;
		FNode Node;
		TSharedRef<FNodeRecord> ParentNodeRef;
		FNodeRecord* ParentNodePtr;
	};

	using FNodeRecordPtr = TSharedPtr<Path::FNodeRecord>;
	typedef TUniquePtr<Path::FNodeRecord> FNodeRecordUPtr;

	enum EVisitStatus
	{
		Unvisited = 0,
		Discovered = 1,
		Visited = 2
	};

	struct FNodeRecord2
	{
		FNodeRecord2()
		{
			VisitStatus = Unvisited;
		};

		FNodeRecord2(const FNode& InNode, EVisitStatus InVisitStatus = Unvisited)
		{
			Node = InNode;
			VisitStatus = InVisitStatus;
		}
		~FNodeRecord2()
		{
			UE_LOG(LogTask, Display, TEXT("Node Record for %s is deconstructed."), *Node.XY.ToString());
		}

		FConnection ParentConnection;
		float HeuristicValue = 0.f;
		float CostSoFar = 0.f;
		float EstimatedTotalCost = 0.f;
		FNode Node;
		FNodeRecord2* ParentNodePtr = nullptr;
		EVisitStatus VisitStatus;
	};

	struct FHeuristic
	{
		FHeuristic() = delete;

		FHeuristic(const FNode& InEndNode)
		{
			EndNode = InEndNode;
		}

		float Estimate(const FNode& InStartNode) const
		{
			return FIntPoint(EndNode.XY - InStartNode.XY).SizeSquared();
		}

		static float Estimate(const FNode& InStartNode, const FNode& InEndNode)
		{
			return FIntPoint(InEndNode.XY - InStartNode.XY).SizeSquared();
		}

	private:
		FNode EndNode;
	};

	/**
	* A predicate class for pushing NodeRecords to the heapified open and closed lists.
	*/
	struct LessDistancePredicate
	{
		bool operator()(FNodeRecord LeftRecord, FNodeRecord RightRecord) const
		{
			return LeftRecord.EstimatedTotalCost < RightRecord.EstimatedTotalCost;
		};

		bool operator()(FNodeRecord* LeftRecord, FNodeRecord* RightRecord) const
		{
			return LeftRecord->EstimatedTotalCost < RightRecord->EstimatedTotalCost;
		};

		bool operator()(const FNodeRecord2& LeftRecord, const FNodeRecord2& RightRecord) const;

		bool operator()(const FNodeRecordUPtr& LeftRecord, const FNodeRecordUPtr& RightRecord) const
		{
			return LeftRecord->EstimatedTotalCost < RightRecord->EstimatedTotalCost;
		};
	};

	struct FGraph
	{
		FGraph(const FGrid& InGrid)
			: GridRef(InGrid)
		{
		}

		TArray<FConnection> GetNodeConnections(const FNodeRecord& InNodeRecord) const;
		TArray<FNode> GetNodeConnections(const FNode& InNode) const;

		const FGrid& GridRef;
	};
}

/**
 * 
 */
class ILLUVIUMTASK_API IT_Pathfinder
{
public:
	IT_Pathfinder() = default;

	void InitGraph(const FGrid& InGrid);

	TArray<Path::FNode> FindPath(const Path::FNode& StartNode, const Path::FNode& EndNode);
	TArray<Path::FNode> GetNeighbors(const Path::FNode& InNode);
	void VisualizePath(UWorld* World, TArray<Path::FNode> Array, float GridScale);

	~IT_Pathfinder();

private:
	TUniquePtr<Path::FGraph> Graph;
};
