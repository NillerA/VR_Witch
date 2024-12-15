// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellCaster.h"
#define phi 1.61803398874989484820458683436563811772030917980576286213544862270526046281890244970720720418939113748475408807538689175212663386222353693179318006076672635443338908659593958290563832266131992829026788067520876689250171169620703222104321626954862629631361443814975870122034080588795445474924618569536486444924104432077134494704956584678850987433944221254487706647809158846074998871240076521705751797883416625624940758906970400028121042762177111777805315317141011704666

// Sets default values for this component's properties
USpellCaster::USpellCaster()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void USpellCaster::Recognize(TArray<FVector2D> points, double& score, int& templateIndex)
{
	TArray<FVector2D> resampledPoints = Resample(points,32);
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

void USpellCaster::AddTemplate(TArray<FVector2D> points)
{
	TArray<FVector2D> newTemplate = Resample(points, 32);
	templates.Add(newTemplate);
}

/// <summary>
/// 
/// </summary>
/// <param name="points"></param>
/// <param name="streokTemplate"></param>
/// <param name="fromAngle"></param>
/// <param name="toAngle"></param>
/// <param name="threshold"></param>
/// <returns></returns>
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

double USpellCaster::IndicativeAngle(TArray<FVector2D> points)
{
	FVector2D centroid = FindCentroid(points);
	return atan2(centroid.Y - points[0].Y, centroid.X - points[0].X);
}

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

