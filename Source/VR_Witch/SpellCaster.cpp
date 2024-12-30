// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellCaster.h"
#include "Engine/Engine.h"
#define phi 1.61803398874989484820458683436563811772030917980576286213544862270526046281890244970720720418939113748475408807538689175212663386222353693179318006076672635443338908659593958290563832266131992829026788067520876689250171169620703222104321626954862629631361443814975870122034080588795445474924618569536486444924104432077134494704956584678850987433944221254487706647809158846074998871240076521705751797883416625624940758906970400028121042762177111777805315317141011704666

// Sets default values for this component's properties
USpellCaster::USpellCaster()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

TArray<FVector2D> USpellCaster::ProjectPointsTo2D(TArray<FVector> Points)
{
	// Calculate the least variation axis
	FVector LeastVariationAxis = CalculateLeastVariationAxis(Points);

	// Find the two other axes perpendicular to the least variation axis
	FVector AxisA = FVector::CrossProduct(LeastVariationAxis, FVector(1, 0, 0));
	if (AxisA.IsZero())
	{
		AxisA = FVector::CrossProduct(LeastVariationAxis, FVector(0, 1, 0));
	}
	AxisA.Normalize();

	FVector AxisB = FVector::CrossProduct(LeastVariationAxis, AxisA);
	AxisB.Normalize();

	TArray<FVector2D> ProjectedPoints;

	// Project each 3D point onto the 2D plane defined by AxisA and AxisB
	for (const FVector& Point : Points)
	{
		// Calculate the 2D projection
		FVector2D ProjectedPoint;
		ProjectedPoint.X = FVector::DotProduct(Point, AxisA);
		ProjectedPoint.Y = FVector::DotProduct(Point, AxisB);

		ProjectedPoints.Add(ProjectedPoint);
	}

	return ProjectedPoints;
}

/// <summary>
/// find out which template the points best match
/// </summary>
/// <param name="points">the points u want to find the most matching template for</param>
/// <param name="score">the score of how much it looks like the template</param>
/// <param name="templateIndex">the index of the template it looks like</param>
void USpellCaster::Recognize(TArray<FVector2D> points, double& score, int& templateIndex)
{
	/*if (templates.Num() == 0) {
		throw "no templates to check";
	}
	if (points.Num() < 32)
	{
		throw "points needs at least 32 points";
	}*/
	
	TArray<FVector2D> resampledPoints = Resample(points,32);
	double radians = IndicativeAngle(resampledPoints);
	resampledPoints = Rotate(resampledPoints, radians);
	resampledPoints = Scale(resampledPoints, 250);
	resampledPoints = Translate(resampledPoints, FVector2D(0, 0));

	int bestTemplateIndex = 0;
	double best = INFINITY;
	for (size_t i = 0; i < templates.Num(); i++)
	{
		double score = DistanceAtBestAngle(resampledPoints, templates[i], -5, 5, 1);
		if (score < best)
		{
			bestTemplateIndex = i;
			best = score;
		}
	}
	score = best;
	templateIndex = bestTemplateIndex;
	return;
}

/// <summary>
/// 
/// </summary>
/// <param name="points"></param>
void USpellCaster::AddTemplate(TArray<FVector2D> points)
{
	if (points.Num() < 32)
	{
		throw "points needs at least 32 points";
	}
	TArray<FVector2D> newTemplate = Resample(points, 32);
	double radians = IndicativeAngle(newTemplate);
	newTemplate = Rotate(newTemplate, radians);
	newTemplate = Scale(newTemplate, 250);
	newTemplate = Translate(newTemplate, FVector2D(0, 0));
	templates.Add(newTemplate);
}

FVector USpellCaster::CalculateLeastVariationAxis(TArray<FVector> Points)
{
	FVector MeanPoint(0.f, 0.f, 0.f);
	int32 NumPoints = Points.Num();

	// Calculate the mean point (center of mass)
	for (const FVector& Point : Points)
	{
		MeanPoint += Point;
	}
	MeanPoint /= NumPoints;

	// Compute the covariance matrix (simplified approach)
	FVector CovX(0.f, 0.f, 0.f);
	FVector CovY(0.f, 0.f, 0.f);
	FVector CovZ(0.f, 0.f, 0.f);

	// Calculate the covariance vectors
	for (const FVector& Point : Points)
	{
		FVector Diff = Point - MeanPoint;
		CovX += FVector(Diff.X * Diff.X, Diff.Y * Diff.X, Diff.Z * Diff.X);
		CovY += FVector(Diff.X * Diff.Y, Diff.Y * Diff.Y, Diff.Z * Diff.Y);
		CovZ += FVector(Diff.X * Diff.Z, Diff.Y * Diff.Z, Diff.Z * Diff.Z);
	}

	// Find which axis has the least variation (the smallest covariance)
	FVector Covariance = CovX + CovY + CovZ;

	FVector LeastVariationAxis;
	if (Covariance.X < Covariance.Y && Covariance.X < Covariance.Z)
	{
		LeastVariationAxis = FVector(1, 0, 0);  // X-axis has least variation
	}
	else if (Covariance.Y < Covariance.Z)
	{
		LeastVariationAxis = FVector(0, 1, 0);  // Y-axis has least variation
	}
	else
	{
		LeastVariationAxis = FVector(0, 0, 1);  // Z-axis has least variation
	}

	return LeastVariationAxis;
}

/// <summary>
/// find the diference between two list of points and find the angle where the score is best and gives you that score back
/// </summary>
/// <param name="points">points to compare to a template</param>
/// <param name="streokTemplate">the template to compare too</param>
/// <param name="fromAngle">the minimum angle to check for</param>
/// <param name="toAngle">the maximum angle to check for</param>
/// <param name="threshold">how close to search for before chsing the score</param>
/// <returns>the distance between the point and the template</returns>
double USpellCaster::DistanceAtBestAngle(TArray<FVector2D> points, TArray<FVector2D> strokeTemplate, double fromAngle, double toAngle, double threshold)
{
	//needs to find the score here
	double x1 = phi * fromAngle + (1.0 - phi) * toAngle;
	double f1 = DistanceAtAngle(points, strokeTemplate, x1);

	double x2 = (1.0 - phi) * fromAngle + phi * toAngle;
	double f2 = DistanceAtAngle(points, strokeTemplate, x2);

	while (abs(toAngle-fromAngle) > threshold)
	{
		if (f1 < f2) {
			toAngle = x2;
			x2 = x1;
			f2 = f1;
			x1 = phi * fromAngle + (1.0 - phi) * toAngle;
			f1 = DistanceAtAngle(points, strokeTemplate, x1);
		}
		else
		{
			fromAngle = x2;
			x1 = x2;
			f1 = f2;
			x2 = (1.0 - phi) * fromAngle + phi * toAngle;
			f2 = DistanceAtAngle(points, strokeTemplate, x2);
		}
	}
	return f1 > f2 ? f2 : f1;
}


// Called when the game starts
void USpellCaster::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}

/// <summary>
/// resamples points in a spell to a given amount with even spacing between
/// </summary>
/// <param name="points">a list of all point's that needs resampling</param>
/// <param name="resampledPointAmount">the amount of point's needed after resampling</param>
TArray<FVector2D> USpellCaster::Resample(TArray<FVector2D> points, int resampledPointAmount)
{
	TArray<FVector2D> initialPoints = points;// a list of points that we started with
	double interval = 0;//a varable that will contain how much spacing there should be between each point
	for (size_t i = 1; i < initialPoints.Num(); i++)//calculated how long the line is
	{
		interval += FVector2D::Distance(initialPoints[i - 1], initialPoints[i]);
	}
	interval /= (resampledPointAmount - 1);//divides the line up so we get a even spacing lenthbetween each point
	double totalLentgh = 0;
	TArray<FVector2D> newPoints;
	newPoints.Add(points[0]);
	for (size_t i = 1; i < initialPoints.Num(); i++)
	{
		double currentLength = FVector2D::Distance(initialPoints[i - 1],initialPoints[i]);
		if (totalLentgh + currentLength >= interval)
		{
			double newX = initialPoints[i - 1].X + ((interval - totalLentgh) / currentLength) * (initialPoints[i].X - initialPoints[i - 1].X);
			double newY = initialPoints[i - 1].Y + ((interval - totalLentgh) / currentLength) * (initialPoints[i].Y - initialPoints[i - 1].Y);
			FVector2D newPoint = FVector2D(newX, newY);
			newPoints.Add(newPoint);
			totalLentgh = 0;
		}
		else {
			totalLentgh += currentLength;
		}
	}
	if (newPoints.Num() == resampledPointAmount)
	{
		newPoints.Add(points.Last());
	}
	return newPoints;
}

/// <summary>
/// 
/// </summary>
/// <param name="points"></param>
/// <returns></returns>
double USpellCaster::IndicativeAngle(TArray<FVector2D> points)
{
	FVector2D centroid = FindCentroid(points);
	return atan2(centroid.Y - points[0].Y, centroid.X - points[0].X);
}

/// <summary>
/// 
/// </summary>
/// <param name="points"></param>
/// <param name="radians"></param>
/// <returns></returns>
TArray<FVector2D> USpellCaster::Rotate(TArray<FVector2D> points, double radians)
{
	FVector2D centroid = FindCentroid(points);
	double cosValue = cos(radians);
	double sinValue = sin(radians);
	TArray<FVector2D> newPoints;
	for (size_t i = 0; i < points.Num(); i++)
	{
		double newX = (points[i].X - centroid.X) * cosValue - (points[i].Y - centroid.Y) * sinValue + centroid.X;
		double newY = (points[i].X - centroid.X) * sinValue - (points[i].Y - centroid.Y) * cosValue + centroid.X;
		newPoints.Add(FVector2D(newX, newY));
	}
	return newPoints;
}

/// <summary>
/// 
/// </summary>
/// <param name="points"></param>
/// <param name="newSize"></param>
/// <returns></returns>
TArray<FVector2D> USpellCaster::Scale(TArray<FVector2D> points, double newSize)
{
	FVector2D boundingBoxPositive;
	FVector2D boundingBoxNegative;
	for (size_t i = 0; i < points.Num(); i++)
	{
		if (points[i].X > boundingBoxPositive.X) {
			boundingBoxPositive.X = points[i].X;
		}
		else if (points[i].X < boundingBoxNegative.X) {
			boundingBoxNegative.X = points[i].X;
		}

		if (points[i].Y > boundingBoxPositive.Y) {
			boundingBoxPositive.Y = points[i].Y;
		}
		else if (points[i].Y < boundingBoxNegative.Y) {
			boundingBoxNegative.Y = points[i].Y;
		}
	}
	FVector2D boundingBox = boundingBoxPositive - boundingBoxNegative;
	TArray<FVector2D> newPoints;
	for (size_t i = 0; i < points.Num(); i++)
	{
		double newX = points[i].X * (newSize / boundingBox.X);
		double newY = points[i].Y * (newSize / boundingBox.Y);
		newPoints.Add(FVector2D(newX, newY));
	}
	return newPoints;
}

/// <summary>
/// 
/// </summary>
/// <param name="points"></param>
/// <param name="toPoint"></param>
/// <returns></returns>
TArray<FVector2D> USpellCaster::Translate(TArray<FVector2D> points, FVector2D toPoint)
{
	FVector2D centroid = FindCentroid(points);
	TArray<FVector2D> newPoints;
	for (size_t i = 0; i < points.Num(); i++)
	{
		double newX = points[i].X + toPoint.X - centroid.X;
		double newY = points[i].Y + toPoint.Y - centroid.Y;
		newPoints.Add(FVector2D(newX, newY));
	}
	return newPoints;
}

/// <summary>
/// 
/// </summary>
/// <param name="points"></param>
/// <returns></returns>
FVector2D USpellCaster::FindCentroid(TArray<FVector2D> points)
{
	FVector2D sum;
	for (size_t i = 0; i < points.Num(); i++)
	{
		sum += points[i];
	}
	FVector2D centroid = sum / points.Num();
	return centroid;
}

/// <summary>
/// 
/// </summary>
/// <param name="samples"></param>
/// <param name="theTemplate"></param>
/// <param name="theta"></param>
/// <returns></returns>
double USpellCaster::DistanceAtAngle(TArray<FVector2D> samples, TArray<FVector2D> theTemplate, float theta)
{
	int maxPoints = 128;
	TArray<FVector2D> newPoints;
	for (size_t i = 0; i < samples.Num(); i++)
	{
		newPoints.Add(samples[i]);
	}
	Rotate(newPoints, theta);
	return PathDistance(newPoints, theTemplate);
}

/// <summary>
/// 
/// </summary>
/// <param name="points1"></param>
/// <param name="points2"></param>
/// <returns></returns>
double USpellCaster::PathDistance(TArray<FVector2D> points1, TArray<FVector2D> points2)
{
	double d = 0;
	int arrayAmount = points1.Num() > points2.Num() ? points2.Num() : points1.Num();
	for (size_t i = 0; i < arrayAmount; i++)
	{
		d += FVector2D::Distance(points1[i], points2[i]);
	}
	return d / points1.Num();
}


// Called every frame
void USpellCaster::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

