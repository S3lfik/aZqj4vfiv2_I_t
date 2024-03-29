#include "StaticData.h"


FGridCoordinates::FGridCoordinates()
	: GridX(0), GridY(0)
{}

FGridCoordinates::FGridCoordinates(int32 X, int32 Y)
{
	GridX = X;
	GridY = Y;
}

bool FGridCoordinates::operator==(const FGridCoordinates& InCoordinates) const
{
	return (GridX == InCoordinates.GridX) && (GridY == InCoordinates.GridY);
}

FGridCoordinates FGridCoordinates::operator-(const FGridCoordinates& InCoordinates) const
{
	return FGridCoordinates{this->GridX - InCoordinates.GridX, this->GridY - InCoordinates.GridY};
}

FString FGridCoordinates::ToString() const
{
	const FString DebugString(TEXT("X:{0}, Y:{1}"));
	return FString::Format(*DebugString, {	*FString::FromInt(GridX), *FString::FromInt(GridY)});
}