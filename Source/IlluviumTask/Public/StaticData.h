#pragma once

/***/
UENUM()
enum class ETeam
{
	NoTeam UMETA(DisplayName="NoTeam"),
	BlueTeam UMETA(DisplayName="BlueTeam"),
	RedTeam UMETA(DisplayName="RedTeam"),
	MAX UMETA(DisplayName="MaxTeams")
};

/**
 * Grid oriented coordinates
 */
struct FGridCoordinates
{
	FGridCoordinates();
	
	explicit FGridCoordinates(int32 X, int32 Y);
		
	int32 GridX = 0;
	int32 GridY = 0;

	bool operator==(const FGridCoordinates& InCoordinates) const;

	FGridCoordinates operator-(const FGridCoordinates& InCoordinates) const;

	// Static helpers / utility
	static const FGridCoordinates Empty()
	{
		return FGridCoordinates{-1, -1};
	};

	static float DistSqr(const FGridCoordinates& InCoordinatesLeft, const FGridCoordinates& InCoordinatesRight)
	{
		return FMath::Square(InCoordinatesLeft.GridX - InCoordinatesRight.GridX) + FMath::Square(
			InCoordinatesLeft.GridY - InCoordinatesRight.GridY);
	}

	FString ToString() const;
};
